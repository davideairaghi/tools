#!/bin/sh

# path where cam.sh is located
CAM_PATH="/YOUR/PATH/"

# list of valid tokens
TOKENS="VALID_TOKEN_1 VALID_TOKEN_2 VALID_TOKEN_N"

# token read by RFIDer
TOKEN=""

# read token value sent by RFIDer
read TOKEN < /dev/stdin

# check if the token read is in the "valid list"
OK=
for CHKTOKEN in $TOKENS ;
do
    if [ "$CHKTOKEN" = "$TOKEN" ]; then
	OK=1
    fi
done

# if the token is valid call the camera ...
if [ "$OK" = "1" ]; then
    ACTION="toggle"
    if [ -e ${CAM_PATH}cam.sh ]; then
	. ${CAM_PATH}/cam.sh
    fi
fi

exit 0

