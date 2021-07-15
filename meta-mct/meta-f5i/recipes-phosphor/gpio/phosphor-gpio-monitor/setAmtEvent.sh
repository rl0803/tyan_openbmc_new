#!/bin/sh
# Update prochot and thermtrip status

type=$1

SERVICE="xyz.openbmc_project.Logging.IPMI"
OBJECT="/xyz/openbmc_project/Logging/IPMI"
INTERFACE="xyz.openbmc_project.Logging.IPMI"
METHOD="IpmiSelAdd"

AMT_SERVICE="xyz.openbmc_project.Settings"
AMT_OBJECT="/xyz/openbmc_project/oem/HostStatus"
AMT_INTERFACE="xyz.openbmc_project.OEM.HostStatus"
AMT_PROPERTY="AmtMonitorStatus"

AMT_FILE="/run/AMT_START"

AmtMonitorStatus=$(busctl get-property $AMT_SERVICE $AMT_OBJECT $AMT_INTERFACE $AMT_PROPERTY |awk 'NR==1 {print $2}')

if [ "$AmtMonitorStatus" == "false" ]; then
   exit 0
fi

if [ "$type" == "assert" ]; then
   busctl call $SERVICE $OBJECT $INTERFACE $METHOD ssaybq "AMT SEL Entry" "/xyz/openbmc_project/sensors/system_event/AMT_START" 3 {0x00,0x00,0xff} yes 0x20
   touch $AMT_FILE
else
   if [ -f $AMT_FILE ]; then
      busctl call $SERVICE $OBJECT $INTERFACE $METHOD ssaybq "AMT SEL Entry" "/xyz/openbmc_project/sensors/system_event/AMT_COMPLETE" 3 {0x00,0x00,0xff} yes 0x20
      rm -f $AMT_FILE
   fi
fi