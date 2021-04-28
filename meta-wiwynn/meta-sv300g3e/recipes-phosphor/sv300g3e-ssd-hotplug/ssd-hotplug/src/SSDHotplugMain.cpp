/*
// Copyright (c) 2020 Wiwynn Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#include "SSDHotplug.hpp"

#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/asio/steady_timer.hpp>
#include <filesystem>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <systemd/sd-journal.h>
#include <openbmc/libobmci2c.h>

/*
boost::asio::io_service io;
auto systemBus = std::make_shared<sdbusplus::asio::connection>(io);
static auto dbus = sdbusplus::bus::new_default_system();
*/
static boost::asio::io_service io;
static boost::asio::steady_timer pollingTimer(io);
static constexpr unsigned int pollingSec = 1;

static bool ssdOriginalStatus[ssdNumber] = {false};
static bool ssdCurrentStatus[ssdNumber] = {false};

static bool acpiPowerStateOn = false;
static std::unique_ptr<sdbusplus::bus::match::match> acpiPowerMatch = nullptr;

bool startAcpiPowerMonitor(
    const std::shared_ptr<sdbusplus::asio::connection>& conn)
{
    // 1. Initialize acpiPowerStateOn
    std::variant<int> state;
    auto method = conn->new_method_call(powerStateService, powerStatePath,
                                        propertyInterface, "Get");
    method.append(powerStateInterface, "pgood");
    try
    {
        auto reply = conn->call(method);
        reply.read(state);

        if( 1 != std::get<int>(state))
        {
            acpiPowerStateOn = false;
        }
        else
        {
            acpiPowerStateOn = true;
        }
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        std::cerr << "Not able to get pgood property\n";
        return false;
    }

    // 2. Initialize acpiPowerStateOn
    acpiPowerMatch = std::make_unique<sdbusplus::bus::match::match>(
        static_cast<sdbusplus::bus::bus &>(*conn),
        "type='signal',interface='org.freedesktop.DBus.Properties',"
        "member='PropertiesChanged',path='/org/openbmc/control/power0',"
        "arg0='org.openbmc.control.Power'",
        [](sdbusplus::message::message& message) {
            boost::container::flat_map<std::string, std::variant<int>> values;
            std::string objectName;
            message.read(objectName, values);

            auto findState = values.find("pgood");
            if (findState != values.end())
            {
                int on = std::get<int>(findState->second);
                if (1 != on)
                {
                    acpiPowerStateOn = false;
                }
                else
                {
                    acpiPowerStateOn = true;
                }
            }
        });

    if(!acpiPowerMatch)
    {
        std::cerr << "[SSD] Power Match not created\n";
        return false;
    }

    return true;
}

static void writeSEL(bool ssdStatus, uint8_t ssdIdx)
{
    auto dbus = sdbusplus::bus::new_default_system();

    // std::cerr << "No." << ssdNum << " SSD's value=" << ssdStatus << "\n";
    std::vector<uint8_t> eventData(9, 0xFF);
    eventData.at(0) = 0x20;
    eventData.at(1) = 0x0;
    eventData.at(2) = evmRev;
    eventData.at(3) = 0xD; // sensorType;
    uint8_t sensorNum = ssdSenNumBase + ssdIdx;
    eventData.at(4) = sensorNum; //sensorNum
    eventData.at(5) = 0x8; //eventType;
    eventData.at(6) = 0x0;
    if(ssdStatus)
    {
        eventData.at(6) = 0x1;
    }
    eventData.at(7) = 0xFF;
    eventData.at(8) = 0xFF;


    sdbusplus::message::message selWrite = dbus.new_method_call(
        ipmiSELService, ipmiSELPath, ipmiSELAddInterface, "IpmiSelAddOem");
    selWrite.append(ipmiSELAddMessage, eventData, recordType);

    try
    {
        dbus.call(selWrite);
    }
    catch (sdbusplus::exception_t& e)
    {
        std::cerr << "[SSD] Failed to add SSD SEL\n";
    }

    return;
}

static int readSSDstatusThruI2C(bool *ssdStatus)
{
    int fd = -1;
    int res = -1;
    std::vector<char> filename;
    filename.assign(20, 0); 

    int busId = 0;
    fd = open_i2c_dev(busId, filename.data(), filename.size(), 0);
    if(fd < 0)
    {
        std::cerr << "[SSD] Fail to open I2C device\n";
        return -1;
    }

    std::vector<uint8_t> cmdData;
    cmdData.assign(1, 0x0);

    std::vector<uint8_t> readBuf;                  
    readBuf.assign(1, 0x0);

    uint8_t slaveAddr = 0x21;

    res = i2c_master_write_read(fd, slaveAddr, cmdData.size(), cmdData.data(),
                                readBuf.size(), readBuf.data());
    uint8_t raw_value = readBuf.at(0);

    close_i2c_dev(fd);

    if (res < 0)
    {
        std::cerr << "[SSD] I2C read error\n";
        return -1;
    }

    // SSD present status in bitwise map
    for(uint8_t i = 0 ; i < ssdNumber ; i++)
    {
        uint8_t tmp = raw_value >> i;

        if((tmp & 0x01) == 0) // 0 means in slot
        {
            ssdStatus[i] = true;
        }
        else
        {
            ssdStatus[i] = false;
        }
    }

    return res;
}

static void pollSSDstatus()
{
    int status = readSSDstatusThruI2C(ssdCurrentStatus);

    if(status != -1)
    {
        for(uint8_t i = 0; i < ssdNumber; i++)
        {
            if(ssdCurrentStatus[i] != ssdOriginalStatus[i])
            {
                ssdOriginalStatus[i] = ssdCurrentStatus[i];

                if (true == acpiPowerStateOn)
                {
                    writeSEL(ssdOriginalStatus[i], i);
                }
            }
        }
    }

    pollingTimer.expires_from_now(boost::asio::chrono::seconds(pollingSec));
    pollingTimer.async_wait([&](const boost::system::error_code& ec) {
        if (ec == boost::asio::error::operation_aborted)
        {
            return; // we're being canceled
        }
        else if (ec)
        {
            std::cerr << "timer error\n";
            return;
        }

        pollSSDstatus();
    });

    return;
}

int main(int argc, char *argv[])
{
    auto systemBus = std::make_shared<sdbusplus::asio::connection>(io);
    systemBus->request_name("xyz.openbmc_project.ssd-hotplug-event");
    sdbusplus::asio::object_server objectServer(systemBus);

    if(!startAcpiPowerMonitor(systemBus))
    {
        std::cerr << "Failed to start ACPI power monitor\n";
        return -1;
    }

    //Initial SSD status
    int status = readSSDstatusThruI2C(ssdOriginalStatus);

    // SSD present status polling
    pollSSDstatus();

    io.run();

    return 0;
}
