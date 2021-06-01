/* Copyright 2020 MCT
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "lpc-interrupt-monitor.hpp"

#include <iostream>
#include "utils.hpp"
#include <xyz/openbmc_project/State/Host/server.hpp>
namespace State = sdbusplus::xyz::openbmc_project::State::server;

enum IpmiRestartCause
{
    Unknown = 0x00,
    ChassisCommandPowerOn = 0x01,
    ResetButton = 0x02,
    PowerButton = 0x03,
    WatchdogTimer = 0x04,
    PowerPolicyAlwaysOn = 0x06,
    PowerPolicyPreviousState = 0x07,
    SoftReset = 0x0a,
    ChassisCommandPowerCycle = 0x0c,
    ChassisCommandPowerReset = 0x0d,
};

static constexpr char const *ipmiSelService = "xyz.openbmc_project.Logging.IPMI";
static constexpr char const *ipmiSelPath = "/xyz/openbmc_project/Logging/IPMI";
static constexpr char const *ipmiSelAddInterface = "xyz.openbmc_project.Logging.IPMI";
static const std::string ipmiSysRestartSelAdd = "System Restart";

static const std::string sensorPathPrefix = "/xyz/openbmc_project/sensors/";


static const std::string restartCauseDefault = "xyz.openbmc_project.State.Host.RestartCause.Unknown";
static const std::string restartCauseChassisPWRON = "xyz.openbmc_project.State.Host.RestartCause.ChassisCommandPowerOn";
static const std::string restartCauseResetButton = "xyz.openbmc_project.State.Host.RestartCause.ResetButton";
static const std::string restartCausePowerButton = "xyz.openbmc_project.State.Host.RestartCause.PowerButton";
static const std::string restartCauseWatchdog = "xyz.openbmc_project.State.Host.RestartCause.WatchdogTimer";
static const std::string restartCausePowerPolicyAlwaysOn = "xyz.openbmc_project.State.Host.RestartCause.PowerPolicyAlwaysOn";
static const std::string restartCausePowerPolicyPrevious = "xyz.openbmc_project.State.Host.RestartCause.PowerPolicyPrevious";
static const std::string restartCauseSoftReset = "xyz.openbmc_project.State.Host.RestartCause.SoftReset";
static const std::string restartCauseChassisPWRCYCLE = "xyz.openbmc_project.State.Host.RestartCause.ChassisCommandPowerCycle";
static const std::string restartCauseChassisPWRRESET = "xyz.openbmc_project.State.Host.RestartCause.ChassisCommandPowerReset";

static constexpr bool DEBUG = false;

void propertyInitialize()
{
    registerMatch = 
    {
        lpc_rst,
        interrupt
    };
}

std::vector<std::string> findRegisterPath()
{
    std::vector<std::string> storedPath;
    std::string parentPath;

    std::vector<fs::path> registerPaths;
    if (!findFiles(fs::path("/sys/bus/platform/drivers/aspeed-bmc-misc"), registerMatch[0], registerPaths, 1))
    {
        if constexpr (DEBUG)
        {
            std::cerr << "Could not monitor register in system\n";
        }
        return storedPath;
    }

    for (const auto& registerPath : registerPaths)
    {
        std::ifstream registerFile(registerPath);
        if (!registerFile.good())
        {
            if constexpr (DEBUG)
            {
                std::cerr << "Failure finding register path " << registerPath << "\n";
            }
            continue;
        }
        parentPath = registerPath.parent_path().string();
        break;
    }

    for (const auto& registerName : registerMatch)
    {
        storedPath.push_back(parentPath+"/"+registerName);
    }
    
    return storedPath;
}

void interruptHandler(std::string lpcPath,std::string interruptPath)
{
    struct inotify_event *event = NULL;
    struct pollfd fds[2];
    char buf[BUF_LEN];
    nfds_t nfds;
    int fd, ret, status;

    fd = inotify_init1(IN_NONBLOCK);
    if (fd == -1)
    {
        if constexpr (DEBUG)
        {
            std::cerr << "Could not access the inotify API\n";
        }
        return;
    }

    ret = inotify_add_watch(fd, interruptPath.c_str(), IN_MODIFY);
    if (ret == -1)
    {
        if constexpr (DEBUG)
        {
            std::cerr << "Cannot watch " << interruptPath <<" \n";
        }
        close(fd);
        return;
    }

    nfds = 2;
    fds[0].fd = STDIN_FILENO;
    fds[0].events = 0;
    fds[1].fd = fd;
    fds[1].events = POLLIN;

    while (1)
    {
        ret = poll(fds, nfds, -1);
        if (ret > 0)
        {
            if (fds[1].revents & POLLIN)
            {
                int len = read(fd, buf, BUF_LEN);
                char* p = buf;
                while (p < buf + len) {
                    event = (struct inotify_event*)p;
                    uint32_t mask = event->mask;
                    if (mask & IN_MODIFY) {
                        if constexpr (DEBUG)
                        {
                            std::cerr<<"LPC interrupt occurred\n";
                        }
                        status = readFileValue(lpcPath);
                        if(status)
                        {
                            interruptAction(lpcPath,status);
                        }
                    }
                    p += sizeof(struct inotify_event) + event->len;
                }
            }
        }
    }
    close(fd);
}

void registerHandler(boost::asio::io_context& io,double delay)
{
    static boost::asio::steady_timer timer(io);

    timer.expires_after(std::chrono::microseconds((long)delay));

    timer.async_wait([&io](const boost::system::error_code&) 
    {
        if(findPath.empty())
        {
            findPath = findRegisterPath();
            if(registerMatch.size()!=findPath.size())
            {
                findPath.clear();
                registerHandler(io, 1*MICRO_OFFSET);
                return; 
            }
        }

        std::vector<std::string>::iterator element;

        element = find(registerMatch.begin(), registerMatch.end(), lpc_rst);
        int lpcPosition = distance(registerMatch.begin(),element);

        element = find(registerMatch.begin(), registerMatch.end(), interrupt);
        int interruptPosition = distance(registerMatch.begin(),element);

        interruptHandler(findPath.at(lpcPosition),findPath.at(interruptPosition));

        registerHandler(io,0);
        return;
    });
}

void interruptAction(std::string lpcPath, int status)
{   
    constexpr auto objectPath = "/";
    constexpr auto interface = "org.freedesktop.DBus";
    constexpr auto signal = "LpcReset";
    uint8_t responseStatus = status & 0x01;

    auto bus = sdbusplus::bus::new_default();
    auto msg = bus.new_signal(objectPath, interface, signal);
    uint32_t parameter = responseStatus;
    msg.append(parameter);
    msg.signal_send();
    writeFileValue(lpcPath,0);

    // Check pgood status
    auto method = bus.new_method_call("org.openbmc.control.Power",
                    "/org/openbmc/control/power0",
                    "org.freedesktop.DBus.Properties", "Get");

    method.append("org.openbmc.control.Power", "pgood");
    try
    {
        std::variant<int> state;
        auto reply = bus.call(method);
        reply.read(state);

        if(std::get<int>(state))
        {
            std::cerr << "Platform Reset!!!\n";

            // SEL Add
            uint16_t genId = 0x20;
            std::vector<uint8_t> eventData(3, 0xFF);
            std::string dbusPath = sensorPathPrefix + "restart/SYSTEM_RESTART";
            bool assert = true;

            /* Sensor type: System Restart (0x1d)
                Sensor specific offset: 07h - Intended to be used with Event Data 2 and or 3
            */
            eventData.at(0) = 0x7;
            eventData.at(1) = getRestartCause();

            sdbusplus::message::message writeSEL = bus.new_method_call(
                ipmiSelService, ipmiSelPath, ipmiSelAddInterface, "IpmiSelAdd");
            writeSEL.append(ipmiSysRestartSelAdd, dbusPath, eventData, assert, genId);

            try
            {
                bus.call_noreply(writeSEL);
            }
            catch (sdbusplus::exception_t& e)
            {
                std::cerr<<"failed to log system restart SEL\n";
            }

            setRestartCauseDefault();
        }
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        std::cerr << "Not able to get pgood property\n";
    }

}

uint8_t getRestartCause()
{
    // Bus for system control
    auto bus = sdbusplus::bus::new_system();

    // Get restart cause from dbus
    auto method = bus.new_method_call("xyz.openbmc_project.State.Host",
                    "/xyz/openbmc_project/state/host0",
                    "org.freedesktop.DBus.Properties", "Get");

    method.append("xyz.openbmc_project.State.Host", "RestartCause");

    uint8_t result = 0x0;

    try
    {
        std::variant<std::string> dbusCause;
        auto reply = bus.call(method);
        reply.read(dbusCause);

        std::cerr<<"Restart Cause: "<< std::get<std::string>(dbusCause) << "\n";

        std::string strCause = std::get<std::string>(dbusCause);
        auto cause = State::Host::convertRestartCauseFromString(strCause);

        switch(cause)
        {
            case State::Host::RestartCause::ChassisCommandPowerOn: 
                return IpmiRestartCause::ChassisCommandPowerOn;
            case State::Host::RestartCause::ResetButton: 
                return IpmiRestartCause::ResetButton;
            case State::Host::RestartCause::PowerButton: 
                return IpmiRestartCause::PowerButton;
            case State::Host::RestartCause::WatchdogTimer: 
                return IpmiRestartCause::WatchdogTimer;
            case State::Host::RestartCause::PowerPolicyAlwaysOn: 
                return IpmiRestartCause::PowerPolicyAlwaysOn;
            case State::Host::RestartCause::PowerPolicyPreviousState: 
                return IpmiRestartCause::PowerPolicyPreviousState;
            case State::Host::RestartCause::Unknown: 
                return IpmiRestartCause::SoftReset;
            case State::Host::RestartCause::ChassisCommandPowerCycle: 
                return IpmiRestartCause::ChassisCommandPowerCycle;
            case State::Host::RestartCause::ChassisCommandPowerReset: 
                return IpmiRestartCause::ChassisCommandPowerReset;
            default:
                return IpmiRestartCause::Unknown;
        }

    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        std::cerr << "Not able to get restart cause property\n";
    }
}

void setRestartCauseDefault()
{
    // Bus for system control
    auto bus = sdbusplus::bus::new_system();

    // Get restart cause from dbus
    auto method = bus.new_method_call("xyz.openbmc_project.State.Host",
                    "/xyz/openbmc_project/state/host0",
                    "org.freedesktop.DBus.Properties", "Set");

    method.append("xyz.openbmc_project.State.Host", "RestartCause", std::variant<std::string>(restartCauseDefault));

    try
    {
        bus.call_noreply(method);
    }
    catch (sdbusplus::exception_t& e)
    {
        std::cerr<<"failed to set restart cause to default\n";
    }
}
int main(int argc, char *argv[])
{
    boost::asio::io_context io;
    bus = std::make_shared<sdbusplus::asio::connection>(io);

    propertyInitialize();

    io.post(
        [&]() { registerHandler(io, 0); });
    
    io.run();
    return 0;
}
