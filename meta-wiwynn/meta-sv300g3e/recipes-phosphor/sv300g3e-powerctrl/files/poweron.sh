#!/bin/bash

echo "[SV300G3-E][S] System Power on"

pwrstatus=$(/usr/bin/gpioget gpiochip0 101)
if [ ${pwrstatus} -eq 0 ]; then
    /usr/bin/gpioset gpiochip0 27=0
    sleep 2
    /usr/bin/gpioset gpiochip0 27=1 
fi

echo "[SV300G3-E][P] System Power on"
exit 0;
