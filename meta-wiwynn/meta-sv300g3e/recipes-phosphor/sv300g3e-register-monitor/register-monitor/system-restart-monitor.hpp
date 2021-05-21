#pragma once

#include <iostream>
#include <vector>
#include <variant>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <climits>
#include <unistd.h>
#include <sys/inotify.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/server.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/log.hpp>

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

namespace fs = std::filesystem;
using namespace phosphor::logging;

static constexpr auto lpc_rst = "lpc-rst";
static constexpr auto interrupt = "interrupt";

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

static std::vector<std::string> registerMatch;
static std::vector<std::string> findPath;


void propertyInitialize();
std::vector<std::string> findRegisterPath();
void interruptAction(std::string lpcPath);
void interruptHandler(std::string lpcPath, std::string interruptPath);
void findLpcIntFiles();
uint8_t getRestartCause();
void setRestartCauseDefault();
