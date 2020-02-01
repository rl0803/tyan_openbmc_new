#pragma once

#include <string>

struct memoryConf
{
    std::string MemoryChannelIndex = "";
    std::string MemoryDeviceType = "";
    std::string MemoryType = "";
    uint64_t OperatingSpeedMhz = 0;
    // Named MemorySizeInKb on D-bus
    uint64_t MemoryCapacity = 0;
};

struct processorConf
{
    // Named ProcessorMaxSpeed on D-bus
    uint64_t MaxSpeedMHz = 0;
    // Named ProcessorFamily and Model on D-bus
    std::string Model = "";
};

struct pcieConf
{
    std::string DeviceClass = "";
    std::string DeviceName = "";
    std::string VendorId = "";
    std::string DeviceId = "";
    std::string SubsystemVendorId = "";
    // Named SubsystemId on D-bus
    std::string SubsystemDeviceId = "";
    int64_t BusNumber = -1;
    int64_t MuxChannel = -1;
    std::string MuxSlaveAddress = "";
};
