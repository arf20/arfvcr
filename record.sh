#!/bin/bash

v4l_dev=/dev/video0
alsa_card=hw:CARD=DVC100,DEV=0

output_dir=$1

fname=$(date +"%Y-%m-%d_%H-%M-%S").mkv



ffmpeg -f alsa -ac 2 -ar 48000 -thread_queue_size 1024 -i $alsa_card -f v4l2 -i $v4l_dev -c:a pcm_s24le -c:v libx264 -b:v 5M -pix_fmt yuv420p -profile:v baseline -preset ultrafast $output_dir/$fname 

