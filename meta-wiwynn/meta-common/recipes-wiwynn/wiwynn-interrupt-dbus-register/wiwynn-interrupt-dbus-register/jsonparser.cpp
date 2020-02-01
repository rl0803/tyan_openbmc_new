#include "jsonparser.hpp"

#include "conf.hpp"
#include "exception.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <sstream>

using json = nlohmann::json;

json loadJson(const std::string& path)
{
    std::ifstream jsonFile(path);
    if (jsonFile.is_open() == false)
    {
        throw LoadJsonException("Failed to open json file");
    }

    auto data = json::parse(jsonFile, nullptr, false);
    if (data.is_discarded())
    {
        throw LoadJsonException("Failed to parse json - invalid format");
    }

    return data;
}

std::string findEEPROMPath(const json& data)
{
    std::string bus;
    std::string address;
    auto fruIt = data.find("Fru");
    if (fruIt != data.end())
    {
        auto frus = data["Fru"];
        for (auto& fru : frus)
        {
            auto busIt = fru.find("Bus");
            if (busIt != fru.end())
            {
                bus = std::to_string(int64_t(fru["Bus"]));
            }

            auto addressIt = fru.find("Address");
            if (addressIt != fru.end())
            {
                int32_t addrNum;
                std::stringstream ss;
                ss << std::hex << std::string(fru["Address"]);
                ss >> addrNum;
                ss.clear();
                ss << std::setw(4) << std::setfill('0') << std::hex << addrNum;
                ss >> address;
            }
        }
    }
    else
    {
        throw JsonConfigurationException("Fru is not found.");
    }

    if (bus.empty() || address.empty())
    {
        throw JsonConfigurationException("bus or address not found.");
    }

    return "/sys/bus/i2c/devices/" + bus + "-" + address + "/eeprom";
}

std::map<int64_t, memoryConf> parseMemoryFromJson(const json& data)
{
    std::map<int64_t, memoryConf> memoryInfos;

    uint64_t index = 1;
    auto indexIt = data.find("MemoryStartIndex");
    if (indexIt != data.end())
    {
        index = *indexIt;
    }
    else
    {
        std::cerr << "MemoryStartIndex is not found\n";
    }

    uint64_t count = 0;
    auto countIt = data.find("MemoryCount");
    if (countIt != data.end())
    {
        count = *countIt;
    }
    else
    {
        std::cerr << "MemoryCount is not found\n";
    }

    auto infoIt = data.find("MemoryInfo");
    if (infoIt != data.end())
    {
        auto info = data["MemoryInfo"];
        for (const auto& detail : info)
        {
            int64_t id;
            memoryConf tmp;

            auto idIt = detail.find("Id");
            if (idIt != detail.end())
            {
                detail.at("Id").get_to(id);
            }
            else
            {
                // If Id is missing, skip it.
                continue;
            }

            auto memoryChannelIndex = detail.find("MemoryChannelIndex");
            if (memoryChannelIndex != detail.end())
            {
                detail.at("MemoryChannelIndex").get_to(tmp.MemoryChannelIndex);
            }

            auto memoryDeviceType = detail.find("MemoryDeviceType");
            if (memoryDeviceType != detail.end())
            {
                detail.at("MemoryDeviceType").get_to(tmp.MemoryDeviceType);
            }

            auto memoryType = detail.find("MemoryType");
            if (memoryType != detail.end())
            {
                detail.at("MemoryType").get_to(tmp.MemoryType);
            }

            auto operatingSpeedMhz = detail.find("OperatingSpeedMhz");
            if (operatingSpeedMhz != detail.end())
            {
                detail.at("OperatingSpeedMhz").get_to(tmp.OperatingSpeedMhz);
            }

            auto memoryCapacity = detail.find("MemoryCapacity");
            if (memoryCapacity != detail.end())
            {
                detail.at("MemoryCapacity").get_to(tmp.MemoryCapacity);
            }

            memoryInfos.insert(std::pair<int64_t, memoryConf>(id, tmp));

            ++index;
        }
    }
    else
    {
        throw JsonConfigurationException("MemoryInfo is not found");
    }

    if (index != count + 1)
    {
        std::cerr << "MemoryInfo size and MemoryCount is not match.";
    }

    return memoryInfos;
}

std::map<int64_t, processorConf> parseProcessorFromJson(const json& data)
{
    std::map<int64_t, processorConf> processorInfos;

    uint64_t index = 1;
    auto indexIt = data.find("ProcessorStartIndex");
    if (indexIt != data.end())
    {
        index = *indexIt;
    }
    else
    {
        std::cerr << "ProcessorStartIndex is not found\n";
    }

    uint64_t count = 0;
    auto countIt = data.find("ProcessorCount");
    if (countIt != data.end())
    {
        count = *countIt;
    }
    else
    {
        std::cerr << "ProcessorCount is not found\n";
    }

    auto infoIt = data.find("ProcessorInfo");
    if (infoIt != data.end())
    {
        auto info = data["ProcessorInfo"];
        for (const auto& detail : info)
        {
            int64_t id;
            processorConf tmp;

            auto idIt = detail.find("Id");
            if (idIt != detail.end())
            {
                detail.at("Id").get_to(id);
            }
            else
            {
                // If Id is missing, skip it.
                continue;
            }

            auto maxSpeedMHz = detail.find("MaxSpeedMHz");
            if (maxSpeedMHz != detail.end())
            {
                detail.at("MaxSpeedMHz").get_to(tmp.MaxSpeedMHz);
            }

            auto model = detail.find("Model");
            if (model != detail.end())
            {
                detail.at("Model").get_to(tmp.Model);
            }

            processorInfos.insert(std::pair<int64_t, processorConf>(id, tmp));

            ++index;
        }
    }
    else
    {
        throw JsonConfigurationException("ProcessorInfo is not found");
    }

    if (index != count + 1)
    {
        std::cerr << "ProcessorInfo size and ProcessorCount is not match. ";
    }

    return processorInfos;
}

std::map<int64_t, pcieConf> parsePCIeFromJson(const json& data)
{
    std::map<int64_t, pcieConf> pcieInfos;

    uint64_t index = 1;
    auto indexIt = data.find("PCIeStartIndex");
    if (indexIt != data.end())
    {
        index = *indexIt;
    }
    else
    {
        std::cerr << "PCIeStartIndex is not found\n";
    }

    uint64_t count = 0;
    auto countIt = data.find("PCIeCount");
    if (countIt != data.end())
    {
        count = *countIt;
    }
    else
    {
        std::cerr << "PCIeCount is not found\n";
    }

    auto infoIt = data.find("PCIeInfo");
    if (infoIt != data.end())
    {
        auto info = data["PCIeInfo"];
        for (const auto& detail : info)
        {
            int64_t id;
            pcieConf tmp;

            auto idIt = detail.find("Id");
            if (idIt != detail.end())
            {
                detail.at("Id").get_to(id);
            }
            else
            {
                // If Id is missing, skip it.
                continue;
            }

            auto deviceClass = detail.find("DeviceClass");
            if (deviceClass != detail.end())
            {
                detail.at("DeviceClass").get_to(tmp.DeviceClass);
            }
            else
            {
                // If DeviceClass is missing, treat this device as not exist.
                continue;
            }

            auto deviceName = detail.find("DeviceName");
            if (deviceName != detail.end())
            {
                detail.at("DeviceName").get_to(tmp.DeviceName);
            }

            auto vendorId = detail.find("VendorId");
            if (vendorId != detail.end())
            {
                detail.at("VendorId").get_to(tmp.VendorId);
            }

            auto deviceId = detail.find("DeviceId");
            if (deviceId != detail.end())
            {
                detail.at("DeviceId").get_to(tmp.DeviceId);
            }

            auto subsystemVendorId = detail.find("SubsystemVendorId");
            if (subsystemVendorId != detail.end())
            {
                detail.at("SubsystemVendorId").get_to(tmp.SubsystemVendorId);
            }

            auto subsystemDeviceId = detail.find("SubsystemDeviceId");
            if (subsystemDeviceId != detail.end())
            {
                detail.at("SubsystemDeviceId").get_to(tmp.SubsystemDeviceId);
            }

            auto smBusInfoIt = detail.find("SMBusInfo");
            if (smBusInfoIt != detail.end())
            {
                auto smBusInfo = detail["SMBusInfo"];
                for (const auto& smBusDetail : smBusInfo)
                {
                    auto busNumberIt = smBusDetail.find("BusNumber");
                    if (busNumberIt != smBusDetail.end())
                    {
                        smBusDetail.at("BusNumber").get_to(tmp.BusNumber);
                    }

                    auto muxInfoIt = smBusDetail.find("MuxInfo");
                    if (muxInfoIt != smBusDetail.end())
                    {
                        auto muxInfo = smBusDetail["MuxInfo"];
                        for (const auto& muxDetail : muxInfo)
                        {
                            auto channelIt = muxDetail.find("Channel");
                            if (channelIt != muxDetail.end())
                            {
                                muxDetail.at("Channel").get_to(tmp.MuxChannel);
                            }

                            auto slaveAddressIt =
                                muxDetail.find("SlaveAddress");
                            if (slaveAddressIt != muxDetail.end())
                            {
                                muxDetail.at("SlaveAddress")
                                    .get_to(tmp.MuxSlaveAddress);
                            }
                        }
                    }

                    if ((tmp.BusNumber != -1) && (tmp.MuxChannel != -1) &&
                        (tmp.MuxSlaveAddress.empty() == false))
                    {
                        break;
                    }
                }
            }

            pcieInfos.insert(std::pair<int64_t, pcieConf>(id, tmp));

            ++index;
        }
    }
    else
    {
        throw JsonConfigurationException("PCIeInfo is not found");
    }

    if (index != count + 1)
    {
        std::cerr << "PCIeInfo size and PCIeCount is not match. ";
    }

    return pcieInfos;
}
