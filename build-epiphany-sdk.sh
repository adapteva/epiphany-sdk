#!/usr/bin/env bash

# Copyright (C) 2009-2014 Adapteva Inc

# Contributor Yaniv Sapir <ysapir@adapteva.com>
# Contributor Ben Chaco <bchaco@x3-c.com>
# Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

# This file is a script to download toolchain source.

# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 3 of the License, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.

# You should have received a copy of the GNU General Public License along
# with this program.  If not, see <http://www.gnu.org/licenses/>.

# Host machine architecture default
ARCH="armv7l"

# Revision number of Epiphany SDK default
REV="RevUndefined"

# Git branch name for build default
BRANCH="master"

# Host architecture triplet for compiler tools
# Likel value (with -c) if cross-building on Ubuntu is "arm-linux-gnueabihf"
ARCH_TRIPLET=""

while getopts :a:c:r:t:h arg; do
	case $arg in

	a)
		ARCH=$OPTARG
		;;

	c)
		ARCH_TRIPLET=$OPTARG
		;;

	r)
		REV=$OPTARG
		;;

	l)
		ESDK_LIBS=$OPTARG
		;;
		
	t)
		BRANCH=$OPTARG
		;;

	h)
		echo "Usage: ./build-epiphany-sdk.sh "
		echo "    [-a <host arch>]: The host architecture. Default $ARCH"
		echo "    [-c <host triplet>]: The architecture triplet. Only needed for Canadian cross builds."
		echo "    [-r <revision>]: The revision string for the SDK. Default $REV"
		echo "    [-t <tag_name>]: The tag name (or branch name) for the SDK sources. Default $BRANCH"
		echo "    [-h]: Show usage"
		echo ""
		exit 0
		;;

	\?)
		echo "Invalid Argument; $1"
		exit 1
		;;
	esac
done

shift $((OPTIND-1))

if [ -z $EPIPHANY_BUILD_HOME ]; then

	d=$(dirname "$0")
	export EPIPHANY_BUILD_HOME=$(cd "$d/.." && pwd)

fi

ESDK="${EPIPHANY_BUILD_HOME}/esdk.${REV}"
HOSTNAME="host.${ARCH}"
HOST="${ESDK}/tools/${HOSTNAME}"
GNUNAME="e-gnu.${ARCH}"
GNU="${ESDK}/tools/${GNUNAME}"
EPIPHANY_HOME=$ESDK

echo '********************************************'
echo '************ Epiphany SDK Build ************'
echo '********************************************'

echo ""
echo "Environment settings:"
echo ""
echo "    EPIPHANY_BUILD_HOME=$EPIPHANY_BUILD_HOME"
echo "    EPIPHANY_HOME=$EPIPHANY_HOME"
echo "    ESDK=$ESDK"
echo ""
echo "Build settings:"
echo ""
echo "    Target architecture:          ${ARCH}"
echo "    Build Revision:               ${REV}"
echo "    Build from branch or tag:     ${BRANCH}"
echo ""

cd $EPIPHANY_BUILD_HOME

pushd sdk >& /dev/null

if [ ! -d ../esdk/tools/${GNUNAME}/ ]; then
	# Create the SDK tree
	echo "Creating the eSDK directory tree..."

	mkdir -p ${ESDK} ${ESDK}/bsps ${ESDK}/tools
	mkdir -p ${HOST}/lib ${HOST}/include ${HOST}/bin
	mkdir -p ${GNU}
fi

# Create toolchain symbolic links (force overwrite if exists)
ln -sTf "esdk.${REV}" ${EPIPHANY_BUILD_HOME}/esdk
ln -sTf ${HOSTNAME} ${ESDK}/tools/host
ln -sTf ${GNUNAME}  ${ESDK}/tools/e-gnu


# Sort out arg for Canadian cross
if [ "x${ARCH_TRIPLET}" = "x" ]; then
	host_str=""
else
	host_str="--host ${ARCH_TRIPLET}"
fi


if [ "$EPIPHANY_BUILD_TOOLCHAIN" != "no" ]; then
	if ! ./download-toolchain.sh --clone; then

		printf "\nAborting...\n"
		exit 1
	fi

	#Build the toolchain (this will take a while)
	if ! ./build-toolchain.sh --install-dir-host ${EPIPHANY_HOME}/tools/${GNUNAME} ${host_str}; then
		printf "The toolchain build failed!\n"
		printf "\nAborting...\n"
		exit 1
	fi
fi

if [ ! -d "$PARALLELLA_LINUX_HOME" ]; then
	# Clone the parallella Linux source tree
	git clone https://github.com/parallella/parallella-linux.git -b main

	export PARALLELLA_LINUX_HOME=$PWD/parallella-linux
fi

# build the epiphany-libs and install the SDK
if ! ./install-sdk.sh -n $REV -x $BRANCH ${host_str}; then
	printf "The Epiphany SDK build failed!\n"
	printf "\nAborting...\n"
	exit 1
fi

popd >& /dev/null

printf "The Epiphany SDK Build Completed successfully\n"
exit 0

