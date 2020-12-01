#!/bin/bash
ffmpeg -video_size 1920x1080 -f x11grab -i :0.0 -f pulse -i 0 -f pulse -i default -filter_complex amerge -ac 2 output.mp4
