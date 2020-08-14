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
#include <chrono>

FastProchotObject::FastProchotObject(boost::asio::io_service& io,
    std::shared_ptr<sdbusplus::asio::connection>& conn,
    gpiod_line* line, gpiod_line_request_config& config,
    int gpioNumber, std::string netName):
    gpioLine(line), gpioConfig(config), gpioEventDescriptor(io),
    conn(conn), gpioNumber(gpioNumber), netName(netName), waitTimer(io)
{
    int res = requestFastProchotEvent();
}

FastProchotObject::~FastProchotObject()
{
    gpiod_line_release(gpioLine);
}

void FastProchotObject::waitFastProchotEvent()
{
    gpioEventDescriptor.async_wait(
        boost::asio::posix::stream_descriptor::wait_read,
        [this](const boost::system::error_code& ec) {
            if (ec)
            {
                std::cerr << "GPIO event read error: "<< netName <<"\n";
                return;
            }
            fastProchotEventHandler();
        });
}

void FastProchotObject::fastProchotEventHandler()
{
    gpiod_line_event gpioLineEvent;
    int res = gpiod_line_event_read_fd(gpioEventDescriptor.native_handle(), &gpioLineEvent);
    if (res < 0)
    {
        std::cerr << "Failed to read gpioLineEvent from fd: "<< netName <<"\n";
    }

    addFastProchotEventSEL();
    waitFastProchotEvent();

    return;
}

int FastProchotObject::requestFastProchotEvent()
{
    if (gpiod_line_request(gpioLine, &gpioConfig, 0) < 0)
    {
        std::cerr << "Failed to request fast prohot gpio line: "<< netName <<"\n";
        return -1;
    }

    int gpioLineFd = gpiod_line_event_get_fd(gpioLine);
    if (gpioLineFd < 0)
    {
        std::cerr << "Failed to get line fd: "<< netName <<"\n";
        return -1;
    }

    gpioEventDescriptor.assign(gpioLineFd);

    waitFastProchotEvent();

    return 0;
}

void FastProchotObject::addFastProchotEventSEL()
{
    uint16_t genId = 0x20;
    bool assert = true;
    std::string dbusPath = sensorPathPrefix + "Fast_Prochot";
    std::vector<uint8_t> eventData(3, 0xFF);
    eventData.at(0) = gpioNumber & 0xFF;

    sdbusplus::message::message selWrite = conn->new_method_call(
            ipmiSELService, ipmiSELPath, ipmiSELAddInterface, "IpmiSelAdd");
    selWrite.append(ipmiSELAddMessage, dbusPath, eventData, assert, genId);

    try
    {
        conn->call_noreply(selWrite);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        std::cerr << "Error in " << __func__ << "\n";
    }

    return;
}
