#!/bin/bash

IMG_W=2592
IMG_H=1944

OFF_START_X=676
OFF_START_Y=464

OFF_SIZE_X=400
OFF_SIZE_Y=470

TIME_START=0800
TIME_END=1600

cd /mnt/tmp

[ -e img.png ] && rm img.png

while [ 1 ]
do

    ORA=$(date +%H%M)

    if [ "$ORA" -ge "${TIME_START}" ]; then
	if [ "$ORA" -lt "${TIME_END}" ]; then
    
            PREF=img-$(date +%s)
	    IMG=${PREF}.png
            raspistill -w $IMG_W -h $IMG_H --output img.png
	    convert img.png[${OFF_SIZE_X}x${OFF_SIZE_Y}+${OFF_START_X}+${OFF_START_Y}] $IMG 2>/dev/null
    	    rm img.png

	else
	    sleep 300s
	fi
    else
	sleep 300s
    fi

    sleep 2s

done

