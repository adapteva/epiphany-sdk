#!/bin/bash

set -e

REV="4.13.03.30"
ESDKPATH="/opt/adapteva"
EPIPHANY_HOME="${ESDKPATH}/esdk"
ESDK="${ESDKPATH}/esdk.${REV}"
ARCH='armv7l'
HOSTNAME="host.${ARCH}"
HOST="${ESDK}/tools/${HOSTNAME}"
GNUNAME="e-gnu.${ARCH}"
GNU="${ESDK}/tools/${GNUNAME}"
BSP='zed_E64G4_512mb'
ESDK_LIBS='../epiphany-libs'

export PATH="${EPIPHANY_HOME}/tools/e-gnu/bin:${EPIPHANY_HOME}/tools/host/bin:${PATH}"

echo "==============================================="
echo "| NOTE: The default BSP is set to ${BSP}      |"
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
ln -sTf ${GNUNAME} ${ESDK}/tools/e-gnu

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


# Build the eSDK libraries from epiphany-libs repo
echo "Building eSDK libraries..."
pushd ${ESDK_LIBS} >& /dev/null
./build-libs.sh
popd >& /dev/null


# Install components
echo "Installing eSDK components..."

# Install the documentation and examples
cp -Rd docs     ${ESDK}
cp -Rd examples ${ESDK}


pushd ${ESDK_LIBS} >& /dev/null

# Install the current BSP
cp -Rd bsps/${BSP} ${ESDK}/bsps
ln -sTf ${BSP} ${ESDK}/bsps/current

# Install the XML parser library
cd src/e-xml
cp -f Release/libe-xml.so ${HOST}/lib
cd ../../

# Install the Epiphnay HAL library
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
cd src/e-loader
cp -f src/e-loader.h ${HOST}/include
ln -sTf e-loader.h   ${HOST}/include/e_loader.h
cd ../../

# Install the Epiphnay GDB RSP Server
cd src/e-server
cp -f Release/e-server ${HOST}/bin
cd ../../

# Install the Epiphnay GNU Tools wrappers
cd src/e-utils
cp -f e-objcopy ${HOST}/bin
cd ../../

# Install the Epiphnay Runtime Library
cd src/e-lib
cp Release/libe-lib.a ${ESDK}/tools/e-gnu/epiphany-elf/lib
cp include/*.h        ${ESDK}/tools/e-gnu/epiphany-elf/sys-include/
cd ../../

popd >& /dev/null


# Any special operations here...

