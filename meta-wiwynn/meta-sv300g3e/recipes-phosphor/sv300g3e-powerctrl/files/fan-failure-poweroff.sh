#!/bin/bash

echo "[SV300G3-E] 2-fan-failure system power off"

CHECK=0
WAIT_TIME=12

pwrstatus=$(busctl get-property org.openbmc.control.Power /org/openbmc/control/power0 org.openbmc.control.Power pgood | cut -d' ' -f2)
if [ ${pwrstatus} -eq 1 ]; then
    # soft-off
    busctl call "org.freedesktop.systemd1" "/org/freedesktop/systemd1" "org.freedesktop.systemd1.Manager" "StartUnit" ss "host-powersoft.service" "replace"

    # Monitor the PGood Status
    while [ ${CHECK} -lt ${WAIT_TIME} ]
    do
        sleep 1
        (( CHECK=CHECK+1 ))
        pwrstatus=$(busctl get-property org.openbmc.control.Power /org/openbmc/control/power0 org.openbmc.control.Power pgood | cut -d' ' -f2)
        if [ ${pwrstatus} -eq 0 ]; then
            echo "It took ${CHECK} seconds to soft power-off the system!"
            break;
        fi
    done

    if [ ${CHECK} -eq ${WAIT_TIME} ]; then
        echo "Failed to soft-off the system."
        echo "Try to force shut down!"

        # force shut-down
        busctl call "org.freedesktop.systemd1" "/org/freedesktop/systemd1" "org.freedesktop.systemd1.Manager" "StartUnit" ss "host-poweroff.service" "replace"
    fi
else
    echo "[SV300G3-E] System power already off"
fi

exit 0;
