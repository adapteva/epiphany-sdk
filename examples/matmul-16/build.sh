#!/bin/bash

set -e

if [[ "`arch`" == "x86_64" ]]; then
	echo "Setting path to ARM tools."
	GNU_PATH='a-'
else
	GNU_PATH=''
fi

Proj='matmul'
if [[ "$1" == "-d" ]]; then
	Config='Debug'
else
	Config='Release'
fi
BUILD_DEVICE='yes'
BUILD_HOST='yes'

MK_CLEAN='yesXX'
MK_ALL='yes'


if [[ "${BUILD_DEVICE}" == "yes" ]]; then
	echo "=== Building device programs ==="

	pushd device/${Config} >& /dev/null
	if [[ "${MK_CLEAN}" == "yes" ]]; then
		echo "*** Cleaning device/${Config}"
		make clean
	fi
	if [[ "${MK_ALL}" == "yes" ]]; then
		echo "*** Building device/${Config}"
		make --warn-undefined-variables BuildConfig=${Config} all
	fi

	popd >& /dev/null

fi


EINCS="${EPIPHANY_HOME}/tools/host/include"
ELIBS="${EPIPHANY_HOME}/tools/host/lib"

if [[ "${BUILD_HOST}" == "yes" ]]; then
	echo "=== Building host program ==="

	pushd ./host/${Config} >& /dev/null
	if [[ "${MK_CLEAN}" == "yes" ]]; then
		echo "*** Cleaning host program"
		make clean
	fi
	if [[ "${MK_ALL}" == "yes" ]]; then
		echo "*** Building host program"
		make --warn-undefined-variables BuildConfig=${Config} all
	fi
	popd >& /dev/null
fi

exit
