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

#include "TimeSync.hpp"

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

static boost::asio::io_service io;
static boost::asio::steady_timer waitTimer(io);
static sdbusplus::bus::bus internalBus =
                  sdbusplus::bus::new_system();
static std::shared_ptr<sdbusplus::asio::dbus_interface> METimeInterface;

static uint32_t meTimestamp = 0;
static bool isTimesyncOK = false;
static unsigned int pollingSec = 10;
static unsigned int retrySec = 2;

static bool getJsonParameter()
{
    std::ifstream ConfigFile(configPath);
    if(!ConfigFile)
    {
        std::cerr << "Failed to open config file [" << configPath <<"]\n";
        return false;
    }

    auto data = nlohmann::json::parse(ConfigFile, nullptr, false);
    if(data.is_discarded())
    {
        std::cerr << "Syntax error in [" << configPath << "]\n";
        return false;
    }

    pollingSec = data["FreqOfPollSyncSeconds"];
    retrySec = data["MESyncFailRetrySeconds"];

    // sd_journal_print(LOG_INFO, "Polling interval: %d Seconds.", pollingSec);
    // sd_journal_print(LOG_INFO, "Retry interval: %d Seconds.", retrySec);

    return true;
}

static bool syncTimeWithME()
{
    std::tuple<int32_t, uint8_t, uint8_t, uint8_t, uint8_t, std::vector<uint8_t>> cmdResponses;

    auto mesg = internalBus.new_method_call(ipmbChannelService, ipmbChannelPath,
                                     ipmbChannelInterface , ipmbChannelMethod);
    mesg.append(commandAddress, netfn, lun, command, commandData);
    try
    {
        auto reply = internalBus.call(mesg);
        reply.read(cmdResponses);
    }
    catch (sdbusplus::exception_t& e)
    {
        sd_journal_print(LOG_ERR, "Failed to sync Time with ME:%s\n", e.name());
        return false;
    }

    std::vector<uint8_t> dataReceived(4, 0);
    int32_t status = -1;
    uint8_t netFn = 0;
    uint8_t rslun = 0;
    uint8_t cmd = 0;
    uint8_t cc = 0;

    std::tie(status, netFn, rslun, cmd, cc, dataReceived) = cmdResponses;
    if (ipmiCmdCcOk != cc)
    {
        sd_journal_print(LOG_ERR, "[0x%x] Invalid ME Get Time CMD CC \n", cc);
        return false;
    }

    if (4 != dataReceived.size())
    {
        sd_journal_print(LOG_ERR, "[%d] Invalid ME Get Time CMD response size\n", dataReceived.size());
        return false;
    }

    sd_journal_print(LOG_INFO, "ME Get Time Data [0x%02x:0x%02x:0x%02x:0x%02x]\n",
        dataReceived.at(0), dataReceived.at(1), dataReceived.at(2),dataReceived.at(3));

    uint32_t selTime = static_cast<uint32_t>(dataReceived.at(3) << 24) |
                                            (dataReceived.at(2) << 16) |
                                            (dataReceived.at(1) << 8) |
                                            (dataReceived.at(0));
    struct timespec selTimeBuf = {};
    selTimeBuf.tv_sec = selTime;
    meTimestamp = selTime;

    if (clock_settime(CLOCK_REALTIME, &selTimeBuf) < 0)
    {
        sd_journal_print(LOG_ERR, "Failed to set BMC system time");
        return false;
    }

    return true;
}

void runMETimeSyncLoop()
{
    unsigned int interval = pollingSec;
    if (isTimesyncOK == false)
    {
        interval = retrySec;
    }
    // sd_journal_print(LOG_INFO, "Next ME Sync: %d seconds.", interval);

    waitTimer.expires_from_now(boost::asio::chrono::seconds(interval));
    waitTimer.async_wait([&](const boost::system::error_code& ec) {
        if (ec == boost::asio::error::operation_aborted)
        {
            return; // we're being canceled
        }
        else if (ec)
        {
            sd_journal_print(LOG_ERR, "Timer Error!");
            return;
        }

        isTimesyncOK = syncTimeWithME();
        if (isTimesyncOK == true)
        {
            METimeInterface->set_property("METimestamp", meTimestamp);
        }

        runMETimeSyncLoop();
    });
}

int main(int argc, char *argv[])
{
    // get parameter from config
    if (!getJsonParameter())
    {
        sd_journal_print(LOG_ERR, "Failed to get json config");
        return -1;
    }

    // Time sync with ME
    isTimesyncOK = syncTimeWithME();

    //set busctl property
    static auto systemBus = std::make_shared<sdbusplus::asio::connection>(io);
    systemBus->request_name("xyz.openbmc_project.METimeSync");
    sdbusplus::asio::object_server objectServer(systemBus);
    std::string dbusPath = "/xyz/openbmc_project/METimeSync";
    METimeInterface = objectServer.add_interface(dbusPath,
                                "xyz.openbmc_project.METimeSync.Value");

    // ME time
    METimeInterface->register_property("METimestamp", meTimestamp,
    [&](const uint32_t& newValue, uint32_t& oldValue) {
        oldValue = newValue;
        return true;
    });

    if (!METimeInterface->initialize())
    {
        sd_journal_print(LOG_ERR, "Failed to initialize dbus interface");
        return -1;
    }

    // Time Sync Polling
    runMETimeSyncLoop();

    io.run();

    return 0;
}
