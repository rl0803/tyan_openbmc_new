#!/bin/bash

echo "[SV300G3-E][S] System Power-on delay"
sleep 1
RETRY=10

# Check the /run/openbmc/host@0-request existence
CHECK=0
while [ ${CHECK} -lt ${RETRY} ]
do
    if [ ! -f "/run/openbmc/host@0-request" ]; then
        break;
    fi
    sleep 1
    (( CHECK=CHECK+1 ))
done
echo "host@0-request: ${CHECK} seconds"


# Check the host-poweroff.service status
CHECK=0
while [ ${CHECK} -lt ${RETRY} ]
do
    status=$(systemctl is-active host-poweroff.service)
    if [ ${status} == "inactive" ]; then
        break;
    fi
    sleep 1
    (( CHECK=CHECK+1 ))
done
echo "host-poweroff.service: ${CHECK} seconds"

/usr/bin/busctl set-property "xyz.openbmc_project.State.Host" "/xyz/openbmc_project/state/host0" "xyz.openbmc_project.State.Host" "RequestedHostTransition" s "xyz.openbmc_project.State.Host.Transition.On"

echo "[SV300G3-E][P] System Power-on delay"
exit 0;
