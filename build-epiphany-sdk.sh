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

# Whether we should check out release tags defined in define-release.sh
do_release="--no-release"

while getopts :a:c:Cdr:Rt:h arg; do
	case $arg in

	a)
		ARCH=$OPTARG
		;;

	c)
		ARCH_TRIPLET=$OPTARG
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
		do_release="--release"
		;;

	t)
		have_branch_arg="yes"
		BRANCH=$OPTARG
		;;

	h)
		echo "Usage: ./build-epiphany-sdk.sh "
		echo "    [-a <host arch>]: The host architecture. Default $ARCH"
		echo "    [-c <host triplet>]: The architecture triplet. Only needed for Canadian cross builds."
		echo "    [-C]: Clean before start building."
		echo "    [-d]: Enable building with debug symbols."
		echo "    [-r <revision>]: The revision string for the SDK. Default $REV"
		echo "    [-R]: Do release build. Use define-release.sh to get right tags"
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

# Define the basedir
d=`dirname "$0"`
basedir=`(cd "$d/.." && pwd)`


# Check args.
if [ "x--release" = "x${do_release}" ]
then
	if [ "xyes" = "x${have_rev_arg}" ]
	then
		echo "You cannot specify both release mode (-R) and revison (-r)"
		exit 1
	fi
	if [ "xyes" = "x${have_branch_arg}" ]
	then
		echo "You cannot specify both release mode (-R) and branch (-t)"
		exit 1
	fi
fi

if [ "x--release" = "x${do_release}" ]
then
	# Set the release parameters
	. ${basedir}/sdk/define-release.sh
	BRANCH=${RELEASE_TAG}
else
	RELEASE=${REV}
fi


ESDK="${EPIPHANY_BUILD_HOME}/esdk.${RELEASE}"
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


# Sort out arg for Canadian cross
if [ "x${ARCH_TRIPLET}" = "x" ]; then
	host_str=""
else
	host_str="--host ${ARCH_TRIPLET}"
fi

# Force gnu89 C standard. GCC 5+ defaults to gnu11, but some of the components
# in the 2015.1 release do not support that version of the standard.
export CFLAGS="-std=gnu89 ${CFLAGS}"

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
if [ "x${ARCH}" = "x$(uname -m)" ]
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

if [ "$EPIPHANY_BUILD_TOOLCHAIN" != "no" ]; then
	if ! ./download-toolchain.sh ${multicore_sim_str} ${do_release} --clone; then

		printf "\nAborting...\n"
		exit 1
	fi

	# Build the toolchain (this will take a while)
	if ! ./build-toolchain.sh --install-dir-host ${EPIPHANY_HOME}/tools/${GNUNAME} \
		${buildarch_install_dir_str} \
		${do_release} \
		${host_str} ${toolchain_clean_str} ${multicore_sim_str}; then
		printf "The toolchain build failed!\n"
		printf "\nAborting...\n"
		exit 1
	fi
fi

# TODO: Move to basedir
if [ ! -d "$PARALLELLA_LINUX_HOME" ]; then
	# Clone the parallella Linux source tree
	git clone https://github.com/parallella/parallella-linux.git -b main

	export PARALLELLA_LINUX_HOME=$PWD/parallella-linux
fi


# build the epiphany-libs and install the SDK
# TODO: We shouldn't need to pass in ${RELEASE} and ${BRANCH} when we say
# --release.

if ! ./install-sdk.sh -n ${RELEASE} -x ${BRANCH} ${do_release} \
	${host_str} ${sdk_debug_str} ${sdk_clean_str}; then
	printf "The Epiphany SDK build failed!\n"
	printf "\nAborting...\n"
	exit 1
fi

popd >& /dev/null

printf "The Epiphany SDK Build Completed successfully\n"
exit 0

