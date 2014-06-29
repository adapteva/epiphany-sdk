#!/bin/sh

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


#		       Script to build the complete SDK
#		       ================================

# There is an assumption the GNU tools have already been built and installed.

# Usage:
#
#     ./build-sdk.sh [-a | --arch | --host <arch>]
#                    [--arm | --x86 ]
#                    [-b | --bsp <bsp_name> ]
#                    [-d | --debug | -r | --release]
#                    [-l | --esdklibs <path> ]
#                    [-p | --esdkpath | --prefix <path>]
#                    [-h | --help]
#                    [--version]

# Some argument name alternatives are for consistency with autotools based
# scripts.

# -a <arch>
# --arch <arch>
# --host <arch>

#     The name of the architecture we are going to run on. Permitted names at
#     present are armv7l or x86_64.

# --arm

#     A synonym for --arch armv7l

# --x86

#     A synonym for --arch x86_64

# -b <bsp_name>
# --bsp <bsp_name>

#     The name of the BSP to use. This must be a valid directory in the bsps
#     subdirectory of epiphany-libs.

# -c
# --clean

#     Perform a clean build (default is not to clean)

# -d
# --debug
# -r
# --release

#    Install the debug or release versions of the tools (default is release).

# -l <path>
# --esdklibs <path>

#     The location of the epiphany-libs repository. May be relative to the
#     directory with this script, or an absolute directory. Defaults to
#     ../epiphan-libs.

# -p <path>
# --esdkpath <path>
# --prefix <path>

#     Where the tools are installed. Follows the hierarchy described in the
#     user manual. Must be an absolute path.

# -h
# --help

#     Print out summary of arguments

# --version

#     Print out the version of the SDK being built.

# Exit immediately if any command or pipe fails.
set -e

# -----------------------------------------------------------------------------
#
#			       Argument parsing
#
# -----------------------------------------------------------------------------

# Set the top level directory.
d=`dirname "$0"`
topdir=`(cd "$d/.." && pwd)`

# Default values

# Path to location of eSDK installation (must be an absolute path)
ESDKPATH="${topdir}/.."     # In user account, adjacent directory

# Revision number of new eSDK build
REV="5.13.09.10"
export REV

# Host machine architecture
ARCH="armv7l"

# List of available BSPs and default BSP
BSPS="zed_E16G3_512mb zed_E64G4_512mb parallella_E16G3_1GB"
BSP="parallella_E16G3_1GB"

# Default location of epiphany-libs.
ESDK_LIBS="${topdir}/epiphany-libs"

# Default version to install
VERSION=Release

# Should we do a clean build?
BUILD_FLAG=-a

# Parse options
getopt_string=`getopt -n build-sdk -o a:b:cdrl:p:h -l arch:,host: \
                   -l arm,x86 -l clean -l debug,release -l help -l version \
                   -l bsp: -l esdklibs: -l esdkpath:,prefix: \
                   -s sh -- "$@"`
eval set -- "$getopt_string"

while true
do
    case $1 in

	-a|--arch|--host)
	    shift
	    ARCH=$1
	    ;;

	--arm)
	    ARCH=armv7l;
	    ;;

	--x86)
	    ARCH=x86_64
	    ;;

	-b|--bsp)
	    shift
	    BSP=$1
	    ;;

	-c|--clean)
	    BUILD_FLAG="-c"
	    ;;

	-d|--debug)
	    VERSION=Debug
	    ;;

	-r|--release)
	    VERSION=Release
	    ;;

	-l|--esdklibs)
	    shift
	    ESDK_LIBS=$1
	    ;;

	-p|--esdkpath|--prefix)
	    shift
	    ESDKPATH=$1
	    ;;

	-h|--help)
	    echo "Epiphany SDK version ${REV}"
	    echo "Usage: ./build-sdk.sh [-a | --arch | --host <arch>]"
            echo "                      [--arm | --x86]"
            echo "                      [-b | --bsp <bsp_name> ]"
            echo "                      [-c | --clean]"
            echo "                      [-d | --debug | -r | --release]"
            echo "                      [-l | --esdklibs <path> ]"
            echo "                      [-p | --esdkpath | --prefix <path>]"
            echo "                      [-h | --help]"
            echo "                      [--version]"
	    exit 0
	    ;;

	--version)
	    shift
	    echo "Epiphany SDK version ${REV}"
	    exit 0
	    ;;

	--)
	    shift
	    break
	    ;;

	*)
	    echo "Internal error!"
	    echo $1
	    exit 1
	    ;;
    esac
    shift
done

# Argument validation
arch_valid="ERR"
if [ \( "x${ARCH}" = "xarmv7l" \) -o \( "x${ARCH}" = "x86_64" \) ]
then
    arch_valid="OK"
else
    echo "\"${ARCH}\" is not a valid SDK architecture"
fi

bsp_valid="ERR"
for bsp in ${BSPS}
do
    if [ "x${BSP}" = "x${bsp}" ]
    then
	bsp_valid="OK"
    fi
done

if [ "$bsp_valid" != "OK" ]
then
    echo "\"${BSP}\" is not a valid BSP"
fi

# Give up if either argument was invalid
if [   \( "${arch_valid}"  != "OK" \) -o \( "${bsp_valid}"   != "OK" \) ]
then
    exit 1
fi

# -----------------------------------------------------------------------------
#
#			  Build and install the SDK
#
# -----------------------------------------------------------------------------

EPIPHANY_HOME="${ESDKPATH}/esdk"
ESDK="${ESDKPATH}/esdk.${REV}"
HOSTNAME="host.${ARCH}"
HOST="${ESDK}/tools/${HOSTNAME}"
GNUNAME="e-gnu.${ARCH}"
GNU="${ESDK}/tools/${GNUNAME}"

# Add Epiphany and host GNU tool to path
PATH="${EPIPHANY_HOME}/tools/e-gnu/bin:${PATH}"
PATH="${EPIPHANY_HOME}/tools/host/bin:${PATH}"
export PATH
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


# Build the eSDK libraries from epiphany-libs repo. We use either -a (to build
# everything) or -c (to do a clean build of everything. From this point on we
# are in the epiphany libraries directory.
echo "Building eSDK libraries..."
cd ${ESDK_LIBS} > /dev/null 2>&1
./build-libs.sh ${BUILD_FLAG}


# Install components
echo "Installing eSDK components..."

# Install the current BSP
echo "-- Installing BSPs"
for bsp in ${BSPS}; do
	cp -Rd bsps/${bsp} ${ESDK}/bsps
done
ln -sTf ${BSP} ${ESDK}/bsps/current

# Install the XML parser library
echo "-- Installing eXML"
cd src/e-xml
cp -f ${VERSION}/libe-xml.so ${HOST}/lib
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
cp -f ${VERSION}/libe-loader.so ${HOST}/lib
cd ../../

# Install the Epiphnay GDB RSP Server
echo "-- Installing eServer"
cd src/e-server
cp -f ${VERSION}/e-server ${HOST}/bin/e-server.e
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
cp ${VERSION}/libe-lib.a ${ESDK}/tools/e-gnu/epiphany-elf/lib
cp include/*.h        ${ESDK}/tools/e-gnu/epiphany-elf/sys-include/
ln -sTf libe-lib.a    ${ESDK}/tools/e-gnu/epiphany-elf/lib/libelib.a
ln -sTf e_lib.h       ${ESDK}/tools/e-gnu/epiphany-elf/sys-include/e-lib.h
cd ../../


# Any special operations here...

echo "==============================================="
echo "| NOTE: The default BSP is set to ${BSP}"
echo "| Please make sure it matches your system,    |"
echo "| or chenge the settings in this build script |"
echo "==============================================="
