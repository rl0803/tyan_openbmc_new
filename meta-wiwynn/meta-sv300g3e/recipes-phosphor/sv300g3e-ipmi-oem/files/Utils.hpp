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

#pragma once

#include <systemd/sd-journal.h>
#include <filesystem>
#include <regex>
#include <string>
#include <vector>
#include <fstream>
#include <sdbusplus/bus.hpp>

using namespace std;

/**
 *  @brief Function of getting files in specified directory.
 *
 *  @param[in] dirPath - Specified absolute directory path.
 *  @param[in] regexStr - Regular expression for filename filter.
 *                        If this parameter is not given,
 *                        Return all filenames in the directory.
 *
 *  @return All filenames in the directory that match the refular expression.
 **/
vector<string> getDirFiles(string dirPath, string regexStr = ".*");

using DbusIntf = std::string;
using DbusService = std::string;
using DbusPath = std::string;
using DbusIntfList = std::vector<DbusIntf>;
using DbusSubTree = std::map<DbusPath, std::map<DbusService, DbusIntfList>>;

/**
*   @brief Get dbus sub tree function.
*
*   @param[in] bus - The bus to register on.
*   @param[in] pathRoot - The root of the tree.
*                         Using "/" will search the whole tree
*   @param[in] depth - The maximum depth of the tree past the root to search.
*                      Use 0 to search all.
*   @param[in] intf - An optional list of interfaces to constrain the search to.
*
*   @return - DbusSubTree, including object path, service, interfaces.
*             map<DbusPath, map<DbusService, DbusIntfList>>
**/
DbusSubTree getSubTree(sdbusplus::bus::bus& bus, const std::string& pathRoot,
                       int depth, const std::string& intf);


/**
*   @brief DIMM configuration function.
*   @param[in] dimmConfig - The structure to store DIMM configuration.
*   @param[in] path - The DIMM configuration path.
*   @return - Successful or Not
**/
bool getDimmConfig(std::vector<uint8_t>& dimmConfig, const std::string& path);
