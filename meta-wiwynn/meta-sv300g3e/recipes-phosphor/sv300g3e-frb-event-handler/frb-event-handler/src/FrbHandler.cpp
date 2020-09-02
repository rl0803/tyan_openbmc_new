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

#include <unistd.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <fstream>
#include <limits>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <string>
#include <systemd/sd-journal.h>

#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>

FrbEventObject::FrbEventObject(sdbusplus::asio::object_server& objectServer,
    boost::asio::io_service& io, std::shared_ptr<sdbusplus::asio::connection>& conn, 
    int gpioNumber, int timeOutSecond):
    objServer(objectServer),gpioEventDescriptor(io),
    conn(conn), waitTimer(io), gpioNumber(gpioNumber), timeOutSecond(timeOutSecond),
    monitorTimes(0), isTimerOn(false)
{
    dbusPath = "/xyz/openbmc_project/frbSignal";
    
    sensorInterface = objectServer.add_interface(dbusPath, "xyz.openbmc_project.frbSignal.Value");

    sensorInterface->register_property(
        "isTimerOn", isTimerOn, [&](const bool newState, bool& oldState) {
            if (newState == oldState)
            {
                return 1;
            }
            oldState = newState;

            return 1;
    });

    //initial dbus service
    if (!sensorInterface->initialize())
    {
        std::cerr << "Error in initialization\n";
    }
}

FrbEventObject::~FrbEventObject()
{
    waitTimer.cancel();
    objServer.remove_interface(sensorInterface);
}

void FrbEventObject::runFrb3Algorithm()
{
    if (!isAcpiPowerOn())
    {
        isTimerOn = false;
        monitorTimes = 0;
        sensorInterface->set_property("isTimerOn", isTimerOn);
        return;
    }

    bool onFlag = isTimerOn;

    if (onFlag == true)
    {
        int frb3PIN = gpiod_ctxless_get_value("0", gpioNumber, NULL, NULL);

        if (1 == frb3PIN)
        {
            onFlag = false;
        }
        else
        {
            if (monitorTimes == timeOutSecond)
            {
                onFlag = false;

                uint16_t genId = 0x20;
                bool assert = true;
                std::string dbusPath = sensorPath;
                std::vector<uint8_t> eventData(3, 0xFF);
                eventData[0] = 0x4;
                sdbusplus::message::message selWrite = conn->new_method_call(
                    ipmiSELService, ipmiSELPath, ipmiSELAddInterface, "IpmiSelAdd");
                selWrite.append(ipmiSELAddMessage, dbusPath, eventData, assert, genId);
                try
                {
                    conn->call(selWrite);
                }
                catch (const sdbusplus::exception::SdBusError& e)
                {
                    std::cerr << "Failed to add FRB3 SEL\n";
                }
            }
        }

        if (onFlag == false)
        {
            monitorTimes = 0;
            isTimerOn = false;
            sensorInterface->set_property("isTimerOn", isTimerOn);
            return;
        }
    }
    else
    {
        std::cerr << "[FRB3] Unexpected loop! Exit\n";
        monitorTimes = 0;
        isTimerOn = false;
        sensorInterface->set_property("isTimerOn", isTimerOn);
        return;
    }

    monitorTimes++;
    waitTimer.expires_from_now(boost::asio::chrono::seconds(1));
    waitTimer.async_wait([&](boost::system::error_code ec) {
    if (ec == boost::asio::error::operation_aborted)
    {
        isTimerOn = false;
        monitorTimes = 0;
        sensorInterface->set_property("isTimerOn", isTimerOn);
        return; // we're being canceled
    }
        runFrb3Algorithm();
    });
}


void FrbEventObject::startFrb3Timer()
{
    if (isTimerOn == false)
    {
        std::cerr << "[FRB3] Timer Start\n";
        isTimerOn = true;
        monitorTimes = 0;
        sensorInterface->set_property("isTimerOn", isTimerOn);
        runFrb3Algorithm();
    }
}
