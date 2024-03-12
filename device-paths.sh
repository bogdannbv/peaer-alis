#!/bin/bash

# For a list of vendor and device IDs, see: https://osmocom.org/projects/rtl-sdr/wiki#Supported-Hardware

vendor_ids=(
  # Realtek Semiconductor Corp.
  "0bda"
)

device_ids=(
  # RTL2838U DVB-T
  "2838"
)

for sysdevpath in $(find /sys/bus/usb/devices/usb*/ -name dev); do
    (
        syspath="${sysdevpath%/dev}"
        devname="$(udevadm info -q name -p $syspath)"

        [[ "$devname" == "bus/"* ]] || [[ "$devname" == "input/"* ]] || [[ "$devname" == "video0"* ]] || exit

        eval "$(udevadm info -q property --export -p $syspath)"
        [[ -z "$ID_SERIAL" ]] && [[ -z "$ID_VENDOR_ID" ]] && [[ -z "$ID_MODEL_ID" ]] && exit

        ! [[ ${vendor_ids[*]} =~ ${ID_VENDOR_ID} ]] && ! [[ ${device_ids[*]} =~ ${ID_MODEL_ID} ]] && exit

        echo "/dev/$devname"
    )
done
