#!/bin/bash

set -e

if [[ "$1" == "-d" ]]; then
	Config='Debug'
else
	Config='Release'
fi

ELIBS="${EPIPHANY_HOME}/tools/host/lib"
EHDF=${EPIPHANY_HDF}

cd host/${Config}

sudo -E LD_LIBRARY_PATH=${ELIBS} EPIPHANY_HDF=${EHDF} ./matmul.elf $@ ../../device/${Config}/e_matmul.srec

cd ../../

