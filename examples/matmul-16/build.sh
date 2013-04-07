#!/bin/bash

set -e

if [[ "`arch`" == "x86_64" ]]; then
	echo "Setting path to ARM tools."
	GNU_PATH=${HOME}'/CodeSourcery/Sourcery_CodeBench_Lite_for_ARM_GNU_Linux/bin/arm-none-linux-gnueabi-'
else
	GNU_PATH=''
fi

Proj='matmul-16'
if [[ "$1" == "-d" ]]; then
	Config='Debug'
else
	Config='Release'
fi
BUILD_DEVICE='yes'
BUILD_HOST='yes'

MK_CLEAN='yes'
MK_ALL='yes'

minRow=32
maxRow=35
minCol=8
maxCol=11

if [[ "${BUILD_DEVICE}" == "yes" ]]; then
	echo "=== Building device programs ==="

	for f in {commonlib/${Config},device/${Config}}; do
		pushd $f >& /dev/null
		if [[ "${MK_CLEAN}" == "yes" ]]; then
			echo "*** Cleaning $f"
			make clean
		fi
		if [[ "${MK_ALL}" == "yes" ]]; then
			echo "*** Building $f"
			make --warn-undefined-variables BuildConfig=${Config} all
		fi
		popd >& /dev/null
	done

	echo "*** Creating srec file"
	rm -rf device/${Config}/e_matmul.srec
	e-objcopy --srec-forceS3 --output-target srec device/${Config}/e_matmul.elf device/${Config}/e_matmul.cores.srec
	for ((Row=${minRow}; Row<=${maxRow}; Row++)); do
		for ((Col=${minCol}; Col<=${maxCol}; Col++)); do
			ThisCore=$(( $Row * 64 + $Col ))
			printf -v CoreID  '%3x' $ThisCore
		done
	done
	mv device/${Config}/e_matmul.cores.srec device/${Config}/e_matmul.srec
	chmod -x device/${Config}/*.srec
fi


EINCS="${EPIPHANY_HOME}/tools/host/include"
ELIBS="${EPIPHANY_HOME}/tools/host/lib"

if [[ "${BUILD_HOST}" == "yes" ]]; then
	echo "=== Building host programs ==="

	mkdir -p ./host/${Config}

	if [[ "${MK_CLEAN}" == "yes" ]]; then
		echo "*** Cleaning host programs"
		rm -f ./host/${Config}/matmul.elf
		rm -f ./host/${Config}/e-probe.elf
	fi

	if [[ "${MK_ALL}" == "yes" ]]; then
		echo "*** Building host program"
		${GNU_PATH}gcc ${STATIC_LINK} \
			-Ofast \
			-Wall \
			-g0 \
			-ffast-math \
			-falign-loops=8 \
			-funroll-loops \
			-D__HOST__ \
			-Dasm=__asm__ \
			-Drestrict= \
			-I./commonlib/src \
			-I${EINCS} \
			-L${ELIBS} \
			-o "./host/${Config}/matmul.elf" \
			"./commonlib/src/matlib.c" \
			"./host/src/matmul_host.c" \
			-le-hal

		echo "*** Building probe program"

		${GNU_PATH}gcc ${STATIC_LINK} \
			-Ofast \
			-Wall \
			-g0 \
			-D__HOST__ \
			-Dasm=__asm__ \
			-Drestrict= \
			-I./commonlib/src \
			-I${EINCS} \
			-L${ELIBS} \
			-o "./host/${Config}/e-probe.elf" \
			"./host/src/e-probe.c" \
			-le-hal
	fi
fi

exit
