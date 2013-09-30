#!/bin/sh -f

#  Copyright (c) 2010, All Right Reserved, Adapteva, Inc.

#  This source is subject to the Adapteva Software License.
#  You should have received a copy of the full license file
#  named ("LICENSE") along with this program.  If not, contact
#  Adapteva immediately at support@adapteva.com

if [ -z "$EPIPHANY_HOME" ]; then
	echo "Please set the EPIPHANY_HOME to the root folder of the SDK release!"
else
	#PATH=${EPIPHANY_HOME}/tools/a-gnu/bin:${PATH}
	PATH=${EPIPHANY_HOME}/tools/e-gnu/bin:${PATH}
	PATH=${EPIPHANY_HOME}/tools/host/bin:${PATH}
	LD_LIBRARY_PATH=${EPIPHANY_HOME}/tools/host/lib:${LD_LIBRARY_PATH}
	EPIPHANY_HDF=${EPIPHANY_HOME}/bsps/current/platform.hdf

	export PATH
	export LD_LIBRARY_PATH
	export EPIPHANY_HOME
	export EPIPHANY_HDF
fi

