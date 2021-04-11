#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <regex>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sdbusplus/bus.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/log.hpp>

namespace fs = std::filesystem;
using namespace phosphor::logging;
using sdbusplus::exception::SdBusError;

constexpr auto ROOT_PATH = "/";
constexpr auto ROOT_INTERFACE = "org.freedesktop.DBus";

/**
 * @brief Find path accroding to match string.
 *
 * @param[in] dirPath - Directory path.
 * @param[in] matchString - String for match.
 * @param[in] foundPaths - Found match path.
 * @param[in] symlinkDepth - Symlink for directory depth.
 * @return On success returns true.
 */
bool findFiles(const fs::path dirPath, const std::string& matchString,
               std::vector<fs::path>& foundPaths, unsigned int symlinkDepth)
{
    if (!fs::exists(dirPath)){
        return false;
    }

    std::regex search(matchString);
    std::smatch match;
    for (auto& p : fs::recursive_directory_iterator(dirPath))
    {
        std::string path = p.path().string();
        if (!is_directory(p))
        {
            if (std::regex_search(path, match, search))
                foundPaths.emplace_back(p.path());
        }
        else if (is_symlink(p) && symlinkDepth)
        {
            findFiles(p.path(), matchString, foundPaths, symlinkDepth - 1);
        }
    }

    return true;
}

/** @brief Read the value from specific path.
 *
 *  @param[in] path - Specific path for reading.
 */
int readFileValue(std::string path)
{
    std::fstream registerFile(path, std::ios_base::in);
    int value;
    registerFile >> value;
    return value;
}

/** @brief Write the value for specific path.
 *
 *  @param[in] path - Specific path for writing.
 *  @param[in] value - Value for writing.
 */
void writeFileValue(std::string path, int value)
{
    std::fstream registerFile(path, std::ios_base::out);

    registerFile << value;
}
