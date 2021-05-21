#include "system-restart-monitor.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include "utils.hpp"
#include <xyz/openbmc_project/State/Host/server.hpp>

namespace State = sdbusplus::xyz::openbmc_project::State::server;

static boost::asio::io_context io;
static boost::asio::steady_timer timer(io);

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
        std::cerr << "Could not monitor register in system\n";
        return storedPath;
    }

    for (const auto& registerPath : registerPaths)
    {
        std::ifstream registerFile(registerPath);
        if (!registerFile.good())
        {
            std::cerr << "Failure finding register path " << registerPath << "\n";
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

void interruptAction(std::string lpcPath)
{
    writeFileValue(lpcPath,0);

    // Bus for system control
    auto bus = sdbusplus::bus::new_system();

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
            std::string dbusPath = sensorPathPrefix + "systemrestart/System_Restart";
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

void interruptHandler(std::string lpcPath, std::string interruptPath)
{
    struct inotify_event *event = NULL;
    struct pollfd fdpoll;
    char buf[BUF_LEN];
    nfds_t nfds;
    int fd = -1;
    int wd = -1;

    fd = inotify_init1(IN_NONBLOCK);
    if (fd < 0)
    {
        std::cerr << "Could not access the inotify API\n";
        return;
    }

    wd = inotify_add_watch(fd, interruptPath.c_str(), IN_MODIFY);
    if (wd < 0)
    {
        std::cerr << "Cannot watch " << interruptPath <<" \n";
        close(fd);
        return;
    }

    nfds = 1;
    fdpoll.fd = fd;
    fdpoll.events = POLLIN;

    while (1)
    {
        int ret = -1;
        int status = 0;
        ret = poll(&fdpoll, nfds, -1);

        if (ret > 0)
        {
            if (fdpoll.revents & POLLIN)
            {
                int len = read(fd, buf, BUF_LEN);
                char* p = buf;
                while (p < buf + len) 
                {
                    event = (struct inotify_event*)p;
                    uint32_t mask = event->mask;

                    if (mask & IN_MODIFY) 
                    {
                        status = readFileValue(lpcPath);

                        if(status)
                        {
                            std::this_thread::sleep_for(boost::asio::chrono::seconds(2));
                            interruptAction(lpcPath);                   
                        }
                    }
                    p += sizeof(struct inotify_event) + event->len;
                }
            }
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);

    return;
}

void findLpcIntFiles()
{
    timer.expires_after(boost::asio::chrono::seconds(2));
    timer.async_wait([&](const boost::system::error_code& ec) {
        if (ec == boost::asio::error::operation_aborted)
        {
            return; // we're being canceled
        }

        if(findPath.empty())
        {
            findPath = findRegisterPath();
            if(registerMatch.size() != findPath.size())
            {
                findPath.clear();
                findLpcIntFiles();
                return;
            }
        }

        std::vector<std::string>::iterator element;

        element = find(registerMatch.begin(), registerMatch.end(), lpc_rst);
        int lpcPosition = distance(registerMatch.begin(),element);

        element = find(registerMatch.begin(), registerMatch.end(), interrupt);
        int interruptPosition = distance(registerMatch.begin(),element);

        interruptHandler(findPath.at(lpcPosition),findPath.at(interruptPosition));

        return;
    });
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
    auto systemBus = std::make_shared<sdbusplus::asio::connection>(io);

    propertyInitialize();
    findLpcIntFiles();

    io.run();
    return 0;
}
