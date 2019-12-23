#!/bin/sh

fanTablePath="/run/initramfs/ro/usr/share/entity-manager/configurations/fan-table.json"

if [ -f "$fanTablePath" ]; then
    /usr/bin/swampd -c $fanTablePath
else
    # If fan table do not exist, write 100% pwm and stop phosphor-pid-control.service
    echo "Fan table does not exist, write 100% pwm and stop fan control service" | systemd-cat -t phosphor-pid-control
    /usr/bin/writePwm.sh 100
    systemctl stop phosphor-pid-control.service
fi
