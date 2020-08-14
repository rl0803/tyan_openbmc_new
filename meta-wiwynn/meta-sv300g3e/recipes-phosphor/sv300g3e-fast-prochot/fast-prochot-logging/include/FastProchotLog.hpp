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

#pragma once

#include <gpiod.h>
#include <unistd.h>
#include <variant>
#include <iostream>
#include <systemd/sd-journal.h>
#include <sdbusplus/asio/object_server.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/chrono/chrono_io.hpp>
#include <boost/process.hpp>
#include <nlohmann/json.hpp>

constexpr char const *ipmiSELService = "xyz.openbmc_project.Logging.IPMI";
constexpr char const *ipmiSELPath = "/xyz/openbmc_project/Logging/IPMI";
constexpr char const *ipmiSELAddInterface = "xyz.openbmc_project.Logging.IPMI";
const static std::string ipmiSELAddMessage = "SEL Entry";

const static std::string gpioConfigPath = "/etc/fast-prochot-gpio.json";
const static std::string sensorPathPrefix = "/xyz/openbmc_project/sensors/oem1/";

class FastProchotObject
{
    public:
    /** @brief Constructs CPU Prochot Monitoring object.
     *
     *  @param[in] io          - io service
     *  @param[in] conn        - dbus connection
     *  @param[in] line        - GPIO line from libgpiod
     *  @param[in] config      - configuration of line with event
     */
    FastProchotObject(boost::asio::io_service& io,
                    std::shared_ptr<sdbusplus::asio::connection>& conn,
                    gpiod_line* line, gpiod_line_request_config& config,
                    int gpioNumber, std::string netName);
    ~FastProchotObject();

    private:
        gpiod_line* gpioLine;
        gpiod_line_request_config gpioConfig;
        int gpioNumber;
        std::string netName;
        boost::asio::posix::stream_descriptor gpioEventDescriptor;
        std::shared_ptr<sdbusplus::asio::connection>& conn;
        boost::asio::steady_timer waitTimer;

        void waitFastProchotEvent();
        void fastProchotEventHandler();
        int requestFastProchotEvent();
        void addFastProchotEventSEL();
};
