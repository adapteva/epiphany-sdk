#!/bin/bash

# Copyright (C) 2009 - 2013 Adapteva Inc.

# Contributor Yaniv Sapir <ysapir@adapteva.com>
# Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

# This file is a script to build and install just the e-hal library.

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
#ESDKPATH="/opt/adapteva" # Default system location
ESDKPATH="${PWD}/.."     # In user account, adjacent directory

# Revision number of new eSDK build
REV="5.13.09.10"

# Host machine architecture
# ARCH="armv7l"
ARCH="x86_64"

# List if available BSPs and default BSP
BSPS="zed_E16G3_512mb zed_E64G4_512mb parallella_E16G3_1GB"
BSP="parallella_E16G3_1GB"

EPIPHANY_HOME="${ESDKPATH}/esdk"
ESDK="${ESDKPATH}/esdk.${REV}"
HOSTNAME="host.${ARCH}"
HOST="${ESDK}/tools/${HOSTNAME}"
GNUNAME="e-gnu.${ARCH}"
GNU="${ESDK}/tools/${GNUNAME}"
ESDK_LIBS="../epiphany-libs"

export PATH="${EPIPHANY_HOME}/tools/host/bin:${EPIPHANY_HOME}/tools/e-gnu/bin:${PATH}"
export EPIPHANY_HOME

# Check prerequisites
if [ ! -d "${ESDK}/tools/e-gnu/epiphany-elf/lib" ]; then
	echo "Please install the Epiphany GNU tools suite first at ${GNU}"
	exit 1
fi

if [ ! -d "${ESDK_LIBS}" ]; then
	echo "ERROR: Can't find the epiphany-libs repository!"
	exit 1
fi


# Build just the e-hal library from epiphany-libs repo
echo "Building eSDK libraries..."
pushd ${ESDK_LIBS} >& /dev/null
./build-libs.sh e-hal
popd >& /dev/null


pushd ${ESDK_LIBS} >& /dev/null

# Install the current BSP
echo "-- Installing BSPs"
for bsp in ${BSPS}; do
	cp -Rd bsps/${bsp} ${ESDK}/bsps
done
ln -sTf ${BSP} ${ESDK}/bsps/current

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
