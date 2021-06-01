#!/bin/bash

# Wait host power on
sleep 10

checkI2cDeviceExist=$(i2cdetect -y 17 |grep -i 51)
checkFileExist=$(ls /sys/bus/i2c/drivers/at24/17-0051)

if [[ ! -n $checkFileExist ]] && [[ -n $checkI2cDeviceExist ]]; then
    echo "Starting to rescan NIC FRU for intel NIC card"
    echo 17-0051 > /sys/bus/i2c/drivers/at24/bind
    sleep 1
    systemctl restart  xyz.openbmc_project.FruDevice.service
    sleep 5
    dbus-send --system --type=signal / org.freedesktop.DBus.Properties.SensorReplaceCache byte:1
fi

exit 0