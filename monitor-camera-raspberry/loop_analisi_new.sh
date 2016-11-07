#!/bin/bash

SOGLIA=35

# directory containing "gdrive" executable | directory che contiene l'eseguibile "gdrive"
GDRIVEPATH="/usr/local/bin/"
# directory containing gdrive config files | directory contenente i file di configurazione di gdrive
GDRIVECONF="/etc/gdrive"
# main directory on google drive | directory principale su Google Drive in cui salvare i file
GDRIVECONT="mangiatoia"
# remote sftp listening port | porta su cui ascolta il server ssh remoto
PORTASSH="22"
# ssh username to use | username da usare per connessione ssh
UTENTESSH="YOUR_USERNAME"
# ssh server name/ip address | indirizzo/nome del server ssh da contattare
SERVERSSH="YOUR_SERVER"

PATH="${PATH}:${GDRIVEPATH}"


cd /mnt/tmp

while [ 1 ]
do

    LISTA=$(ls img-*.png 2>/dev/null)
    OLD=""
    FILE=""

    OK=""

	DATEDIR=$(date +%Y-%m-%d)
    GDRIVEDIR=$( gdrive --config ${GDRIVECONF} list --query "mimeType contains 'folder' " 2>/dev/null| grep ${GDRIVECONT} | head -n 1 | awk '{print $1}')
    GDRIVEUPL=""

    if [ "${GDRIVEDIR}" != "" ]; then

        GDRIVEUPL=$(gdrive --config ${GDRIVECONF} list --query "mimeType contains 'folder' and '${GDRIVEDIR}' in parents "  2>/dev/null| grep $DATEDIR | head -n 1 | awk '{print $1}')
    
        if [ "${GDRIVEUPL}" = "" ]; then
			gdrive --config ${GDRIVECONF} mkdir --parent ${GDRIVEDIR} ${DATEDIR} &>/dev/null
			GDRIVEUPL=$(gdrive --config ${GDRIVECONF} list --query "mimeType contains 'folder' and '${GDRIVEDIR}' in parents " 2>/dev/null| grep $DATEDIR | head -n 1 | awk '{print $1}')
        fi

    fi

    GDRIVEDIR=""


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
	
		if [ "$GDRIVEUPL" = "" ]; then

			echo "cd foto/" > upload.cmd
			echo "-mkdir ${DATEDIR}" >> upload.cmd
			echo "cd ${DATEDIR}" >> upload.cmd
			echo "mput foto-*.jpg " >> upload.cmd
			sftp -b upload.cmd -i ssh/chiave_privata -P ${PORTASSH} ${UTENTESSH}@${SERVERSSH} && rm foto-*.jpg
		
		else
		
			for FILEFOTO in foto-*.jpg;
			do
				gdrive  --config ${GDRIVECONF} upload --parent ${GDRIVEUPL} $FILEFOTO &>/dev/null && rm $FILEFOTO
			done
		
		fi
		
	fi

        sleep 60s

    else

	sleep 300s

    fi



done