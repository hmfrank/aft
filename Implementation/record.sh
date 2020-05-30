#!/bin/bash

# find your device name using the following command
# $ pacmd list | grep "\.monitor"
DEVICE=alsa_output.pci-0000_00_1b.0.analog-stereo.monitor
#DEVICE=alsa_input.pci-0000_00_1b.0.analog-stereo

pacat --record --rate=1000 --channels=1 --format=float32le --latency-msec 25 -d "$DEVICE"
