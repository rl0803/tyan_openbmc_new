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

constexpr char const *propertyInterface = "org.freedesktop.DBus.Properties";
constexpr char const *powerStateService = "org.openbmc.control.Power";
constexpr char const *powerStatePath = "/org/openbmc/control/power0";
constexpr char const *powerStateInterface = "org.openbmc.control.Power";

constexpr char const *ipmiSELService = "xyz.openbmc_project.Logging.IPMI";
constexpr char const *ipmiSELPath = "/xyz/openbmc_project/Logging/IPMI";
constexpr char const *ipmiSELAddInterface = "xyz.openbmc_project.Logging.IPMI";
const static std::string ipmiSELAddMessage = "SEL Entry";

const static std::string gpioConfigPath = "/etc/frb-event-setting.json";
const static std::string sensorPath = "/xyz/openbmc_project/sensors/processor/Processor_fail";

bool isAcpiPowerOn(void);

class FrbEventObject
{
    public:

    FrbEventObject(sdbusplus::asio::object_server& objectServer, 
                    boost::asio::io_service& io,
                    std::shared_ptr<sdbusplus::asio::connection>& conn,
                    int gpioNumber, int timeOutSecond);
    ~FrbEventObject();

        void startFrb3Timer();

    private:
        sdbusplus::asio::object_server& objServer;
        boost::asio::posix::stream_descriptor gpioEventDescriptor;
        std::shared_ptr<sdbusplus::asio::connection>& conn;
        boost::asio::steady_timer waitTimer;
        std::shared_ptr<sdbusplus::asio::dbus_interface> sensorInterface;
        std::string dbusPath;
        int gpioNumber;
        int timeOutSecond;
        
        // for start timer use
        int monitorTimes;
        bool isTimerOn;

        void runFrb3Algorithm();
};
