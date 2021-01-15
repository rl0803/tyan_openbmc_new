
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

    struct jtag_mode jtag_mode;
    jtag_mode.feature = JTAG_XFER_MODE;
    jtag_mode.mode = JTAG_XFER_SW_MODE;

    int retval = ioctl(fd, JTAG_SIOCMODE, &jtag_mode);

    if (retval < 0)
    {
        std::cerr << "Failed in setting jtag mode.\n";
        close_jtag_dev(fd);
        return -1;
    }

    return fd;
}

int jtag_interface_status_get(int fd, uint32_t* state)
{
    if(fd < 0)
    {
        return -1;
    }

    uint32_t current_state;
    int retval = ioctl(fd, JTAG_GIOCSTATUS, &current_state);

    if (retval < 0)
    {
        std::cerr << "Failed in jtag interface end_tap_state.\n";
        return -1;
    }

    *state = current_state;

    return 0;
}

/**
 *  @fd:    jtag driver fd
 *  @reset: 1 = force controller and slave into reset state
 *  @from: from state
 *  @endstate: end state
 *  @tck: tck cycles in idle/run-test state
*/
int jtag_interface_end_tap_state(int fd, uint8_t reset,
                                 uint8_t endstate, uint8_t tck)
{
    if(fd < 0)
    {
        return -1;
    }

    // Get current tap state
    int retval = -1;
    uint32_t current_state;

    retval = jtag_interface_status_get(fd, &current_state);
    if (retval < 0)
    {
        std::cerr << "Failed to get current jtag tap state.\n";
        return -1;
    }

    struct jtag_tap_state state;
    state.reset = reset;
    state.endstate = endstate;
    state.tck = tck;
    if(state.reset == 1)
    {
        state.from = JTAG_STATE_TLRESET;
    }
    else
    {
        state.from = (uint8_t)current_state;
    }

    retval = ioctl(fd, JTAG_SIOCSTATE, &state);

    if (retval < 0)
    {
        std::cerr << "Failed in jtag interface end_tap_state.\n";
        return -1;
    }

    // Stay in idle/run-test state for tck clock
    if (endstate == JTAG_STATE_IDLE)
    {
        uint8_t tdo;
        for (int i=0; i< tck; i++)
        {
            retval = jtag_interface_bitbang(fd, 0, 0, &tdo);
            if (retval < 0)
            {
                std::cerr << "Failed to stay in idle/run-test state.\n";
                return -1;
            }
        }
    }

    return 0;
}

int jtag_interface_set_freq(int fd, unsigned int freq)
{
    if(fd < 0)
    {
        return -1;
    }

    int retval = ioctl(fd, JTAG_SIOCFREQ, &freq);

    if (retval < 0)
    {
        std::cerr << "Failed in jtag interface set freq.\n";
        return -1;
    }

    return 0;
}

int jtag_interface_get_freq(int fd, unsigned int* freq)
{
    if(fd < 0)
    {
        return -1;
    }

    unsigned int value = 0;

    int retval = ioctl(fd, JTAG_GIOCFREQ, &value);

    if (retval < 0)
    {
        std::cerr << "Failed in jtag interface get freq.\n";
        return -1;
    }

    *freq = value;

    return 0;
}

int jtag_interface_xfer(int fd, uint8_t type, uint8_t direction,
                        uint8_t endstate, uint32_t length, uint32_t *buf)
{
    if(fd < 0)
    {
        return -1;
    }

    // Get current tap state
    int retval = -1;
    uint32_t current_state;

    retval = jtag_interface_status_get(fd, &current_state);
    if (retval < 0)
    {
        std::cerr << "Failed to get current jtag tap state.\n";
        return -1;
    }

    struct jtag_xfer xfer;
    uint32_t tdio = (unsigned int)buf;

    xfer.type = type;
    xfer.direction = direction;
    xfer.length = length;
    xfer.from = (uint8_t)current_state;
    xfer.endstate = endstate;
    xfer.padding = 0;
    xfer.tdio = tdio;

    retval = ioctl(fd, JTAG_IOCXFER, &xfer);
    if (retval < 0)
    {
        std::cerr << "Failed in jtag interface xfer.\n";
        return -1;
    }

    return 0;
}

/**
 * @fd:    jtag driver fd
 * @endir: IR end state
 * @len:   data length in bit
 * @tdi:   instruction data
*/

int jtag_interface_sir_xfer(int fd, unsigned char endir,
                            unsigned int len, unsigned int tdi)
{
    if(len > 32 || fd < 0)
    {
        return -1;
    }

    // Get current tap state
    int retval = -1;
    uint32_t current_state;

    retval = jtag_interface_status_get(fd, &current_state);
    if (retval < 0)
    {
        std::cerr << "Failed to get current jtag tap state.\n";
        return -1;
    }

    struct jtag_xfer xfer;
    unsigned int *buf = &tdi;
    unsigned int tdio = (unsigned int)buf;

    xfer.type = JTAG_SIR_XFER;
    xfer.direction = JTAG_WRITE_XFER;
    xfer.from = (uint8_t)current_state;
    xfer.endstate = endir;
    xfer.length = len;
    xfer.padding = 0;
    xfer.tdio = tdio;

    retval = ioctl(fd, JTAG_IOCXFER, &xfer);
    if (retval < 0)
    {
        std::cerr << "Failed in jtag interface sir xfer.\n";
        return -1;
    }

    return 0;
}

/**
 * @fd:    jtag driver fd
 * @enddr: DR endstate
 * @len:   data length in bit
 * @tdio:  data array
*/

int jtag_interface_tdo_xfer(int fd, unsigned char endir,
                            unsigned int len, unsigned int *buf)
{
    if(buf == NULL || fd < 0)
    {
        return -1;
    }

    // Get current tap state
    int retval = -1;
    uint32_t current_state;

    retval = jtag_interface_status_get(fd, &current_state);
    if (retval < 0)
    {
        std::cerr << "Failed to get current jtag tap state.\n";
        return -1;
    }

    struct jtag_xfer xfer;
    unsigned int tdio = (unsigned int)buf;

    xfer.type = JTAG_SDR_XFER;
    xfer.direction = JTAG_READ_XFER;
    xfer.from = (uint8_t)current_state;
    xfer.endstate = endir;
    xfer.length = len;
    xfer.padding = 0;
    xfer.tdio = tdio;

    retval = ioctl(fd, JTAG_IOCXFER, &xfer);
    if (retval < 0)
    {
        std::cerr << "Failed in jtag interface tdo xfer.\n";
        return -1;
    }

    return 0;
}

/**
 * @fd:    jtag driver fd
 * @enddr: DR end state
 *         0 = idle, otherwise = pause
 * @len:   data length in bit
 * @tdio:  data array
*/
int jtag_interface_tdi_xfer(int fd, unsigned char endir,
                            unsigned int len, unsigned int *buf)
{
    if(buf == NULL || fd < 0)
    {
        return -1;
    }

    // Get current tap state
    int retval = -1;
    uint32_t current_state;

    retval = jtag_interface_status_get(fd, &current_state);
    if (retval < 0)
    {
        std::cerr << "Failed to get current jtag tap state.\n";
        return -1;
    }

    struct jtag_xfer xfer;
    unsigned int tdio = (unsigned int)buf;

    xfer.type = JTAG_SDR_XFER;
    xfer.direction = JTAG_WRITE_XFER;
    xfer.from = (uint8_t)current_state;
    xfer.endstate = endir;
    xfer.length = len;
    xfer.padding = 0;
    xfer.tdio = tdio;

    retval = ioctl(fd, JTAG_IOCXFER, &xfer);
    if (retval < 0)
    {
        std::cerr << "Failed in jtag interface tdi xfer.\n";
        return -1;
    }

    return 0;
}

/**
 * @fd:    jtag driver fd
 * @tms:   tmd bit
 * @tdi:   tdi bit
 * @tdi:   tdo bit
*/
int jtag_interface_bitbang(int fd, uint8_t tms,
                           uint8_t tdi, uint8_t *tdo)
{
    if(fd < 0)
    {
        return -1;
    }

    struct tck_bitbang bitbang;
    bitbang.tms = tms & 0x1;
    bitbang.tdi = tdi & 0x1;

    struct bitbang_packet bitbang_pkg;
    bitbang_pkg.data = &bitbang;
    bitbang_pkg.length = 1;

    int retval = ioctl(fd, JTAG_IOCBITBANG, &bitbang_pkg);

    if (retval < 0)
    {
        std::cerr << "Failed in jtag bitbang operation.\n";
        return -1;
    }

    *tdo = bitbang.tdo;

    return 0;
}

void close_jtag_dev(int fd)
{
    close(fd);
}
