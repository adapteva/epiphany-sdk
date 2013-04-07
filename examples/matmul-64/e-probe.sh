#!/bin/bash

set -e

ELIBS="${EPIPHANY_HOME}/tools/host/lib"
EHDF="${EPIPHANY_HOME}/bsps/zed_E64G4_512mb/zed_E64G4_512mb.hdf"

sudo -E LD_LIBRARY_PATH=${ELIBS} EPIPHANY_HDF=${EHDF} host/Release/e-probe.elf $@

