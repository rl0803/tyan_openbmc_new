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

#include "FastProchotLog.hpp"

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

static std::map<std::string, int> polarityMap = {
        {"FALLING", GPIOD_LINE_REQUEST_EVENT_FALLING_EDGE},
        {"RISING", GPIOD_LINE_REQUEST_EVENT_RISING_EDGE},
        {"BOTH", GPIOD_LINE_REQUEST_EVENT_BOTH_EDGES}
};

bool createFastProchotObjects(
    boost::asio::io_service& io,
    std::shared_ptr<sdbusplus::asio::connection>& conn,
    std::vector<std::unique_ptr<FastProchotObject>>& proHotObjs)
{
    std::ifstream gpioConfigFile(gpioConfigPath);

    if(!gpioConfigFile)
    {
        std::cerr << "Failed to open Button GPIO config file: "
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

    int idx = 0;
    while(!data[idx].is_null())
    {
        if(!data[idx]["Name"].is_null() &&
           !data[idx]["GpioNum"].is_null())
        {
            int gpioNumber = static_cast<int>(data[idx]["GpioNum"]);
            std::string netName = data[idx]["Name"];
            std::string triggerType("BOTH");
            
            if(!data[idx]["EventMon"].is_null())
            {
                triggerType = data[idx]["EventMon"];
            }

            gpiod_line* line = nullptr;
            struct gpiod_line_request_config config
            {
                "fast-prohot-logging", GPIOD_LINE_REQUEST_EVENT_BOTH_EDGES, 0
            };

            line = gpiod_line_get("0", gpioNumber);
            auto findEvent = polarityMap.find(triggerType);
            if (findEvent != polarityMap.end())
            {
                config.request_type = findEvent->second;
            }
            proHotObjs.push_back(std::make_unique<FastProchotObject>(io, conn, line, config,
                                                                    gpioNumber, netName));
        }
        idx++;
    }

    return true;
}

int main(int argc, char *argv[])
{
    boost::asio::io_service io;
    auto systemBus = std::make_shared<sdbusplus::asio::connection>(io);
    systemBus->request_name("xyz.openbmc_project.FastProhotLogging");
    sdbusplus::asio::object_server objectServer(systemBus);
    std::vector<std::unique_ptr<FastProchotObject>> proHotObjs;

    if(!createFastProchotObjects(io, systemBus, proHotObjs))
    {
        std::cerr << "Failed to create Fast Prohot objects\n";
        return -1;
    }

    io.run();

    return 0;
}
