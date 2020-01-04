#!/bin/sh
# Update prochot status

type=$1

SERVICE="xyz.openbmc_project.Logging.IPMI"
OBJECT="/xyz/openbmc_project/Logging/IPMI"
INTERFACE="xyz.openbmc_project.Logging.IPMI"
METHOD="IpmiSelAdd"


if [ $2 == "CPU1_State" ]; then
    present=$(busctl get-property xyz.openbmc_project.CPUSensor /xyz/openbmc_project/inventory/system/chassis/motherboard/CPU0 \
        xyz.openbmc_project.Inventory.Item Present |awk '{print $2}')
    if [ $present == "false" ]; then
        exit 0;
    fi
fi 

if [ "$type" == "assert" ]; then
   busctl call $SERVICE $OBJECT $INTERFACE $METHOD ssaybq "SEL Entry" "/xyz/openbmc_project/sensors/processor/$2" 3 {0x0A,0x00,0xff} yes 0x20  
else
   busctl call $SERVICE $OBJECT $INTERFACE $METHOD ssaybq "SEL Entry" "/xyz/openbmc_project/sensors/processor/$2" 3 {0x0A,0x00,0xff} no 0x20
fi

