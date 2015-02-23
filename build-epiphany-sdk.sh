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



# Revision number of Epiphany SDK default
REV="RevUndefined"

# Git branch name for build default
BRANCH="master"

# Host architecture triplet for compiler tools
# Likely value (with -c) if cross-building on Ubuntu is "arm-linux-gnueabihf"
GNU_ARCH_PREFIX=""

ELIBS_ARCH_PREFIX="arm-linux-gnueabihf"

# Whether we should check out release tags defined in define-release.sh
do_release="--no-release"

while getopts c:e:Cdr:Rt:h arg; do
	case $arg in

	c)
		GNU_ARCH_PREFIX=$OPTARG
		;;

	e)
		ELIBS_ARCH_PREFIX=$OPTARG
		;;


	C)
		CLEAN=yes
		;;

	d)
		DEBUG=yes
		;;

	r)
		have_rev_arg="yes"
		REV=$OPTARG
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
		echo "                        Default: ${ELIBS_ARCH_PREFIX}"
		echo "    [-C]:               Clean before start building."
		echo "    [-d]:               Enable building with debug symbols."
		echo "    [-r <revision>]:    The revision string for the SDK."
		echo "                        Default $REV"
		echo "    [-t <tag_name>]:    The tag name (or branch name) for the SDK sources."
		echo "                        Default: $BRANCH"
		echo "    [-h]:               Show usage"
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

# Define the basedir
d=`dirname "$0"`
basedir=`(cd "$d/.." && pwd)`

. ${basedir}/sdk/define-release.sh

ESDK="${EPIPHANY_BUILD_HOME}/esdk.${RELEASE}"
HOSTNAME="host.$(getarch ${ELIBS_ARCH_PREFIX})"
HOST="${ESDK}/tools/${HOSTNAME}"
GNUNAME="e-gnu.$(getarch ${GNU_ARCH_PREFIX})"
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
echo "    Tool-chain host prefix:       ${GNU_ARCH_PREFIX}"
echo "    epiphany-libs host prefix:    ${ELIBS_ARCH_PREFIX}"
echo "    Build version:                ${RELEASE}"
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
ln -sTf "esdk.${RELEASE}" ${EPIPHANY_BUILD_HOME}/esdk
ln -sTf ${HOSTNAME} ${ESDK}/tools/host
ln -sTf ${GNUNAME}  ${ESDK}/tools/e-gnu


# Sort out arg for cross compiling / Canadian cross

# Toolchain host architecture prefix
if [ "x${GNU_ARCH_PREFIX}" = "x" ]; then
	host_str=""
else
	host_str="--host ${GNU_ARCH_PREFIX}"
fi

# epiphany-libs host architecture prefix
if [ "x${ELIBS_ARCH_PREFIX}" = "x" ]; then
	elibs_host_str=""
else
	elibs_host_str="--host ${ELIBS_ARCH_PREFIX}"
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
if [ "x${GNU_ARCH_PREFIX}" = "x" -o \
     "x$(getarch ${GNU_ARCH_PREFIX})" = "x$(uname -m)" ]
then
    id_buildarch_toolchain="${GNU}"
    buildarch_install_dir_str=""
else
    id_buildarch_toolchain=${EPIPHANY_BUILD_HOME}/builds/id-$(uname -m)-${RELEASE}-toolchain
    buildarch_install_dir_str="--install-dir-build ${id_buildarch_toolchain}"
fi

# install-sdk.sh must have an Epiphany toolchain in the PATH
export PATH="${id_buildarch_toolchain}/bin:${PATH}"


# TODO: We only want multicore-sim for x86_64 but we don't have the
# infrastructure for checking host arch in this file yet. However,
# 'build-toolchain.sh' will emit a warning and refuse to build it for hosts
# other than x86_64. So we'll rely on that for the time being.
multicore_sim_str="--multicore-sim"

if ! ./download-components.sh ${multicore_sim_str} --clone; then

	printf "\nAborting...\n"
	exit 1
fi

if [ "$EPIPHANY_BUILD_TOOLCHAIN" != "no" ]; then
	# Build the toolchain (this will take a while)
	if ! ./build-toolchain.sh --install-dir-host ${EPIPHANY_HOME}/tools/${GNUNAME} \
		${buildarch_install_dir_str} \
		${host_str} ${toolchain_clean_str} ${multicore_sim_str}; then
		printf "The toolchain build failed!\n"
		printf "\nAborting...\n"
		exit 1
	fi
fi

# TODO: Move to basedir
if [ ! -d "$PARALLELLA_LINUX_HOME" ]; then
	(
		cd ${basedir}
		# Clone the parallella Linux source tree
		git clone https://github.com/parallella/parallella-linux.git -b main
	)
	export PARALLELLA_LINUX_HOME=${basedir}/parallella-linux
fi


# build the epiphany-libs and install the SDK
# TODO: We shouldn't need to pass in ${RELEASE} and ${BRANCH}.
if ! ./install-sdk.sh -n ${RELEASE} -x ${BRANCH} \
	${elibs_host_str} ${sdk_debug_str} ${sdk_clean_str}; then
	printf "The Epiphany SDK build failed!\n"
	printf "\nAborting...\n"
	exit 1
fi

popd >& /dev/null

printf "The Epiphany SDK Build Completed successfully\n"
exit 0

