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
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <random>
#include <cstdio>
#include <openbmc/libmisc.h>
#include <openbmc/libobmci2c.h>
#include "guid.hpp"

int createGUID(uint8_t guid_type)
{
    uint32_t random[4] = {0};
    uint32_t rngReg = 0x1e6e2078;
    IPMI_GUID_T guid;
    uint8_t buff[6] = {0};
    uint8_t writeBuffer[17] = {0};
    uint32_t offset;
    uint8_t invalid_rn = 0;

    if (device_guid == guid_type)
    {
        offset = offset_devguid;
    }
    else if (system_guid == guid_type)
    {
        offset = offset_sysguid;
    }
    else
    {
        std::cerr << "Wrong GUID type entered\n";
        return -1;
    }

    for (int i = 0; i < sizeof(random)/sizeof(random[0]); i++)
    {
        if (read_register(rngReg, &random[i]) < 0)
        {
            std::cerr << "Failed to generate random number\n";
            return -1;
        }

        if (0 == random[i])
        {
            std::cerr << "Random number is equal to 0\n";
            invalid_rn = 1;
            break;
        }
    }

    // Check whether random number data equal to zero
    if (1 == invalid_rn)
    {
        std::random_device rd;

        std::mt19937 generator(rd());

        for (int i = 0; i < sizeof(random)/sizeof(random[0]); i++)
        {
            random[i] = static_cast<uint32_t>(generator());
        }        
    }

    // Copy random number to GUID
    memcpy(&buff[0], &random[0], 4);
    memcpy(&buff[4], &random[0], 2);
    memcpy(&(guid.nodeID), buff, sizeof(Node_T));

    guid.clock_seq_resved = (random[1] & 0x3FFF) | 0x8000;
    guid.time_low = random[2] & 0xFFFFFFFF;
    guid.time_mid = random[3] & 0xFFFF;
    guid.time_high_ver = (random[3] >> 16) & 0x0FFF;
    guid.time_high_ver |= (static_cast<uint16_t>(guid_generation) << 12) &0xF000;

    // Write GUID to EEPROM
    memcpy(writeBuffer, &guid, sizeof(IPMI_GUID_T));
    writeBuffer[sizeof(writeBuffer)-1] = zeroChecksum(writeBuffer, sizeof(writeBuffer)-1);

    if (FAILURE == i2cEEPROMSet(i2cbus_eeprom, i2caddr_eeprom, offset, sizeof(writeBuffer), writeBuffer))
    {
        std::cerr << "Failed to write EEPROM\n";
        return -1;
    }

    return 0;
}

int checkGUID(uint8_t guid_type)
{
    uint8_t buffer[17] = {0};
    uint8_t checksum;
    uint32_t offset;

    if (device_guid == guid_type)
    {
        offset = offset_devguid;
    }
    else if (system_guid == guid_type)
    {
        offset = offset_sysguid;
    }
    else
    {
        std::cerr << "Invalid GUID type entered\n";
        return -1;
    }

    if (FAILURE == i2cEEPROMGet(i2cbus_eeprom, i2caddr_eeprom, offset, sizeof(buffer), buffer))
    {
        std::cerr << "Failed to read EEPROM\n";
        return -1;
    }
    else
    {
        checksum = zeroChecksum(buffer, sizeof(buffer)-1);
        if (checksum == buffer[sizeof(buffer)-1])
        {
            if (device_guid == guid_type)
            {
                std::cerr << "Valid Device GUID existed:";
            }
            else
            {
                std::cerr << "Valid System GUID existed:";
            }

            for (int i = 0; i < sizeof(buffer)-1; i++)
            {
                fprintf(stderr, " 0x%x", buffer[i]);
            }
            std::cerr << "\n";
        }
        else
        {
            if (device_guid == guid_type)
            {
                std::cerr << "Create new Device GUID\n";
            }
            else
            {
                std::cerr << "Create new System GUID\n";
            }

            if (createGUID(guid_type) < 0)
            {
                std::cerr << "Failed to create new Device GUID";
                return -1;
            }
        }
    }

    return 0;
}

int checkAMTStatus()
{
    uint8_t buffer[2] = {0};
    uint8_t AMT_status_value = 0xff;
    uint8_t AMT_status_checksum = 0xff;
    uint8_t writeIgnore[2] = {0};

    if (FAILURE == i2cEEPROMGet(i2cbus_eeprom, i2caddr_eeprom,
                                offset_AMT_status, sizeof(buffer), buffer))
    {
        std::cerr << "Failed to read AMT value from EEPROM\n";
        return -1;
    }

    AMT_status_value = buffer[0];
    AMT_status_checksum = buffer[1];
    writeIgnore[0] = ignoreAMT;
    writeIgnore[1] = zeroChecksum(writeIgnore, sizeof(writeIgnore)-1);
    std::printf("AMT status [%x,%x]\n", AMT_status_value, AMT_status_checksum);

    if ( AMT_status_value != disableAMT &&
         AMT_status_value != enableAMT  &&
         AMT_status_value != ignoreAMT)
    {
        std::printf("AMT Status invalid! Write to [%x,%x]\n", writeIgnore[0], writeIgnore[1]);
        if (FAILURE == i2cEEPROMSet(i2cbus_eeprom, i2caddr_eeprom,
                                    offset_AMT_status, sizeof(writeIgnore), writeIgnore))
        {
            std::cerr << "Failed to write AMT status ignore to EEPROM\n";
            return -1;
        }

        return 0;
    }

    uint8_t checksum = zeroChecksum(buffer, sizeof(buffer)-1);

    if (checksum != AMT_status_checksum)
    {
        std::printf("Checksum should be %x! Write to [%x,%x]\n", checksum, writeIgnore[0], writeIgnore[1]);
        if (FAILURE == i2cEEPROMSet(i2cbus_eeprom, i2caddr_eeprom,
                                    offset_AMT_status, sizeof(writeIgnore), writeIgnore))
        {
            std::cerr << "Failed to write AMT status ignore to EEPROM\n";
            return -1;
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    // Check Device GUID
    if (checkGUID(device_guid) < 0)
    {
        std::cerr << "Failed to Access device GUID\n";
        return -1;
    }

    // Check System GUID
    if (checkGUID(system_guid) < 0)
    {
        std::cerr << "Failed to Access system GUID\n";
        return -1;
    }

    // Check AMT Status
    if (checkAMTStatus() < 0)
    {
        std::cerr << "Failed to check AMT Status\n";
        return -1;
    }

    return 0;
}
