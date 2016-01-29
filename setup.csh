#!/bin/csh 

#  Copyright (c) 2010, All Right Reserved, Adapteva, Inc.

#  This source is subject to the Adapteva Software License.
#  You should have received a copy of the full license file
#  named ("LICENSE") along with this program.  If not, contact
#  Adapteva immediately at support@adapteva.com

if($?EPIPHANY_HOME) then

else
	echo "Please set EPIPHANY_HOME to root folder of the SDK release!"
	exit 2
endif


switch ( `uname -m` )
	case x86_64:
		setenv _esdk_arch ".x86_64"
		breaksw
	case arm*:
		setenv _esdk_arch ".armv7l"
		breaksw
	default:
		setenv _esdk_arch ""
endsw


#setenv PATH ${EPIPHANY_HOME}/tools/a-gnu/bin:${PATH}
setenv PATH ${EPIPHANY_HOME}/tools/e-gnu${_esdk_arch}/bin:${PATH}
setenv PATH ${EPIPHANY_HOME}/tools/host${_esdk_arch}/bin:${PATH}
setenv EPIPHANY_HDF ${EPIPHANY_HOME}/bsps/current/platform.hdf

if ( $?MANPATH  ) then
    setenv MANPATH ${EPIPHANY_HOME}/tools/e-gnu${_esdk_arch}/share/man:${MANPATH}
else
    setenv MANPATH ${EPIPHANY_HOME}/tools/e-gnu${_esdk_arch}/share/man
endif

if ( $?LD_LIBRARY_PATH  ) then
	setenv LD_LIBRARY_PATH ${EPIPHANY_HOME}/tools/host${_esdk_arch}/lib:${LD_LIBRARY_PATH}
	setenv LD_LIBRARY_PATH ${EPIPHANY_HOME}/tools/e-gnu${_esdk_arch}/lib:${LD_LIBRARY_PATH}
else
	setenv LD_LIBRARY_PATH ${EPIPHANY_HOME}/tools/host${_esdk_arch}/lib
	setenv LD_LIBRARY_PATH ${EPIPHANY_HOME}/tools/e-gnu${_esdk_arch}/lib:${LD_LIBRARY_PATH}
endif

unset _esdk_arch
