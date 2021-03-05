#!/bin/sh
# Update prochot and thermtrip status

type=$1

SERVICE="xyz.openbmc_project.Logging.IPMI"
OBJECT="/xyz/openbmc_project/Logging/IPMI"
INTERFACE="xyz.openbmc_project.Logging.IPMI"
METHOD="IpmiSelAdd"

if [ "$type" == "assert" ]; then
   busctl call $SERVICE $OBJECT $INTERFACE $METHOD ssaybq "AMT SEL Entry" "/xyz/openbmc_project/sensors/system_event/AMT" 3 {0x00,0x00,0xff} yes 0x20  
else
   busctl call $SERVICE $OBJECT $INTERFACE $METHOD ssaybq "AMT SEL Entry" "/xyz/openbmc_project/sensors/system_event/AMT" 3 {0x00,0x00,0xff} no 0x20
fi