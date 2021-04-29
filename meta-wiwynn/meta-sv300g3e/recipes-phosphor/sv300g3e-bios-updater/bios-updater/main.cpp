#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <openbmc/libsysfs.h>
#include "bios-updater.hpp"

void usage () {
    std::cout << "Usage: bios-updater [file name] : update bios firmware\n";
}

int main(int argc, char **argv) {

    if (argc != 2) {
        usage();
        return -1;
    }

    if (access(argv[1], F_OK) < 0)
    {
        std::cerr << "Missing BIOS FW file.\n";
        return -1;
    }

    constexpr uint32_t biosFileSize = (32*1024*1024); // 32M
    struct stat buf;
    BiosUpdateManager bios_updater;
    uint8_t state = std::numeric_limits<uint8_t>::quiet_NaN();

    int ret = 0;
    ret = Sem_Acquired(SEM_BIOSUPDATELOCK);
    if (ret < 0)
    {
        printf("semaphor acquired error : %s, ret : %d \n", __func__, ret);
        return ret;
    }

    if (stat(argv[1], &buf) < 0)
    {
        std::cerr << "Failed to get BIOS FW file info.\n";
        goto error_handle;
    }

    if (!S_ISREG(buf.st_mode))
    {
        std::cerr << "Invalid BIOS FW file type.\n";
        goto error_handle;
    }

    if (buf.st_size != biosFileSize)
    {
        std::cerr << "Invalid BIOS FW file size.\n";
        goto error_handle;
    }

    std::cout << "Checking system power state...\n";

    ret = bios_updater.biosUpdatePwrStateCheck(state);
    if ((ret < 0) || (state != off))
    {
        std::cerr << "The system is NOT in OFF state.\n";
        std::cerr << "Please power off the system!!!\n";
        goto error_handle;
    }
    std::cout << "The system is in OFF state.\n";

    ret = bios_updater.biosUpdatePrepare();
    if (ret < 0)
    {
        std::cerr << "Failed in bios update prepare.\n";
        goto error_handle;
    }

    ret = bios_updater.biosUpdate(argv[1]);
    if (ret < 0)
    {
        std::cerr << "Failed to bios update.\n";
        goto error_handle;
    }

    ret = bios_updater.biosUpdateFinished(argv[1]);
    if (ret < 0)
    {
        std::cerr << "Failed in bios update finished.\n";
        goto error_handle;
    }

    std::cout << "Powering on the host\n";
    ret = bios_updater.biosUpdatePwrCtl(on);
    if (ret < 0)
    {
        std::cerr << "Failed to power on the host.\n";
        goto error_handle;
    }

    std::cout << "BIOS FW update finished\n";
    ret = Sem_Released(SEM_BIOSUPDATELOCK);
    if (ret < 0)
    {
        printf("semaphor release error : %s, ret : %d \n", __func__, ret);
        return ret;
    }

    return 0;

error_handle:
    ret = Sem_Released(SEM_BIOSUPDATELOCK);
    if (ret < 0)
    {
        printf("semaphor release error : %s, ret : %d \n", __func__, ret);
        return ret;
    }

    return -1;
}
