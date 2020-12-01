#!/bin/bash

# find your device name using the following command
# $ pacmd list | grep "\.monitor"
DEVICE=alsa_output.pci-0000_00_1b.0.analog-stereo.monitor
# DEVICE=alsa_output.usb-Plantronics_Plantronics_GameCom_780-00.analog-stereo.monitor
# DEVICE=alsa_input.pci-0000_00_1b.0.analog-stereo

pacat --record --rate=44100 --channels=1 --format=float32le --latency-msec 25 -d "$DEVICE"
