
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/jtag.h>

#include "libobmcjtag.hpp"

const static constexpr char* jtagDevicePath = "/dev/jtag0";

int open_jtag_dev()
{
    int fd = open(jtagDevicePath, O_RDWR);

    if (fd <= 0)
    {
        std::cerr << "Failed to open JTAG device.\n";
        return -1;
    }

    return fd;
}

int jtag_interface_xfer(int fd, uint8_t type, uint8_t direction,
                        uint8_t endstate, uint32_t length, uint32_t *buf)
{
    struct jtag_xfer xfer;
    uint32_t tdio = (unsigned int)buf;

    xfer.type = type;
    xfer.direction = direction;
    xfer.length = length;
    xfer.endstate = endstate;
    xfer.tdio = tdio;

    int retval = ioctl(fd, JTAG_IOCXFER, &xfer);

    if (retval < 0)
    {
        std::cerr << "Failed in jtag interface xfer.\n";
        return -1;
    }

    return 0;
}

void close_jtag_dev(int fd)
{
    close(fd);
}
