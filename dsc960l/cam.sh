#!/bin/bash

#
# DAVIDE AIRAGHI - http://www.airaghi.net
#
# DLINK 960L console motion detection manager
# License GPLv3 - see https://www.gnu.org/licenses/gpl-3.0.en.html
#
# This script allows you to manage (query, enable, disable) 
# motion detection on your DCS-960L camera from console
#
# Requirements: 
# - bash shell
# - curl
# - same network used for camera and computer
#

# camera authentication info
USER="CAMERA_USER"
PASS="CAMERA_PASSWORD"

# generic info
TIMEOUT_BEFORE_ENABLE=15
TIMEOUT_BEFORE_DISABLE=0

# camera base url
URL="http://CAMERA_IP:CAMERA_PORT"

# camera video capture params
CAPTURE_SENSITIVITY="80"
CAPTURE_ENABLE_MASK="FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
CAPURE_DISABLE_MASK="000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
CAPTURE_ENABLE_URL="/config/motion.cgi?enable=yes"
CAPTURE_DISABLE_URL="/config/motion.cgi?enable=no"
INFO_URL="/config/motion.cgi"

if [ "$ACTION" = "" ]; then
    ACTION=$1
fi

case $ACTION in
	info) 
		ACTION=""
		IS_ENABLED=$( curl -u ${USER}:${PASS} ${URL}${INFO_URL} 2>/dev/null | grep 'enable=yes' )
		if [ "${IS_ENABLED}" != "" ]; then
			echo enabled
		else
			echo disabled
		fi
		;;
	enable)
		ACTION=""
		sleep $TIMEOUT_BEFORE_ENABLE
		curl -u ${USER}:${PASS} "${URL}${CAPTURE_ENABLE_URL}&sensitivity=${CAPTURE_SENSITIVITY}&mbmask=${CAPTURE_ENABLE_MASK}" 2>/dev/null >/dev/null || exit 1
		;;
	disable)
		ACTION=""
		sleep $TIMEOUT_BEFORE_DISABLE
		curl -u ${USER}:${PASS} "${URL}${CAPTURE_DISABLE_URL}" 2>/dev/null >/dev/null|| exit 1
		;;
	toggle)
		ACTION=""
		IS_ENABLED=$( curl -u ${USER}:${PASS} ${URL}${INFO_URL} 2>/dev/null | grep 'enable=yes' )
		if [ "${IS_ENABLED}" != "" ]; then
			sleep $TIMEOUT_BEFORE_DISABLE
			curl -u ${USER}:${PASS} "${URL}${CAPTURE_DISABLE_URL}" 2>/dev/null >/dev/null || exit 1
		else
			sleep $TIMEOUT_BEFORE_ENABLE
			curl -u ${USER}:${PASS} "${URL}${CAPTURE_ENABLE_URL}&sensitivity=${CAPTURE_SENSITIVITY}&mbmask=${CAPTURE_ENABLE_MASK}" 2>/dev/null >/dev/null || exit 1
		fi
		;;
	*) 
		ACTION=""
		;;
esac

exit 0

