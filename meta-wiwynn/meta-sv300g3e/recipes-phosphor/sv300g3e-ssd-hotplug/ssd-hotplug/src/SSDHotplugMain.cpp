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
#include <filesystem>
#include <sdbusplus/asio/connection.hpp>
#include <systemd/sd-journal.h>
#include <openbmc/libobmci2c.h>

/*
boost::asio::io_service io;
auto systemBus = std::make_shared<sdbusplus::asio::connection>(io);
*/
static auto dbus = sdbusplus::bus::new_default_system();
static constexpr unsigned int intervalMs = 1000;

void writeSEL(bool ssdStatus, uint8_t ssdIdx)
{
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
        std::cerr << "Failed to add SSD SEL\n";
    }
}

void readSSDstatusFromI2C(bool *ssdStatus)
{
    int fd = -1;
    int res = -1;
    std::vector<char> filename;
    filename.assign(20, 0); 

    int busId = 0;
    fd = open_i2c_dev(busId, filename.data(), filename.size(), 0);

    if(fd < 0)
    {
        std::cerr << "Fail to open I2C device\n";
    }

    std::vector<uint8_t> cmdData;
    cmdData.assign(1, 0x0);

    std::vector<uint8_t> readBuf;                  
    readBuf.assign(1, 0x0);

    uint8_t slaveAddr = 0x21;

    res = i2c_master_write_read(fd, slaveAddr, cmdData.size(), cmdData.data(), readBuf.size(), readBuf.data());
    uint8_t raw_value = readBuf.at(0);
    if (res < 0)
    {
        std::cerr << "I2C read error\n";
    }

    close_i2c_dev(fd);

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
}


int main(int argc, char *argv[])
{
    //Initial SSD status
    bool ssdOriginalStatus[ssdNumber];
    readSSDstatusFromI2C(ssdOriginalStatus);
    for (uint8_t ssdIdx = 0; ssdIdx < ssdNumber; ssdIdx++)
    {
        writeSEL(ssdOriginalStatus[ssdIdx], ssdIdx);
    }

    //Polling SSD status each second
    bool ssdCurrentStatus[ssdNumber];
    while (true)
    {
        readSSDstatusFromI2C(ssdCurrentStatus);
        
        for(uint8_t i = 0; i < ssdNumber; i++)
        {
            if(ssdCurrentStatus[i] != ssdOriginalStatus[i])
            {
                ssdOriginalStatus[i] = ssdCurrentStatus[i];
                writeSEL(ssdOriginalStatus[i], i);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
    }

    return 0;
}
