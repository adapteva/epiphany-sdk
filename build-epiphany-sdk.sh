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

# Define the basedir
d=`dirname "$0"`
basedir=`(cd "$d/.." && pwd)`

# Common functions
. ${basedir}/sdk/common-functions

# Set the release parameters
. ${basedir}/sdk/define-release.sh

BRANCH=""
# Git branch name for build default
if [ -e ${basedir}/sdk/.git ] && which git >/dev/null; then
	BRANCH=$(cd ${basedir}/sdk && git rev-parse --abbrev-ref HEAD || true)
fi

if [ "x${BRANCH}" = "x" ]; then
	BRANCH="master"
fi

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
		echo "Usage: ${0}"
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
		echo "    ESDK_BUILDROOT"
		echo "    ESDK_BUILD_TOOLCHAIN"
		echo "    ESDK_DESTDIR"
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

if [ "x${ESDK_BUILDROOT}" = "x" ]; then
	d=$(dirname "$0")
	ESDK_BUILDROOT=$(cd "$d/.." && pwd)
fi

if [ "x${ESDK_DESTDIR}" = "x" ]; then
	ESDK_DESTDIR="$(echo ${ESDK_BUILDROOT}/esdk.${RELEASE}/ | sed s,[/]*$,/,g)"
else
	ESDK_DESTDIR="$(echo ${ESDK_DESTDIR}/ | sed s,[/]*$,/,g)"
fi

ESDK_PREFIX=/opt/adapteva
ESDK="${ESDK_PREFIX}/esdk.${RELEASE}"
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
echo "    ESDK_BUILDROOT=${ESDK_BUILDROOT}"
echo "    ESDK_DESTDIR=${ESDK_DESTDIR}"
echo ""
echo "Build settings:"
echo ""
echo "    eSDK install directory:       ${ESDK_DESTDIR}"
echo "    eSDK prefix directory:        ${ESDK}"
echo "    Tool chain host prefix:       ${TOOLCHAIN_CANONICAL_HOST}"
echo "    epiphany-libs host prefix:    ${ELIBS_CANONICAL_HOST}"
echo "    Build version:                ${RELEASE}"
echo "    Build from branch or tag:     ${BRANCH}"
echo ""

cd ${ESDK_BUILDROOT}/sdk || exit 1

# Create the SDK tree
echo "Creating the eSDK directory tree..."

mkdir -p ${ESDK_DESTDIR}${ESDK} ${ESDK_DESTDIR}${ESDK}/bsps ${ESDK_DESTDIR}${ESDK}/tools
mkdir -p ${ESDK_DESTDIR}${HOST}/lib ${ESDK_DESTDIR}${HOST}/include ${ESDK_DESTDIR}${HOST}/bin
mkdir -p ${ESDK_DESTDIR}${GNU}/epiphany-elf

# Create toolchain symbolic links (force overwrite if exists)
(
    cd ${ESDK_DESTDIR}${ESDK}/..
    ln -sTf esdk.${RELEASE} esdk
    cd ${ESDK_DESTDIR}${ESDK}/tools
    ln -sTf ${HOSTNAME} host
    ln -sTf ${GNUNAME}  e-gnu
    cd ${ESDK_DESTDIR}${HOST}
    ln -sTf ../${GNUNAME}/epiphany-elf epiphany-elf
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

CFLAGS_FOR_TARGET=${CFLAGS_FOR_TARGET:-"-O2"}
CXXFLAGS_FOR_TARGET=${CXXFLAGS_FOR_TARGET:-"-O2"}
CFLAGS=${CFLAGS:-"-O2"}
CXXFLAGS=${CFLAGS:-"-O2"}

if [ "xyes" = "x$DEBUG" ]; then
	export CFLAGS="-g ${CFLAGS}"
	export CXXFLAGS="-g ${CXXFLAGS}"
	sdk_debug_str="--debug"
else
	sdk_debug_str=""
fi

export CFLAGS="-fdebug-prefix-map=${basedir}=${ESDK}/src ${CFLAGS}"
export CXXLAGS="-fdebug-prefix-map=${basedir}=${ESDK}/src ${CXXFLAGS}"
export CFLAGS_FOR_TARGET="-fdebug-prefix-map=${basedir}=${ESDK}/src ${CFLAGS_FOR_TARGET}"
export CXXLAGS_FOR_TARGET="-fdebug-prefix-map=${basedir}=${ESDK}/src ${CXXFLAGS_FOR_TARGET}"

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
    id_buildarch_toolchain="${ESDK_DESTDIR}${GNU}"
    buildarch_install_dir_str=""
else
    id_buildarch_toolchain=${ESDK_BUILDROOT}/builds/id-$(uname -m)-${RELEASE}-toolchain
    buildarch_install_dir_str="--install-dir-build ${id_buildarch_toolchain}"
fi

# install-sdk.sh must have an Epiphany toolchain in the PATH
export PATH="${id_buildarch_toolchain}/bin:${PATH}"

if ! ./download-components.sh --clone; then
	printf "\nAborting...\n"
	exit 1
fi

if [ "$ESDK_BUILD_TOOLCHAIN" != "no" ]; then
	# Build the toolchain (this will take a while)
	# TODO: Remove --enable-cgen-maint and --enable-werror
	# before we release.
	if ! ./build-toolchain.sh ${jobs_str} \
		--install-dir-host ${GNU} \
		--destdir ${ESDK_DESTDIR} \
		--disable-werror \
		--enable-cgen-maint \
		${buildarch_install_dir_str} \
		${host_str} ${toolchain_clean_str}; then
		printf "The toolchain build failed!\n"
		printf "\nAborting...\n"
		exit 1
	fi
fi

# Build order for dependencies is:
# 1. e-lib
# 2. pal (host + epiphany device)
# 4. e-hal (depends on pal) + the rest of epiphany-libs

# Build epiphany-libs (w/o e-hal == only e-lib)
# We must clean because we share build directory w/ e-hal build
# below (ugly but works !!!)
if ! ./build-epiphany-libs.sh \
	${jobs_str} \
	--install-dir-host   ${HOST} \
	--install-dir-target ${GNU}/epiphany-elf \
	--install-dir-bsps   ${ESDK}/bsps \
	--destdir ${ESDK_DESTDIR} \
	--clean \
	--config-extra "--disable-ehal --enable-elib" \
	${sdk_host_str};
then
	printf "The epiphany-libs (elib) build failed!\n"
	printf "\nAborting...\n"
	exit 1
fi

# Build pal for host
export EPIPHANY_HOME=${ESDK_DESTDIR}/${ESDK}
if ! ./build-pal.sh \
	${jobs_str} \
	--install-dir-host   ${HOST} \
	--install-dir-target ${GNU}/epiphany-elf \
	--destdir ${ESDK_DESTDIR} \
	--config-extra "--enable-device-epiphany --enable-device-epiphany-sim" \
	${sdk_host_str} \
	${sdk_clean_str};
then
	printf "The pal build failed!\n"
	printf "\nAborting...\n"
	exit 1
fi
unset EPIPHANY_HOME

# Build e-hal + rest of epiphany-libs
if ! CFLAGS="-I${HOST}/include ${CFLAGS}" LDFLAGS="-L${HOST}/lib ${LDFLAGS}" \
     ./build-epiphany-libs.sh \
	${jobs_str} \
	--install-dir-host   ${HOST} \
	--install-dir-target ${GNU}/epiphany-elf \
	--install-dir-bsps   ${ESDK}/bsps \
	--destdir ${ESDK_DESTDIR} \
	--clean \
	--config-extra "--disable-elib --enable-ehal --enable-pal-target" \
	${sdk_host_str};
then
	printf "The epiphany-libs (e-hal) build failed!\n"
	printf "\nAborting...\n"
	exit 1
fi

# Copy top files
echo "Copying top files"
cp -d README    ${ESDK_DESTDIR}${ESDK}
cp -d COPYING   ${ESDK_DESTDIR}${ESDK}
cp -d setup.sh  ${ESDK_DESTDIR}${ESDK}
cp -d setup.csh ${ESDK_DESTDIR}${ESDK}

echo "Creating tarball"
tar czf ${ESDK_BUILDROOT}/esdk.${RELEASE}.tar.gz -C ${ESDK_DESTDIR}${ESDK_PREFIX} esdk.${RELEASE}

printf "The Epiphany SDK Build Completed successfully\n"
exit 0
