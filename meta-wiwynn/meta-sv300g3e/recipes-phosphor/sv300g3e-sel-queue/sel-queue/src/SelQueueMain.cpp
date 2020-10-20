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

#include <SelQueue.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/container/flat_set.hpp>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <systemd/sd-journal.h>
#include <sys/stat.h>
#include <unistd.h>

static boost::asio::io_service io;
static boost::asio::steady_timer filterTimer(io);
static boost::asio::steady_timer filterDelayTimer(io);
static boost::asio::steady_timer DelayTimer(io);
static sdbusplus::bus::bus reqBus = sdbusplus::bus::new_system();

static bool
    selAddQueue(const std::string &message, const std::string &path,
                const std::vector<uint8_t> &selData, const bool &assert,
                const uint16_t &genId, const uint8_t &mode)
{
    if(mode == SelMode::Normal)
    {
        struct SelInfo newSel = {message ,path ,selData ,assert ,genId};
        QSelInfo.push(newSel);
    }
    else if(mode == SelMode::Delay)
    {
        struct SelInfo newSel = {message ,path ,selData ,assert ,genId};
        QDelaySelDInfo.push(newSel);
    }
    else
    {
        throw std::invalid_argument("Invalid SEL add mode");
    }
    return true;
}


static void SendSel(void)
{
    while(!QSelInfo.empty())
    {
        // auto reqBus = sdbusplus::bus::new_system();

        struct SelInfo firstSel = QSelInfo.front();

        sdbusplus::message::message selWrite = reqBus.new_method_call(
                ipmiSELService, ipmiSELPath, ipmiSELAddInterface, "IpmiSelAdd");
        selWrite.append(firstSel.ipmiSELAddMessage, firstSel.dbusPath, 
            firstSel.eventData, firstSel.assert, firstSel.genId);
        try
        {
            reqBus.call_noreply(selWrite);
            QSelInfo.pop();
        }
        catch (sdbusplus::exception_t& e)
        {
            std::cerr << "Failed to log non-delay type SEL\n";
        }

    }

    // this implicitly cancels the timer
    filterTimer.expires_from_now(boost::asio::chrono::seconds(1));

    filterTimer.async_wait([&](const boost::system::error_code& ec) {
        if (ec == boost::asio::error::operation_aborted)
        {
            /* we were canceled*/
            return;
        }
        else if (ec)
        {
            std::cerr << "timer error\n";
            return;
        }
        SendSel();
        
    });
    return; 
}

static void SendDelaySel(void)
{
    if(!QDelaySelDInfo.empty())
    {
        filterDelayTimer.cancel();
        DelayTimer.expires_from_now(boost::asio::chrono::milliseconds(500));
        DelayTimer.async_wait([&](const boost::system::error_code& ec) {
            if (ec == boost::asio::error::operation_aborted)
            {
                /* we were canceled*/
                return;
            }
            else if (ec)
            {
                std::cerr << "timer error\n";
                return;
            }

            while(!QDelaySelDInfo.empty())
            {
                // auto reqBus = sdbusplus::bus::new_system();

                struct SelInfo firstSel = QDelaySelDInfo.front();

                sdbusplus::message::message selWrite = reqBus.new_method_call(
                        ipmiSELService, ipmiSELPath, ipmiSELAddInterface, "IpmiSelAdd");
                selWrite.append(firstSel.ipmiSELAddMessage, firstSel.dbusPath, 
                    firstSel.eventData, firstSel.assert, firstSel.genId);
                try
                {
                    reqBus.call_noreply(selWrite);
                    QDelaySelDInfo.pop();
                }
                catch (sdbusplus::exception_t& e)
                {
                    std::cerr << "Failed to log delay type SEL\n";
                }

            }

                // this implicitly cancels the timer
            filterDelayTimer.expires_from_now(boost::asio::chrono::seconds(1));

            filterDelayTimer.async_wait([&](const boost::system::error_code& ec) {
                if (ec == boost::asio::error::operation_aborted)
                {
                    /* we were canceled*/
                    return;
                }
                else if (ec)
                {
                    std::cerr << "timer error\n";
                    return;
                }
                SendDelaySel();

            });
        });

    }
    else
    {
        // this implicitly cancels the timer
        filterDelayTimer.expires_from_now(boost::asio::chrono::seconds(1));

        filterDelayTimer.async_wait([&](const boost::system::error_code& ec) {
            if (ec == boost::asio::error::operation_aborted)
            {
                /* we were canceled*/
                return;
            }
            else if (ec)
            {
                std::cerr << "timer error\n";
                return;
            }
            SendDelaySel();

        });
    }
    return;
}


int main(int argc, char *argv[])
{
    auto systemBus = std::make_shared<sdbusplus::asio::connection>(io);
    // sdbusplus::bus::bus internalBus = sdbusplus::bus::new_system();

    // IPMI SEL Object
    systemBus->request_name(ipmiSelQueueObject);
    auto server = sdbusplus::asio::object_server(systemBus);

    // Add SEL Interface
    std::shared_ptr<sdbusplus::asio::dbus_interface> ifaceAddQueue =
        server.add_interface(ipmiSelQueuePath, ipmiSelAddQueueInterface);

    // Add a new SEL entry
    ifaceAddQueue->register_method(
        "IpmiSelAddQueue", [](const std::string &message, const std::string &path,
                         const std::vector<uint8_t> &selData, const bool &assert, 
                         const uint16_t &genId, const uint8_t &mode) {
            return selAddQueue(message, path, selData, assert, genId, mode);
        });

    ifaceAddQueue->initialize();

    SendSel();

    SendDelaySel();

    io.run();

    return 0;
}
