#!/usr/bin/env bash

# Copyright (C) 2009-2014 Adapteva Inc

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
# Host machine architecture
ARCH="armv7l"

# Revision number of Epiphany SDK
REV=${1:-"DevBuild"}

# Git branch name for build
BRANCH=${2:-"master"}

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

if [ -z $EPIPHANY_BUILD_HOME ]; then
	echo "Please define the environment variable EPIPHANY_BUILD_HOME and "
	echo "rerun the build-epiphany-sdk.sh script."
	exit 1
fi

cd $EPIPHANY_BUILD_HOME

pushd sdk >& /dev/null

if [ "$EPIPHANY_BUILD_TOOLCHAIN" != "no" ]; then
    # Run the download script
    if ! ./download-toolchain.sh --clone; then
	## If we fail, download script will report failure
	printf "\nAborting...\n"
	exit 1
    fi
	#Build the toolchain (this will take a while)
    if ! ./build-toolchain.sh; then
	printf "The toolchain build failed!\n"
	printf "\nAborting...\n"
	exit 1
    fi
fi

if [ ! -d ../esdk/tools/${GNUNAME}/ ]; then
	# Create the SDK tree and set default symlinks
	# Note the install-sdk.sh script will attempt to create
	# this toolchain tree but it does not Install the
	# toolchain!!
	echo "Creating the eSDK directory tree..."

	mkdir -p ${ESDK}
	ln -sT "esdk.${REV}" ${EPIPHANY_BUILD_HOME}/esdk
	mkdir -p ${ESDK}/bsps
	mkdir -p ${ESDK}/tools

	mkdir -p ${HOST}
	mkdir -p ${GNU}
	ln -s ${HOSTNAME} ${ESDK}/tools/host
	ln -s ${GNUNAME}  ${ESDK}/tools/e-gnu

	mkdir -p ${HOST}/lib
	mkdir -p ${HOST}/include
	mkdir -p ${HOST}/bin
fi

if [ ! -d "$PARALLELLA_LINUX_HOME" ]; then
	# Clone the parallella Linux source tree
	if ! git clone https://github.com/parallella/parallella-linux.git -b main; then
		printf "The Epiphany SDK build failed!\n"
		printf "\nAborting...\n"
	fi

	export PARALLELLA_LINUX_HOME=$PWD/parallella-linux
fi

# build the epiphany-libs and install the SDK
if ! ./install-sdk.sh -n $REV -x $BRANCH; then
	printf "The Epiphany SDK build failed!\n"
	printf "\nAborting...\n"
	exit 1
fi

popd >& /dev/null

printf "The Epiphany SDK Build Completed successfully\n"
exit 0

