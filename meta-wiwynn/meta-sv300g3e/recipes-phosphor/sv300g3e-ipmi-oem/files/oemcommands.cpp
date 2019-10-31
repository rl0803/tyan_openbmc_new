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
#include <oemcommands.hpp>

const static constexpr char* solPatternService = "xyz.openbmc_project.SolPatternSensor";
const static constexpr char* solPatternInterface = "xyz.openbmc_project.Sensor.SOLPattern";
const static constexpr char* solPatternObjPrefix = "/xyz/openbmc_project/sensors/pattern/Pattern";

static void register_oem_functions() __attribute__((constructor));

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

static void register_oem_functions(void)
{
    // <Set SOL Pattern>
    ipmi_register_callback(netFnSv300g3eOEM3, CMD_SET_SOL_PATTERN,
                           NULL, ipmiSetSolPattern, PRIVILEGE_USER);

    // <Get SOL Pattern>
    ipmi_register_callback(netFnSv300g3eOEM3, CMD_GET_SOL_PATTERN,
                           NULL, ipmiGetSolPattern, PRIVILEGE_USER);
}
