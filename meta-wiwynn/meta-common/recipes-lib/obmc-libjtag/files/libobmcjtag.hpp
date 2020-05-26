#pragma once

#include <stdint.h>

int open_jtag_dev();
int jtag_interface_xfer(int fd, uint8_t type, uint8_t direction,
                        uint8_t endstate, uint32_t length, uint32_t *buf);
void close_jtag_dev(int fd);
