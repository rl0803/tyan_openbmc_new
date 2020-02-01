#!/bin/sh
# echo "End of Post SEL Add"

SERVICE="xyz.openbmc_project.Logging.IPMI"
OBJECT="/xyz/openbmc_project/Logging/IPMI"
INTERFACE="xyz.openbmc_project.Logging.IPMI"
METHOD="IpmiSelAdd"

busctl call $SERVICE $OBJECT $INTERFACE $METHOD ssaybq "SEL Entry" "/xyz/openbmc_project/sensors/systemevent/End_of_Post" 3 {0x1,0xff,0xff} yes 0x20

exit 0
