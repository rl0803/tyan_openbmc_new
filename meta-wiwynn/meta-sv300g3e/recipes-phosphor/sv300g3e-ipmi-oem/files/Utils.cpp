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

#include "Utils.hpp"

vector<string> getDirFiles(string dirPath, string regexStr)
{
    vector<string> result;

    for (const auto& entry : filesystem::directory_iterator(dirPath))
    {
        // If filename matched the regular expression put it in result.
        if (regex_match(entry.path().filename().string(), regex(regexStr)))
        {
            result.emplace_back(move(entry.path().string()));
        }
    }

    return result;
}

DbusSubTree getSubTree(sdbusplus::bus::bus& bus, const std::string& pathRoot,
                       int depth, const std::string& intf)
{
    DbusSubTree subTree;
    auto subTreeMsg = bus.new_method_call(
        "xyz.openbmc_project.ObjectMapper", "/xyz/openbmc_project/object_mapper",
        "xyz.openbmc_project.ObjectMapper", "GetSubTree");

    const std::vector<std::string> interfaces = {intf};

    subTreeMsg.append(pathRoot, depth, interfaces);

    sdbusplus::message::message reply;
    try
    {
        reply = bus.call(subTreeMsg);
    }
    catch (sdbusplus::exception::SdBusError& e)
    {
        sd_journal_print(LOG_ERR,
                         "GetSubTree Failed in bus call, %s\n",
                         e.what());
    }

    try
    {
        reply.read(subTree);
    }
    catch (sdbusplus::exception::SdBusError& e)
    {
        sd_journal_print(LOG_ERR,
                         "GetSubTree Failed in read reply, %s\n",
                         e.what());
    }

    return subTree;
}
