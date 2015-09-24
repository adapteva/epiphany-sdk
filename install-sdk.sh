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
# FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU General Public License for
# more details.

# You should have received a copy of the GNU General Public License along
# with this program.  If not, see <http://www.gnu.org/licenses/>.


#			   Script to build the complete SDK
#			   ================================

# There is an assumption the GNU tools have already been built and installed.

# Usage:
#
#	  ./install-sdk.sh [-c | --host <arch triplet>]
#					 [-C | --clean]
#					 [-b | --bsp <bsp_name>]
#					 [--debug | --no-debug]
#					 [-l | --esdklibs <path> ]
#					 [-p | --esdkpath | --prefix <path>]
#					 [-h | --help]
#					 [--version]

# Some argument name alternatives are for consistency with autotools based
# scripts.


# -b <bsp_name>
# --bsp <bsp_name>

#	  The name of the BSP to use. This must be a valid directory in the bsps
#	  subdirectory of epiphany-libs.


# -c <arch triplet>
# --host <arch triplet>

#    Canonical host system name
#    The ARM cross toolchain prefix. Usinng this option will override
#    the CROSS_COMPILE environment varible.
#    Likely value if cross-building on Ubuntu is "arm-linux-gnueabihf"

# -C
# --clean

#	 Clean before building


# --debug | --no-debug

#	 Install the debug or release versions of the tools (default is release).


# -l <path>
# --esdklibs <path>

#	  The location of the epiphany-libs repository. May be relative to the
#	  directory with this script, or an absolute directory. Defaults to
#	  ../epiphan-libs.

# -p <path>
# --esdkpath <path>
# --prefix <path>

#	  Where the tools are installed. Follows the hierarchy described in the
#	  user manual. Must be an absolute path.


# -h
# --help

#	  Print out summary of arguments

# --version

#	  Print out the version of the SDK being built.

# Exit immediately if any command or pipe fails.
set -e


# Set the top level directory.
d=`dirname "$0"`
basedir=`(cd "$d/.." && pwd)`

# Common functions
. ${basedir}/sdk/common-functions

# Set the release parameters
. ${basedir}/sdk/define-release.sh

# Set up a clean log
logfile=${LOGDIR}/build-$(date -u +%F-%H%M).log
rm -f "${logfile}"
echo "Logging to ${logfile}"
touch ${logfile}


# -----------------------------------------------------------------------------
#
#				   Argument parsing
#
# -----------------------------------------------------------------------------

# Default values

# Path to location of eSDK installation (must be an absolute path)
ESDKPATH="${basedir}"	 # In user account, adjacent directory

# Default Revision number of new eSDK build
REV="DevBuild"
export REV

# List of available BSPs and default BSP
BSPS="zed_E16G3_512mb zed_E64G4_512mb parallella_E16G3_1GB"
BSP="parallella_E16G3_1GB"

# Default location of epiphany-libs.
ESDK_LIBS="${basedir}/epiphany-libs"

# The default branch for cloning/checkout
BRANCH="master"

# Default version to install
BLD_VERSION=Release

# Parse options
getopt_string=`getopt -n install-sdk -o b:c:Cl:p:n:x:h \
				   -l host: \
				   -l debug -l no-debug -l help -l version \
				   -l bsp: -l bldname: -l branch: -l esdklibs: -l esdkpath:,prefix: \
				   -l clean \
				   -s sh -- "$@"`
eval set -- "$getopt_string"

while true
do
	case $1 in

	-c|--host)
		shift
		host=$1;
		;;

	-c|--clean)
		CLEAN=yes
		;;

	-b|--bsp)
		shift
		BSP=$1
		;;

	--debug)
		BLD_VERSION=Debug
		;;

	--no-debug)
		BLD_VERSION=Release
		;;

	-l|--esdklibs)
		shift
		ESDK_LIBS=$1
		;;

	-p|--esdkpath|--prefix)
		shift
		ESDKPATH=$1
		;;

    -n| --bldname)
		shift
		REV=$1
		;;

    -x| --branch)
		shift
		BRANCH=$1
		;;

	-h|--help)
		echo "Epiphany SDK version ${REV}"
		echo "Usage: ./build-sdk.sh [-c | --host <arch triplet>]"
			echo "						[-b | --bsp <bsp_name> ]"
			echo "						[-C | --clean ]"
			echo "						[--debug | --no-debug]"
			echo "						[-l | --esdklibs <path> ]"
			echo "						[-p | --esdkpath | --prefix <path>]"
			echo "						[-n | --bldname]"
			echo "						[-x | --branch]"
			echo "						[-h | --help]"
			echo "						[--version]"
			echo ""
			echo "The arguments provided above will override the values of "
			echo "the following environment variables:"
			echo ""
			echo "\tBSP"
			echo "\tBLD_VERSION"
			echo "\tCROSS_COMPILE"
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


# Sort out Canadian cross stuff. First is it really a canadian cross
build_arch=$(getarch $(uname -m))
if [ "x" != "x${host}" ]
then
    host_arch=`getarch ${host}`

    if [ "x${host_arch}" = "x${build_arch}" ]
    then
	# Not really a Canadian Cross
	host=
    else
	export CROSS_COMPILE=${host}-
    fi
else
    host_arch=${build_arch}
fi

# Argument validation
host_arch_valid="ERR"
if [ \( "x${host_arch}" = "xarmv7l" \) -o \( "x${host_arch}" = "x86_64" \) ]
then
	host_arch_valid="OK"
else
	echo "\"${host_arch}\" is not a valid SDK architecture"
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
if [   \( "${host_arch_valid}"  != "OK" \) -o \( "${bsp_valid}"	 != "OK" \) ]
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
HOSTNAME="host.${host_arch}"
HOST="${ESDK}/tools/${HOSTNAME}"
GNUNAME="e-gnu.${host_arch}"
GNU="${ESDK}/tools/${GNUNAME}"

# Add Epiphany toolchain to path
# Make sure we include the path to the right tools if we do canadian cross.
org_path=${PATH}
for p in "${basedir}/builds/id-${build_arch}-${RELEASE}-toolchain/bin" \
	 "${EPIPHANY_HOME}/tools/e-gnu.${build_arch}/bin" \
	 "/opt/adapteva/esdk.${RELEASE}/tools/e-gnu.${build_arch}/bin"
do
    PATH="${p}:${org_path}"
    check_toolchain "epiphany-elf-" && break
done
unset org_path

export EPIPHANY_PREFIX EPIPHANY_HOME PATH

# Check that we have all build tools
if ! check_toolchain "epiphany-elf-"
then
    echo "Epiphany toolchain not found on build machine"
    exit 1
fi

if [ "x" != "x${CROSS_COMPILE}" ]; then
    if ! check_toolchain "${CROSS_COMPILE}"
    then
	echo "Cross-compile toolchain not found on build machine"
	exit 1
    fi
fi

echo ""
echo "=============================================="
echo "Build Settings:"
echo ""
echo "Build Rev     = $REV"
echo "Branch        = $BRANCH"
echo "EPIPHANY_HOME = $EPIPHANY_HOME"
echo "ESDK          = $ESDK"
echo "HOSTNAME      = $HOSTNAME"
echo "GNUNAME       = $GNUNAME"
echo "GNU           = $GNU"
echo "PATH          = $PATH"
echo "=============================================="
echo ""

# Create the SDK tree and set default symlinks
if [ ! -d ${ESDK} ]; then
	echo "Creating the eSDK directory tree..."

	mkdir -p ${ESDK}/bsps ${ESDK}/tools

	ln -sTf "esdk.${REV}" ${ESDKPATH}/esdk
	ln -sTf ${HOSTNAME} ${ESDK}/tools/host
	ln -sTf ${GNUNAME}	${ESDK}/tools/e-gnu
fi

if [ ! -d ${HOST} ]; then
	mkdir -p ${HOST}/lib ${HOST}/include ${HOST}/bin
fi

if [ ! -d ${GNU} ]; then
	mkdir -p ${GNU}
fi

# Check prerequisites
if [ ! -d "${ESDK}/tools/e-gnu/epiphany-elf/lib" ]; then
	echo "${ESDK}/tools/e-gnu/epiphany-elf/lib not found..."
	echo "Please install the Epiphany GNU tools suite first at ${GNU}"
	exit 1
fi


if ! cd ${basedir} > /dev/null 2>&1
then
    echo "ERROR: Could not change directory to ${basedir}"
    exit 1
fi

check_dir_exists "epiphany-libs" || failedbuild

# Checkout and pull repos if necessary
# TODO: Add flags for autopull and autocheckout
autopull="--auto-pull"
autocheckout="--auto-checkout"
regex="sdk\|parallella"
if ! ${basedir}/sdk/get-versions.sh ${basedir} sdk/components.conf \
     ${logfile} ${auto_pull} ${auto_checkout} \
     --regex ${regex}
then
    echo "ERROR: Could not get correct versions of tools"
    exit 1
fi


# Build the eSDK libraries from epiphany-libs repo. From this point on we are
# in the epiphany libraries directory.
cd ${ESDK_LIBS} > /dev/null 2>&1

if [ "xyes" = "x$CLEAN" ]; then
	build_elibs_flag="-c"
else
	build_elibs_flag="-a"
fi

echo "Building eSDK libraries..."
echo $PATH
if ! ./build-libs.sh ${build_elibs_flag} ; then
	echo "epiphany-libs failed to build"
	exit 1
fi

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
cp -f ${BLD_VERSION}/libe-xml.so ${HOST}/lib
cd ../../

# Install the Epiphnay HAL library
echo "-- Installing eHAL"
cd src/e-hal
ln -sTf ../../../bsps/current/libe-hal.so ${HOST}/lib/libe-hal.so
cp -f src/epiphany-hal.h				  ${HOST}/include
cp -f src/epiphany-hal-data.h			  ${HOST}/include
cp -f src/epiphany-hal-data-local.h		  ${HOST}/include
cp -f src/epiphany-hal-api.h			  ${HOST}/include
cp -f src/epiphany-shm-manager.h		  ${HOST}/include
ln -sTf epiphany-hal.h					  ${HOST}/include/e-hal.h
ln -sTf epiphany-hal.h					  ${HOST}/include/e_hal.h
cd ../../

# Install the Epiphnay Loader library
echo "-- Installing eLoader"
cd src/e-loader
cp -f src/e-loader.h ${HOST}/include
ln -sTf e-loader.h	 ${HOST}/include/e_loader.h
cp -f ${BLD_VERSION}/libe-loader.so ${HOST}/lib
cd ../../

# Install the Epiphnay GDB RSP Server
echo "-- Installing eServer"
cd src/e-server
cp -f ${BLD_VERSION}/e-server ${HOST}/bin/e-server
cd ../../

# Install the Epiphnay Utilities
echo "-- Installing eUtilities"
cd src/e-utils
cp -f e-reset/e-reset                                  ${HOST}/bin/
cp -f e-loader/Debug/e-loader                          ${HOST}/bin/
cp -f e-read/Debug/e-read                              ${HOST}/bin/
cp -f e-write/Debug/e-write                            ${HOST}/bin/
cp -f e-hw-rev/e-hw-rev                                ${HOST}/bin/
cp -f e-trace/include/a_trace.h                        ${HOST}/include
cp -f e-objcopy                                        ${HOST}/bin
cp -f e-trace-dump/${BLD_VERSION}/e-trace-dump         ${HOST}/bin
cp -f e-trace-server/${BLD_VERSION}/e-trace-server     ${HOST}/bin
cp -f e-trace/${BLD_VERSION}/libe-trace.so             ${HOST}/lib
cp -f e-clear-shmtable/${BLD_VERSION}/e-clear-shmtable ${HOST}/bin
cd ../../

# Install the Epiphnay Runtime Library
echo "-- Installing eLib"
cd src/e-lib
cp ${BLD_VERSION}/libe-lib.a ${ESDK}/tools/e-gnu/epiphany-elf/lib
cp include/*.h		  ${ESDK}/tools/e-gnu/epiphany-elf/include/
ln -sTf libe-lib.a	  ${ESDK}/tools/e-gnu/epiphany-elf/lib/libelib.a
ln -sTf e_lib.h		  ${ESDK}/tools/e-gnu/epiphany-elf/include/e-lib.h


cd ${basedir}

# Copy top files
echo "Copying top files"
cp -d ./sdk/README    ${ESDK}
cp -d ./sdk/COPYING   ${ESDK}
cp -d ./sdk/setup.sh  ${ESDK}
cp -d ./sdk/setup.csh ${ESDK}

printf "=%.0s" $(seq 1 60)
printf "\n"
printf "| %-56s |\n" "NOTE: The default BSP is set to \"${BSP}\""
printf "| %-56s |\n" "Please make sure it matches your system,"
printf "| %-56s |\n" "or change the settings in this build script"
printf "=%.0s" $(seq 1 60)
printf "\n"

exit 0
