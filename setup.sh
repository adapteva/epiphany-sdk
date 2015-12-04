#!/bin/sh -f

#  Copyright (c) 2010, All Right Reserved, Adapteva, Inc.

#  This source is subject to the Adapteva Software License.
#  You should have received a copy of the full license file
#  named ("LICENSE") along with this program.  If not, contact
#  Adapteva immediately at support@adapteva.com

case $(uname -m) in
	x86_64)
		_esdk_arch=".x86_64"
		;;
	arm*)
		_esdk_arch=".armv7l"
		;;
	*)
		_esdk_arch=""
		;;
esac

if [ -z "$EPIPHANY_HOME" ]; then
	echo "Please set the EPIPHANY_HOME to the root folder of the SDK release!"
else
	#PATH=${EPIPHANY_HOME}/tools/a-gnu/bin:${PATH}
	PATH=${EPIPHANY_HOME}/tools/e-gnu${_esdk_arch}/bin:${PATH}
	PATH=${EPIPHANY_HOME}/tools/host${_esdk_arch}/bin:${PATH}
	LD_LIBRARY_PATH=${EPIPHANY_HOME}/tools/host${_esdk_arch}/lib:${LD_LIBRARY_PATH}
	LD_LIBRARY_PATH=${EPIPHANY_HOME}/tools/e-gnu${_esdk_arch}/lib:${LD_LIBRARY_PATH}
	EPIPHANY_HDF=${EPIPHANY_HOME}/bsps/current/platform.hdf
	MANPATH=${EPIPHANY_HOME}/tools/e-gnu${_esdk_arch}/share/man:${MANPATH}

	export PATH
	export LD_LIBRARY_PATH
	export EPIPHANY_HOME
	export EPIPHANY_HDF
	export MANPATH
fi

unset _esdk_arch
