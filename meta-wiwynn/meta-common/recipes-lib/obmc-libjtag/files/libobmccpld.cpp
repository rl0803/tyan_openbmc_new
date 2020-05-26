
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/jtag.h>
#include <cstring>

#include "libobmcjtag.hpp"
#include "libobmccpld.hpp"

constexpr const uint32_t latticeIrWtLen = 8;
constexpr const uint32_t latticeDrRdLen = 32;

int getCpldUserCode(uint8_t part, uint32_t* buffer)
{
    uint32_t jtagIrWtLen = 0;
    uint32_t jtagDrRdLen = 0;
    uint32_t jtagCmd = 0x0;

    switch (part)
    {
        case LATTICE:
        {
            jtagIrWtLen = latticeIrWtLen;
            jtagDrRdLen = latticeDrRdLen;
            jtagCmd = lattice_read_usercode;
            break;
        }

        default:
            std::cerr << "Invalid CPLD part providor. " << __FUNCTION__ << "\n";
            return -1;
    }

    int jtagFd = open_jtag_dev();
    if (jtagFd < 0)
    {
        std::cerr << "Error in " << __FUNCTION__ << " in " << __LINE__ << "\n";
        return -1;
    }

    int ret = -1;
    ret = jtag_interface_xfer(jtagFd, JTAG_SIR_XFER, JTAG_WRITE_XFER,
                              JTAG_STATE_IDLE, jtagIrWtLen, &jtagCmd);
    if (ret < 0)
    {
         std::cerr << "Error in " << __FUNCTION__ << " in " << __LINE__ << "\n";
         close_jtag_dev(jtagFd);
         return -1;
    }

    sleep(1);

    uint32_t result = 0;
    ret = jtag_interface_xfer(jtagFd, JTAG_SDR_XFER, JTAG_WRITE_XFER,
                              JTAG_STATE_IDLE, jtagDrRdLen, &result);
    if (ret < 0)
    {
         std::cerr << "Error in " << __FUNCTION__ << " in " << __LINE__ << "\n";
         close_jtag_dev(jtagFd);
         return -1;
    }

    close_jtag_dev(jtagFd);

    std::memcpy(buffer, &result, sizeof(uint32_t));

    return 0;
}
