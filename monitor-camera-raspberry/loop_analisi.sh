#!/bin/bash

SOGLIA=35

cd /mnt/tmp

while [ 1 ]
do

    LISTA=$(ls img-*.png 2>/dev/null)
    OLD=""
    FILE=""

    OK=""

    for FILE in $LISTA;
    do
	if [ "$OLD" != "" ]; then
	    if [ -e $OLD ]; then
	    if [ -e $FILE ]; then
		DIFF=$(compare -metric PSNR $OLD $FILE null: 2>&1 | sed -es/\\./\ / | awk '{print $1}')
		if [ $SOGLIA -gt $DIFF ]; then
		    # img da tenere !
	    	    convert $FILE foto-$(date +%s)-${DIFF}.jpg
		    OK=1
		fi
	    fi
	    fi
	    rm $OLD
	fi
	sleep 1s
        OLD=$FILE
    done

    if [ "$LISTA" != "" ]; then

	if [ "$OK" = "1" ]; then
	
	    DATEDIR=$(date +%Y-%m-%d)
	    echo "cd foto/" > upload.cmd
	    echo "-mkdir ${DATEDIR}" >> upload.cmd
	    echo "cd ${DATEDIR}" >> upload.cmd
	    echo "mput foto-*.jpg " >> upload.cmd
	    sftp -b upload.cmd -i /home/pi/ssh/chiave_privata -P PORTASSH UTENTESSH@SERVERSSH && rm foto-*.jpg
	fi

        sleep 60s

    else

	sleep 300s

    fi



done

