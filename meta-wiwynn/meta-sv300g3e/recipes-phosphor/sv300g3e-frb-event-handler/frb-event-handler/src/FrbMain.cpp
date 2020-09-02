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

#include "FrbHandler.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/container/flat_set.hpp>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <systemd/sd-journal.h>
#include <unistd.h>

static bool acpiPowerStateOn = false;
static std::unique_ptr<sdbusplus::bus::match::match> acpiPowerMatch = nullptr;
static std::unique_ptr<FrbEventObject> frbHandlerObj = nullptr;

bool isAcpiPowerOn(void)
{
    return acpiPowerStateOn;
}

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
                if (1 == on)
                {
                    acpiPowerStateOn = true;
                    if (frbHandlerObj != nullptr)
                    {
                        frbHandlerObj->startFrb3Timer();
                    }
                }
                else
                {
                     acpiPowerStateOn = false;
                }
            }
        });

    if(!acpiPowerMatch)
    {
        std::cerr << "Power Match not created\n";
        return false;
    }

    return true;
}

bool createFrbEventObjects(
    boost::asio::io_service& io,
    std::shared_ptr<sdbusplus::asio::connection>& conn,
    sdbusplus::asio::object_server& objectServer)
{
    std::ifstream gpioConfigFile(gpioConfigPath);

    if(!gpioConfigFile)
    {
        std::cerr << "Failed to open frb GPIO config file: "
                  << gpioConfigPath <<"\n";

        return false;
    }
    auto data = nlohmann::json::parse(gpioConfigFile, nullptr, false);
    if(data.is_discarded())
    {
        std::cerr << "Syntax error in "
                  << gpioConfigPath << "\n";

        return false;
    }
    int GpioNum = data["prefrb2GpioNum"];
    int waitSeconds = data["TimeOutSec"];

    // frbHandlerObj.push_back(std::make_unique<FrbEventObject>(objectServer, io, conn, GpioNum, waitSeconds));
    frbHandlerObj = std::make_unique<FrbEventObject>(objectServer, io, conn, GpioNum, waitSeconds);

    return true;
}

int main(int argc, char *argv[])
{
    boost::asio::io_service io;
    auto systemBus = std::make_shared<sdbusplus::asio::connection>(io);
    systemBus->request_name("xyz.openbmc_project.FrbHanler");
    sdbusplus::asio::object_server objectServer(systemBus);
    // std::vector<std::unique_ptr<FrbEventObject>> frbHandlerObj;

    // if(!createFrbEventObjects(io, systemBus, frbHandlerObj, objectServer))
    if(!createFrbEventObjects(io, systemBus, objectServer))
    {
        std::cerr << "Failed to create Frb objects\n";
        return -1;
    }

    if(!startAcpiPowerMonitor(systemBus))
    {
        std::cerr << "Failed to start ACPI power monitor\n";
        return -1;
    }

    io.run();

    return 0;
}
