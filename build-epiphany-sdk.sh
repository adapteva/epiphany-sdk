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



################################################################################
#                                                                              #
#                              Shell functions                                 #
#                                                                              #
################################################################################

# Get the architecture from a triplet.

# This is the first field up to -, but with "arm" translated to "armv7l".

# @param[in] $1  triplet
# @return  The architecture of the triplet, but with arm translated to armv7l.
getarch () {
    triplet=$1


    if [ "x${triplet}" = "x" ]
    then
	arch=$(uname -m)
    else
	arch=`echo $triplet | sed -e 's/^\([^-]*\).*$/\1/'`
    fi

    if [ "x${arch}" = "xarm" ]
    then
	arch="armv7l"
    fi

    echo ${arch}
}


# Git branch name for build default
BRANCH="master"

# Host architecture triplet for toolchain
# Likely value (with -c) if cross-building for Pubuntu is "arm-linux-gnueabihf"
TOOLCHAIN_CANONICAL_HOST=""

# Host e-hal e-loader etc. should be compiled for, e.g., Parallella.
ELIBS_CANONICAL_HOST="arm-linux-gnueabihf"

jobs_str=""

while getopts c:e:Cdj:r:Rt:h arg; do
	case $arg in

	c)
		TOOLCHAIN_CANONICAL_HOST=$OPTARG
		;;

	e)
		ELIBS_CANONICAL_HOST=$OPTARG
		;;


	C)
		CLEAN=yes
		;;

	d)
		DEBUG=yes
		;;

	j)
		jobs_str="--jobs ${OPTARG}"
		;;

	r)
		echo "Warning: -r is deprecated"
		;;

	R)
		echo "Warning: -R is deprecated"
		;;

	t)
		have_branch_arg="yes"
		BRANCH=$OPTARG
		;;

	h)
		echo "Usage: ./build-epiphany-sdk.sh "
		echo "    [-c <host prefix>]: Tool-chain architecture prefix."
		echo "                        Only needed for Canadian cross builds."
		echo "    [-e <host prefix>]: epiphany-libs architecture prefix."
		echo "                        Default: ${ELIBS_CANONICAL_HOST}"
		echo "    [-C]:               Clean before start building."
		echo "    [-d]:               Enable building with debug symbols."
		echo "    [-j <count>]:       Specify that parallel make should run at"
		echo "                        most <count> jobs."
		echo "    [-t <tag_name>]:    The tag name (or branch name) for the SDK sources."
		echo "                        Default: $BRANCH"
		echo "    [-h]:               Show usage"
		echo ""
		echo "Environment variables:"
		echo "    EPIPHANY_BUILDROOT"
		echo "    EPIPHANY_BUILD_TOOLCHAIN"
		echo "    EPIPHANY_DESTDIR"
		echo "    PARALLELLA_LINUX_HOME"
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

if [ "x${EPIPHANY_BUILDROOT}" = "x" ]; then
	d=$(dirname "$0")
	EPIPHANY_BUILDROOT=$(cd "$d/.." && pwd)
fi

if [ "x${EPIPHANY_DESTDIR}" = "x" ]; then
	EPIPHANY_DESTDIR=${EPIPHANY_BUILDROOT}
fi

# Define the basedir
d=`dirname "$0"`
basedir=`(cd "$d/.." && pwd)`

. ${basedir}/sdk/define-release.sh

ESDK="${EPIPHANY_DESTDIR}/esdk.${RELEASE}"
HOSTNAME="host.$(getarch ${ELIBS_CANONICAL_HOST})"
HOST="${ESDK}/tools/${HOSTNAME}"
GNUNAME="e-gnu.$(getarch ${TOOLCHAIN_CANONICAL_HOST})"
GNU="${ESDK}/tools/${GNUNAME}"


echo '********************************************'
echo '************ Epiphany SDK Build ************'
echo '********************************************'

echo ""
echo "Environment settings:"
echo ""
echo "    EPIPHANY_BUILDROOT=${EPIPHANY_BUILDROOT}"
echo "    EPIPHANY_DESTDIR=${EPIPHANY_DESTDIR}"
echo ""
echo "Build settings:"
echo ""
echo "    eSDK install directory:       ${ESDK}"
echo "    Tool chain host prefix:       ${TOOLCHAIN_CANONICAL_HOST}"
echo "    epiphany-libs host prefix:    ${ELIBS_CANONICAL_HOST}"
echo "    Build version:                ${RELEASE}"
echo "    Build from branch or tag:     ${BRANCH}"
echo ""

cd ${EPIPHANY_BUILDROOT}/sdk || exit 1

# Create the SDK tree
echo "Creating the eSDK directory tree..."

mkdir -p ${ESDK} ${ESDK}/bsps ${ESDK}/tools
mkdir -p ${HOST}/lib ${HOST}/include ${HOST}/bin
mkdir -p ${GNU}

# Create toolchain symbolic links (force overwrite if exists)
(
    cd ${ESDK}/..
    ln -sTf esdk.${RELEASE} esdk
    cd ${ESDK}/tools
    ln -sTf ${HOSTNAME} host
    ln -sTf ${GNUNAME}  e-gnu
)


# Sort out arg for cross compiling / Canadian cross

# Toolchain host architecture prefix
if [ "x${TOOLCHAIN_CANONICAL_HOST}" = "x" ]; then
	host_str=""
else
	host_str="--host ${TOOLCHAIN_CANONICAL_HOST}"
fi

# epiphany-libs host architecture prefix
if [ "x${ELIBS_CANONICAL_HOST}" = "x" ]; then
	sdk_host_str=""
else
	sdk_host_str="--host ${ELIBS_CANONICAL_HOST}"
fi



if [ "xyes" = "x$DEBUG" ]; then
	export CFLAGS="-g ${CFLAGS}"
	export CXXFLAGS="-g ${CXXFLAGS}"
	sdk_debug_str="--debug"
else
	sdk_debug_str=""
fi

if [ "xyes" = "x$CLEAN" ]; then
	toolchain_clean_str="--clean-build --clean-host"
	sdk_clean_str="--clean"
else
	toolchain_clean_str=""
	sdk_clean_str=""
fi

# If we're building for same host buildmachine the toolchain will naturally be
# installed to ${GNU}. Otherwise install into builds/
if [ "x${TOOLCHAIN_CANONICAL_HOST}" = "x" -o \
     "x$(getarch ${TOOLCHAIN_CANONICAL_HOST})" = "x$(uname -m)" ]
then
    id_buildarch_toolchain="${GNU}"
    buildarch_install_dir_str=""
else
    id_buildarch_toolchain=${EPIPHANY_BUILDROOT}/builds/id-$(uname -m)-${RELEASE}-toolchain
    buildarch_install_dir_str="--install-dir-build ${id_buildarch_toolchain}"
fi

# install-sdk.sh must have an Epiphany toolchain in the PATH
export PATH="${id_buildarch_toolchain}/bin:${PATH}"


# TODO: We only want multicore-sim for x86_64 but we don't have the
# infrastructure for checking host arch in this file yet. However,
# 'build-toolchain.sh' will emit a warning and refuse to build it for hosts
# other than x86_64. So we'll rely on that for the time being.
multicore_sim_str="--multicore-sim"

if ! ./download-components.sh --clone; then
	printf "\nAborting...\n"
	exit 1
fi

if [ "$EPIPHANY_BUILD_TOOLCHAIN" != "no" ]; then
	# Build the toolchain (this will take a while)
	if ! ./build-toolchain.sh ${jobs_str} \
		--install-dir-host ${GNU} \
		${buildarch_install_dir_str} \
		${host_str} ${toolchain_clean_str} ${multicore_sim_str}; then
		printf "The toolchain build failed!\n"
		printf "\nAborting...\n"
		exit 1
	fi
fi

if [ -z "${PARALLELLA_LINUX_HOME}" ]; then
    PARALLELLA_LINUX_HOME=${basedir}/parallella-linux
    export PARALLELLA_LINUX_HOME
fi

# build the epiphany-libs and install the SDK
# TODO: We shouldn't need to pass in ${RELEASE} and ${BRANCH}.
if ! ./install-sdk.sh -n ${RELEASE} -x ${BRANCH} \
	--prefix ${EPIPHANY_DESTDIR} \
	${sdk_host_str} ${sdk_debug_str} ${sdk_clean_str}; then
	printf "The Epiphany SDK build failed!\n"
	printf "\nAborting...\n"
	exit 1
fi

printf "The Epiphany SDK Build Completed successfully\n"
exit 0

