/*
// Copyright (c) 2019 Wiwynn Corporation
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

#include "xyz/openbmc_project/Common/error.hpp"
#include "xyz/openbmc_project/Led/Physical/server.hpp"

#include <systemd/sd-journal.h>

#include <array>
#include <boost/container/flat_map.hpp>
#include <boost/process/child.hpp>
#include <boost/process/io.hpp>
#include <filesystem>
#include <iostream>
#include <ipmid/api.hpp>
#include <ipmid/utils.hpp>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/message/types.hpp>
#include <sdbusplus/server/object.hpp>
#include <sdbusplus/timer.hpp>
#include <string>
#include <variant>
#include <vector>
#include "oemcommands.hpp"
#include "Utils.hpp"

const static constexpr char* solPatternService = "xyz.openbmc_project.SolPatternSensor";
const static constexpr char* solPatternInterface = "xyz.openbmc_project.Sensor.SOLPattern";
const static constexpr char* solPatternObjPrefix = "/xyz/openbmc_project/sensors/pattern/Pattern";

const static constexpr char* leakyBucketService = "xyz.openbmc_project.LeakyBucket";
const static constexpr char* thresholdObjPath = "/xyz/openbmc_project/leaky_bucket/threshold";
const static constexpr char* thresholdInterface = "xyz.openbmc_project.LeakyBucket.threshold";

static void register_oem_functions() __attribute__((constructor));

/**
 *  @brief Function of setting fan speed duty.
 *  @brief NetFn: 0x30, Cmd: 0x11
 *
 *  @param[in] pwmIndex - Index of PWM.
 *  @param[in] pwmValue - PWM value for the specified index.
 *
 *  @return Size of command response - Completion Code.
 **/
ipmi_ret_t IpmiSetPwm(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                      ipmi_request_t request, ipmi_response_t response,
                      ipmi_data_len_t dataLen, ipmi_context_t context)
{
    if (*dataLen != sizeof(SetPwmCmdRequest))
    {
        sd_journal_print(LOG_ERR,
                         "IPMI SetPwm request data len invalid, "
                         "received: %d, required: %d\n",
                         *dataLen, sizeof(SetPwmCmdRequest));
        *dataLen = 0;
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    SetPwmCmdRequest* req = reinterpret_cast<SetPwmCmdRequest*>(request);
    *dataLen = 0;

    constexpr auto parentPwmDir = "/sys/devices/platform/ahb/ahb:apb/"
                                  "1e786000.pwm-tacho-controller/hwmon/";

    // Find the directory that stores pwm file
    auto pwmDirVec = getDirFiles(parentPwmDir, "hwmon[0-9]+");
    if (pwmDirVec.size() == 0)
    {
        sd_journal_print(LOG_ERR,
                         "IPMI SetPwm failed in getting pwm file directory."
                         "Pwm directory not found\n");
        return IPMI_CC_UNSPECIFIED_ERROR;
    }
    else if (pwmDirVec.size() > 1)
    {
        sd_journal_print(LOG_ERR,
                         "IPMI SetPwm failed in getting pwm file directory."
                         "Found more than one pwm directory\n");
        return IPMI_CC_UNSPECIFIED_ERROR;
    }

    // Determine which pwm file we are going to write
    if (req->pwmIndex == pwmFileTable.size())
    {
        for (uint8_t i = 0; i < pwmFileTable.size(); i++)
        {
            auto pwmFile = pwmFileTable.find(i);
            if (pwmFile == pwmFileTable.end())
            {
                sd_journal_print(LOG_ERR,
                                "IPMI SetPwm invalid field request(pwmIndex), "
                                "received = 0x%x, required = 0x00 ~ 0x06\n",
                                req->pwmIndex);
                return IPMI_CC_PARM_OUT_OF_RANGE;
            }
            auto pwmFilePath = pwmDirVec[0] + "/" + pwmFile->second;

            // Open the pwm file
            std::ofstream fout(pwmFilePath);
            if (fout.fail())
            {
                sd_journal_print(LOG_ERR, "IPMI SetPwm Failed in open pwm file\n");
                return IPMI_CC_UNSPECIFIED_ERROR;
            }

            // Scale input pwm value from 0~100 to 0~255
            double scaledPwmValue = 0;
            if ((req->pwmValue >= 10) && (req->pwmValue <= 100))
            {
                scaledPwmValue = ((req->pwmValue * 255) / 100);
            }
            else
            {
                sd_journal_print(LOG_ERR,
                                "IPMI SetPwm invalid field request(pwmValue), "
                                "received = 0x%x, required = 0x0a ~ 0x64\n",
                                req->pwmValue);
                return IPMI_CC_PARM_OUT_OF_RANGE;
            }

            // Write pwm value to pwm file
            fout << static_cast<int64_t>(scaledPwmValue);
            if (fout.fail())
            {
                sd_journal_print(LOG_ERR,
                                "IPMI SetPwm failed to write the pwm file\n");
                fout.close();
                return IPMI_CC_UNSPECIFIED_ERROR;
            }

            fout.close();
        }
    }
    else
    {
        auto pwmFile = pwmFileTable.find(req->pwmIndex);
        if (pwmFile == pwmFileTable.end())
        {
            sd_journal_print(LOG_ERR,
                            "IPMI SetPwm invalid field request(pwmIndex), "
                            "received = 0x%x, required = 0x00 ~ 0x06\n",
                            req->pwmIndex);
            return IPMI_CC_PARM_OUT_OF_RANGE;
        }
        auto pwmFilePath = pwmDirVec[0] + "/" + pwmFile->second;

        // Open the pwm file
        std::ofstream fout(pwmFilePath);
        if (fout.fail())
        {
            sd_journal_print(LOG_ERR, "IPMI SetPwm Failed in open pwm file\n");
            return IPMI_CC_UNSPECIFIED_ERROR;
        }

        // Scale input pwm value from 0~100 to 0~255
        double scaledPwmValue = 0;
        if ((req->pwmValue >= 10) && (req->pwmValue <= 100))
        {
            scaledPwmValue = ((req->pwmValue * 255) / 100);
        }
        else
        {
            sd_journal_print(LOG_ERR,
                            "IPMI SetPwm invalid field request(pwmValue), "
                            "received = 0x%x, required = 0x0a ~ 0x64\n",
                            req->pwmValue);
            return IPMI_CC_PARM_OUT_OF_RANGE;
        }

        // Write pwm value to pwm file
        fout << static_cast<int64_t>(scaledPwmValue);
        if (fout.fail())
        {
            sd_journal_print(LOG_ERR,
                            "IPMI SetPwm failed to write the pwm file\n");
            fout.close();
            return IPMI_CC_UNSPECIFIED_ERROR;
        }

        fout.close();
    }
    return IPMI_CC_OK;
}

/**
 *  @brief Function of setting fan speed control mode.
 *  @brief NetFn: 0x30, Cmd: 0x12
 *
 *  @param[in] ControlMode - Manual mode or auto mode.
 *
 *  @return Size of command response - Completion Code.
 **/
ipmi_ret_t IpmiSetFscMode(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                          ipmi_request_t request, ipmi_response_t response,
                          ipmi_data_len_t dataLen, ipmi_context_t context)
{
    if (*dataLen != sizeof(SetFscModeCmdRequest))
    {
        sd_journal_print(LOG_ERR,
                         "IPMI CtrlFanSpeed request data len invalid, "
                         "received: %d, required: %d\n",
                         *dataLen, sizeof(SetFscModeCmdRequest));
        *dataLen = 0;
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    SetFscModeCmdRequest* req = reinterpret_cast<SetFscModeCmdRequest*>(request);
    *dataLen = 0;

    constexpr auto modeService = "xyz.openbmc_project.State.FanCtrl";
    constexpr auto modeRoot = "/xyz/openbmc_project/settings/fanctrl";
    constexpr auto modeIntf = "xyz.openbmc_project.Control.Mode";
    constexpr auto propIntf = "org.freedesktop.DBus.Properties";

    // Bus for system control
    auto bus = sdbusplus::bus::new_default_system();

    // Get all zones mode object path
    DbusSubTree zonesPath = getSubTree(bus, modeRoot, 1, modeIntf);
    if(zonesPath.empty() == true)
    {
        sd_journal_print(LOG_ERR,
                         "IPMI CtrlFanSpeed GetSubTree "
                         "can not get zones object path\n");
        return IPMI_CC_UNSPECIFIED_ERROR;
    }

    // Value to open/close manual mode
    sdbusplus::message::variant<bool> value;
    if (req->mode == FSC_MODE_MANUAL)
    {
        value = true;
    }
    else if (req->mode == FSC_MODE_AUTO)
    {
        value = false;
    }
    else
    {
        sd_journal_print(LOG_ERR,
                         "IPMI SetFSCMode invalid field request, "
                         "received: %d, require: 0x00(Manual) or 0x01(Auto)\n",
                         req->mode);
        return IPMI_CC_INVALID_FIELD_REQUEST;
    }

    for (auto& path : zonesPath)
    {
        auto msg = bus.new_method_call(modeService, path.first.c_str(), propIntf, "Set");
        msg.append(modeIntf, "Manual", value);

        try
        {
            bus.call_noreply(msg);
        }
        catch (const sdbusplus::exception::SdBusError& e)
        {
            sd_journal_print(LOG_ERR,
                             "IPMI CtrlFanSpeed Failed in call method, %s\n",
                             e.what());
            return IPMI_CC_UNSPECIFIED_ERROR;
        }
    }

    return IPMI_CC_OK;
}

/*
    Set SOL pattern func
    NetFn: 0x3E / CMD: 0xB2
*/
ipmi_ret_t ipmiSetSolPattern(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                             ipmi_request_t request, ipmi_response_t response,
                             ipmi_data_len_t data_len, ipmi_context_t context)
{
    SetSolPatternCmdReq* reqData = reinterpret_cast<SetSolPatternCmdReq*>(request);

    int32_t reqDataLen = (int32_t)*data_len;
    *data_len = 0;

    /* Data Length
       Pattern Number (1) + Max length (256) */
    if((reqDataLen == 0) || (reqDataLen > (maxPatternLen+1)))
    {
        sd_journal_print(LOG_CRIT, "[%s] invalid cmd data length %d\n",
                         __FUNCTION__, reqDataLen);
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    /* Pattern Number */
    uint8_t patternNum = reqData->patternIdx;
    if((patternNum < 1) || (patternNum > 4))
    {
        sd_journal_print(LOG_CRIT, "[%s] invalid pattern number %d\n",
                         __FUNCTION__, patternNum);
        return IPMI_CC_INVALID_FIELD_REQUEST;
    }

    /* Copy the Pattern String */
    char tmpPattern[maxPatternLen+1];
    memset(tmpPattern, '\0', maxPatternLen+1);
    memcpy(tmpPattern, reqData->data, (reqDataLen-1));
    std::string patternData = tmpPattern;

    /* Set pattern to dbus */
    std::string solPatternObjPath = solPatternObjPrefix + std::to_string((patternNum));

    std::shared_ptr<sdbusplus::asio::connection> dbus = getSdBus();
    try
    {
        ipmi::setDbusProperty(*dbus, solPatternService, solPatternObjPath,
                               solPatternInterface, "Pattern", patternData);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        return IPMI_CC_UNSPECIFIED_ERROR;
    }

    return IPMI_CC_OK; 
}

/*
    Get SOL pattern func
    NetFn: 0x3E / CMD: 0xB3
*/
ipmi_ret_t ipmiGetSolPattern(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                             ipmi_request_t request, ipmi_response_t response,
                             ipmi_data_len_t data_len, ipmi_context_t context)
{
    GetSolPatternCmdReq* reqData = reinterpret_cast<GetSolPatternCmdReq*>(request);
    GetSolPatternCmdRes* resData = reinterpret_cast<GetSolPatternCmdRes*>(response);

    int32_t reqDataLen = (int32_t)*data_len;
    *data_len = 0;

    /* Data Length
       Pattern Number (1) */
    if(reqDataLen != 1)
    {
        sd_journal_print(LOG_CRIT, "[%s] invalid cmd data length %d\n",
                         __FUNCTION__, reqDataLen);
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    /* Pattern Number */
    uint8_t patternNum = reqData->patternIdx;
    if((patternNum < 1) || (patternNum > 4))
    {
        sd_journal_print(LOG_CRIT, "[%s] invalid pattern number %d\n",
                         __FUNCTION__, patternNum);
        return IPMI_CC_INVALID_FIELD_REQUEST;
    }

    /* Get pattern to dbus */
    std::string solPatternObjPath = solPatternObjPrefix + std::to_string((patternNum));
    std::string patternData;

    std::shared_ptr<sdbusplus::asio::connection> dbus = getSdBus();
    try
    {
        auto value = ipmi::getDbusProperty(*dbus, solPatternService, solPatternObjPath,
                               solPatternInterface, "Pattern");
        patternData = std::get<std::string>(value);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        return IPMI_CC_UNSPECIFIED_ERROR;
    }

    /* Invalid pattern length */
    int32_t resDataLen = patternData.size();

    if(resDataLen > maxPatternLen)
    {
        return IPMI_CC_UNSPECIFIED_ERROR;
    }

    memcpy(resData->data, patternData.data(), resDataLen);
    *data_len = resDataLen;

    return IPMI_CC_OK;
}

/*
    Set Leaky Bucket threshold func
    NetFn: 0x3E / CMD: 0xB4
*/
ipmi_ret_t ipmiSetLbaThreshold(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                               ipmi_request_t request, ipmi_response_t response,
                               ipmi_data_len_t data_len, ipmi_context_t context)
{
    int32_t reqDataLen = (int32_t)*data_len;
    *data_len = 0;

    /* Data Length -
       threshold number (1) + threshold value (2) */
    if(reqDataLen != (sizeof(SetLbaThresholdCmdReq)))
    {
        sd_journal_print(LOG_CRIT, "[%s] invalid cmd data length %d\n",
                         __FUNCTION__, reqDataLen);
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    SetLbaThresholdCmdReq* reqData = reinterpret_cast<SetLbaThresholdCmdReq*>(request);

    /* Threshold index */
    uint8_t thresholdNum = reqData->thresholdIdx;
    if((thresholdNum < 1) || (thresholdNum > maxLbaThresholdNum))
    {
        sd_journal_print(LOG_CRIT, "[%s] invalid threshold number %d\n",
                         __FUNCTION__, thresholdNum);
        return IPMI_CC_INVALID_FIELD_REQUEST;
    }

    /* T1 & T3 can not be 0 */
    uint16_t thresholdData = reqData->thresholdVal;
    if((thresholdData == 0) &&
      ((thresholdNum == 1) || (thresholdNum == 3)))
    {
        sd_journal_print(LOG_CRIT, "[%s] invalid threshold T%d value %d\n",
                         __FUNCTION__, thresholdNum, thresholdData);
        return IPMI_CC_INVALID_FIELD_REQUEST;
    }

    /* Set the threshold property through dbus */
    std::string thresholdString = "Threshold" + std::to_string(thresholdNum);

    std::shared_ptr<sdbusplus::asio::connection> dbus = getSdBus();
    try
    {
        ipmi::setDbusProperty(*dbus, leakyBucketService, thresholdObjPath,
                               thresholdInterface, thresholdString.c_str(), thresholdData);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        return IPMI_CC_UNSPECIFIED_ERROR;
    }

    return IPMI_CC_OK;
}

/*
    Get Leaky Bucket threshold func
    NetFn: 0x3E / CMD: 0xB5
*/
ipmi_ret_t ipmiGetLbaThreshold(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                               ipmi_request_t request, ipmi_response_t response,
                               ipmi_data_len_t data_len, ipmi_context_t context)
{
    int32_t reqDataLen = (int32_t)*data_len;
    *data_len = 0;

    /* Data Length - threshold number (1) */
    if(reqDataLen != 1)
    {
        sd_journal_print(LOG_CRIT, "[%s] invalid cmd data length %d\n",
                         __FUNCTION__, reqDataLen);
        return IPMI_CC_REQ_DATA_LEN_INVALID;
    }

    GetLbaThresholdCmdReq* reqData = reinterpret_cast<GetLbaThresholdCmdReq*>(request);
    GetLbaThresholdCmdRes* resData = reinterpret_cast<GetLbaThresholdCmdRes*>(response);

    /* Threshold index */
    uint8_t thresholdNum = reqData->thresholdIdx;
    if((thresholdNum < 1) || (thresholdNum > maxLbaThresholdNum))
    {
        sd_journal_print(LOG_CRIT, "[%s] invalid threshold number %d\n",
                         __FUNCTION__, thresholdNum);
        return IPMI_CC_INVALID_FIELD_REQUEST;
    }

    /* Get the threshold property through dbus */
    std::string thresholdString = "Threshold" + std::to_string(thresholdNum);
    uint16_t thresholdData;

    std::shared_ptr<sdbusplus::asio::connection> dbus = getSdBus();
    try
    {
        auto value = ipmi::getDbusProperty(*dbus, leakyBucketService, thresholdObjPath,
                               thresholdInterface, thresholdString.c_str());
        thresholdData = std::get<uint16_t>(value);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        return IPMI_CC_UNSPECIFIED_ERROR;
    }

    *data_len = sizeof(thresholdData);
    resData->thresholdVal = thresholdData;

    return IPMI_CC_OK;
}

static void register_oem_functions(void)
{
    // <Set Fan PWM>
    ipmi_register_callback(netFnSv300g3eOEM2, CMD_SET_FAN_PWM,
                           NULL, IpmiSetPwm, PRIVILEGE_USER);

    // <Set FSC Mode>
    ipmi_register_callback(netFnSv300g3eOEM2, CMD_SET_FSC_MODE,
                           NULL, IpmiSetFscMode, PRIVILEGE_USER);

    // <Set SOL Pattern>
    ipmi_register_callback(netFnSv300g3eOEM3, CMD_SET_SOL_PATTERN,
                           NULL, ipmiSetSolPattern, PRIVILEGE_USER);

    // <Get SOL Pattern>
    ipmi_register_callback(netFnSv300g3eOEM3, CMD_GET_SOL_PATTERN,
                           NULL, ipmiGetSolPattern, PRIVILEGE_USER);

    // <Set Leaky Bucket Threshold>
    ipmi_register_callback(netFnSv300g3eOEM3, CMD_SET_LBA_THRESHOLD,
                           NULL, ipmiSetLbaThreshold, PRIVILEGE_USER);

    // <Get Leaky Bucket Threshold>
    ipmi_register_callback(netFnSv300g3eOEM3, CMD_GET_LBA_THRESHOLD,
                           NULL, ipmiGetLbaThreshold, PRIVILEGE_USER);
}
