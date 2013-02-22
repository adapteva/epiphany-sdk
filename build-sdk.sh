#!/bin/bash

set -e

ESDK=${EPIPHANY_HOME}
ARCH='armv7l'
HOST=${ESDK}/tools/host/${ARCH}
BSP='zed_E64G4_808_512mb'
ESDK_LIBS='../epiphany-libs'

if [ ! -d "${ESDK}/tools/host/lib" ]; then
	mkdir -p ${ESDK}/tools/host/${ARCH}/lib
	mkdir -p ${ESDK}/tools/host/${ARCH}/include
	mkdir -p ${ESDK}/tools/host/${ARCH}/bin
fi

if [ ! -d "${ESDK}/bsps" ]; then
	mkdir -p ${ESDK}/bsps
fi

if [ ! -d "${ESDK}/tools/e-gnu/epiphany-elf/lib" ]; then
	echo "Please install the Epiphany GNU tools suite first at ${ESDK}/tools/e-gnu!"
	exit
fi

if [ ! -d "${ESDK_LIBS}" ]
#	echo "Please make sure the epiphany-libs repo is placed correctly!"
	echo "ERROR: Can't find the epiphany-libs repository!"
	exit 1
fi


echo "Building eSDK libraries..."
pushd ${ESDK_LIBS} >& /dev/nul
./build-libs.sh
popd >& /dev/nul

echo "Installing eSDK components..."

# Install the documentation and examples
cp -Rd docs ${ESDK}/docs/
cp -Rd examples ${ESDK}/examples/


pushd ${ESDK_LIBS} >& /dev/nul

# Install the current BSP
cp -Rd bsps/${BSP} ${ESDK}/bsps/
ln -sTf ${BSP} ${ESDK}/bsps/bsp

# Install the XML parser library
cd src/e-xml
cp -f Release/libe-xml.so ${ESDK}/tools/host/${ARCH}/lib
cd ../../

# Install the Epiphnay HAL library
cd src/e-hal
ln -sTf ../../../../bsps/bsp/libe-hal.so ${HOST}/lib/libe-hal.so
cp -f src/epiphany-hal.h ${HOST}/include
cp -f src/epiphany-hal-defs.h ${HOST}/include
cp -f src/epiphany-hal-data.h ${HOST}/include
cp -f src/epiphany-hal-api.h ${HOST}/include
ln -sTf epiphany-hal.h ${HOST}/include/e-hal.h
ln -sTf epiphany-hal.h ${HOST}/include/e_hal.h
cd ../../

# Install the Epiphnay Loader library
cd src/e-loader
cp -f Release/libe-loader.so ${HOST}/lib
cp -f Release/libe-loader.a ${HOST}/lib
cp -f src/e-loader.h ${HOST}/include
ln -sTf e-loader.h ${HOST}/include/e_loader.h
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
cp include/*.h ${ESDK}/tools/e-gnu/epiphany-elf/sys-include/
cd ../../

popd >& /dev/nul


