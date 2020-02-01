#pragma once

#include "conf.hpp"

#include <map>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

/**
 * Given a json configuration file, load it.
 *
 * @param[in] path - path to the configuration
 * @return the json data.
 **/
json loadJson(const std::string& path);

/**
 * Given json data file, find Fru bus and address.
 *
 * @param[in] data - json data
 * @return EEPROM file path.
 **/
std::string findEEPROMPath(const json& data);

/**
 * Given json data and parse the data.
 *
 * @param[in] data - json data
 * @return a map that key is ID and value is correspond config data.
 **/
std::map<int64_t, memoryConf> parseMemoryFromJson(const json& data);
std::map<int64_t, processorConf> parseProcessorFromJson(const json& data);
std::map<int64_t, pcieConf> parsePCIeFromJson(const json& data);
