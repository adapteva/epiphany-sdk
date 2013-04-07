#!/bin/bash

set -e

if [[ "$1" == "-d" ]]; then
	Config='Debug'
else
	Config='Release'
fi

ELIBS="${EPIPHANY_HOME}/tools/host/lib"
EHDF="${EPIPHANY_HOME}/bsps/zed_E16G3_512mb/zed_E16G3_512mb.hdf"

cd host/${Config}

sudo -E LD_LIBRARY_PATH=${ELIBS} EPIPHANY_HDF=${EHDF} ./matmul.elf $@ ../../device/${Config}/e_matmul.srec

cd ../../

