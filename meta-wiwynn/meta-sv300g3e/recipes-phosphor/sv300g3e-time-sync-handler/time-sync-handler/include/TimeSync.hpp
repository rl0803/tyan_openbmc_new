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

#include <unistd.h>
#include <variant>
#include <iostream>
#include <systemd/sd-journal.h>
#include <sdbusplus/asio/object_server.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/chrono/chrono_io.hpp>
#include <boost/process.hpp>
#include <nlohmann/json.hpp>

const static std::string configPath = "/etc/time-sync.json";

constexpr char const *ipmbChannelService = "xyz.openbmc_project.Ipmi.Channel.Ipmb";
constexpr char const *ipmbChannelPath = "/xyz/openbmc_project/Ipmi/Channel/Ipmb";
constexpr char const *ipmbChannelInterface = "org.openbmc.Ipmb";
constexpr char const *ipmbChannelMethod = "sendRequest";

const static uint8_t commandAddress = 1;
const static uint8_t netfn = 0xa;
const static uint8_t lun = 0;
const static uint8_t command = 0x48;
const static std::vector<uint8_t> commandData = {};
const static uint8_t ipmiCmdCcOk = 0x0;
