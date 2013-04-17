#!/bin/bash

set -e

if [[ "`arch`" == "x86_64" ]]; then
	echo "Setting path to ARM tools."
	GNU_PATH=${HOME}'/CodeSourcery/Sourcery_CodeBench_Lite_for_ARM_GNU_Linux/bin/arm-none-linux-gnueabi-'
else
	GNU_PATH=''
fi

ESDK=${EPIPHANY_HOME}
ELIBS=${ESDK}/tools/host/lib
EINCS=${ESDK}/tools/host/include
ELDF=${ESDK}/bsps/current/internal.ldf

# Build HOST side application
${GNU_PATH}gcc int-test.c -o int-test.elf -I ${EINCS} -L ${ELIBS} -le-hal

# Build DEVICE side program
e-gcc -O0 -T ${ELDF} e-int-test.master.c -o e-int-test.master.elf -le-lib
e-gcc -O0 -T ${ELDF} e-int-test.slave.c  -o e-int-test.slave.elf  -le-lib

# Convert ebinary to SREC file
e-objcopy --srec-forceS3 --output-target srec e-int-test.master.elf e-int-test.master.srec
e-objcopy --srec-forceS3 --output-target srec e-int-test.slave.elf  e-int-test.slave.srec

