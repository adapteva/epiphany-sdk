#!/bin/bash

# Copyright (C) 2009 - 2013 Adapteva Inc.

# Contributor Yaniv Sapir <ysapir@adapteva.com>
# Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

# This file is a script to build key elements of the Epiphany SDK

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

set -e

# Path to location of eSDK installation
# (must be an absolute path)
ESDKPATH="/opt/adapteva" # Default system location
#ESDKPATH="${PWD}/.."     # In user account, adjacent directory

# Revision number of new eSDK build
export REV="5.13.09.10"

# Host machine architecture
ARCH="armv7l"
# ARCH="x86_64"

# List if available BSPs and default BSP
BSPS="zed_E16G3_512mb zed_E64G4_512mb parallella_E16G3_1GB"
#BSP="parallella_E16G3_1GB"
BSP=zed_E64G4_512mb

EPIPHANY_HOME="${ESDKPATH}/esdk"
ESDK="${ESDKPATH}/esdk.${REV}"
HOSTNAME="host.${ARCH}"
HOST="${ESDK}/tools/${HOSTNAME}"
GNUNAME="e-gnu.${ARCH}"
GNU="${ESDK}/tools/${GNUNAME}"
ESDK_LIBS="../epiphany-libs"

export PATH="${EPIPHANY_HOME}/tools/host/bin:${EPIPHANY_HOME}/tools/e-gnu/bin:${PATH}"
export EPIPHANY_HOME

echo "==============================================="
echo "| NOTE: The default BSP is set to ${BSP}"
echo "| Please make sure it matches your system,    |"
echo "| or chenge the settings in this build script |"
echo "==============================================="

# Create the SDK tree and set default symlinks
echo "Creating the eSDK directory tree..."
mkdir -p ${ESDK}
ln -sTf "esdk.${REV}" ${ESDKPATH}/esdk
mkdir -p ${ESDK}/bsps
mkdir -p ${ESDK}/docs
mkdir -p ${ESDK}/examples
mkdir -p ${ESDK}/tools

mkdir -p ${HOST}
mkdir -p ${GNU}
ln -sTf ${HOSTNAME} ${ESDK}/tools/host
ln -sTf ${GNUNAME}  ${ESDK}/tools/e-gnu

mkdir -p ${HOST}/lib
mkdir -p ${HOST}/include
mkdir -p ${HOST}/bin


# Check prerequisites
if [ ! -d "${ESDK}/tools/e-gnu/epiphany-elf/lib" ]; then
	echo "Please install the Epiphany GNU tools suite first at ${GNU}"
	exit 1
fi

if [ ! -d "${ESDK_LIBS}" ]; then
	echo "ERROR: Can't find the epiphany-libs repository!"
	exit 1
fi


# Copy top files
cp -d ./README    ${ESDK}
cp -d ./COPYING   ${ESDK}
cp -d ./setup.sh  ${ESDK}
cp -d ./setup.csh ${ESDK}


# Build the eSDK libraries from epiphany-libs repo
echo "Building eSDK libraries..."
pushd ${ESDK_LIBS} >& /dev/null

# No checking on the command line at present. Only -c (for clean first) and -d
# (for debug) will do anything really useful. Others (like the numbers 1-6)
# will actively confuse things!
./build-libs.sh $* -a
popd >& /dev/null

version=Release

for f in $*
do
    if [ "x$f" == "x-d" ]
    then
	version=Debug
    fi
done

# Install components
echo "Installing eSDK components..."

# Install the documentation and examples
cp -Rd docs     ${ESDK}
cp -Rd examples ${ESDK}


pushd ${ESDK_LIBS} >& /dev/null

# Install the current BSP
echo "-- Installing BSPs"
for bsp in ${BSPS}; do
	cp -Rd bsps/${bsp} ${ESDK}/bsps
done
ln -sTf ${BSP} ${ESDK}/bsps/current

# Install the XML parser library
echo "-- Installing eXML"
cd src/e-xml
cp -f ${version}/libe-xml.so ${HOST}/lib
cd ../../

# Install the Epiphnay HAL library
echo "-- Installing eHAL"
cd src/e-hal
ln -sTf ../../../bsps/current/libe-hal.so ${HOST}/lib/libe-hal.so
cp -f src/epiphany-hal.h                  ${HOST}/include
cp -f src/epiphany-hal-data.h             ${HOST}/include
cp -f src/epiphany-hal-data-local.h       ${HOST}/include
cp -f src/epiphany-hal-api.h              ${HOST}/include
ln -sTf epiphany-hal.h                    ${HOST}/include/e-hal.h
ln -sTf epiphany-hal.h                    ${HOST}/include/e_hal.h
cd ../../

# Install the Epiphnay Loader library
echo "-- Installing eLoader"
cd src/e-loader
cp -f src/e-loader.h ${HOST}/include
ln -sTf e-loader.h   ${HOST}/include/e_loader.h
cd ../../

# Install the Epiphnay GDB RSP Server
echo "-- Installing eServer"
cd src/e-server
cp -f ${version}/e-server ${HOST}/bin/e-server.e
cp -f e-server.sh      ${HOST}/bin/e-server
cd ../../

# Install the Epiphnay Utilities
echo "-- Installing eUtilities"
cd src/e-utils
cp -f e-reset/e-reset         ${HOST}/bin/e-reset.e
cp -f e-reset/e-reset.sh      ${HOST}/bin/e-reset
cp -f e-loader/Debug/e-loader ${HOST}/bin/e-loader.e
cp -f e-loader/e-loader.sh    ${HOST}/bin/e-loader
cp -f e-read/Debug/e-read     ${HOST}/bin/e-read.e
cp -f e-read/e-read.sh        ${HOST}/bin/e-read
cp -f e-write/Debug/e-write   ${HOST}/bin/e-write.e
cp -f e-write/e-write.sh      ${HOST}/bin/e-write
cp -f e-hw-rev/e-hw-rev       ${HOST}/bin/e-hw-rev.e
cp -f e-hw-rev/e-hw-rev.sh    ${HOST}/bin/e-hw-rev
cp -f e-objcopy               ${HOST}/bin
cd ../../

# Install the Epiphnay Runtime Library
echo "-- Installing eLib"
cd src/e-lib
cp ${version}/libe-lib.a ${ESDK}/tools/e-gnu/epiphany-elf/lib
cp include/*.h        ${ESDK}/tools/e-gnu/epiphany-elf/sys-include/
ln -sTf libe-lib.a    ${ESDK}/tools/e-gnu/epiphany-elf/lib/libelib.a
ln -sTf e_lib.h       ${ESDK}/tools/e-gnu/epiphany-elf/sys-include/e-lib.h
cd ../../

popd >& /dev/null


# Any special operations here...
ln -sf ../bsps/current/parallella_prototype_quick_start_guide.pdf ${ESDK}/docs

echo "==============================================="
echo "| NOTE: The default BSP is set to ${BSP}"
echo "| Please make sure it matches your system,    |"
echo "| or chenge the settings in this build script |"
echo "==============================================="

