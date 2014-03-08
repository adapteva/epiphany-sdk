#!/bin/sh

# Copyright (C) 2013, 2014 Embecosm Inc.

# Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

# This file is part of the Embecosm build system for Epiphany

# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 3 of the License, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.

# You should have received a copy of the GNU General Public License along
# with this program.  If not, see <http://www.gnu.org/licenses/>.          

#		 SCRIPT TO GET CORRECT GIT VERSIONS OF TOOLS
#                ===========================================
# -----------------------------------------------------------------------------
# Usage:

#     get-versions.sh <rootdir>
#                     [--auto-checkout | --no-auto-checkout]
#                     [--auto-pull | --no-auto-pull]

# We checkout the desired branch for each tool. Note that these must exist or
# we fail.

# Set the top level directory
topdir=$1
shift

if [ "x${topdir}" = "x" ]
then
    echo "get-versions.sh: No top level directory specified."
    exit 1
fi

if ! cd ${topdir}
then
    echo "get-versions.sh: Cannot access top level directory."
    exit 1
fi

# Default options
autocheckout="--auto-checkout"
autopull="--auto-pull"

# Parse options
until
opt=$1
case ${opt} in
    --auto-checkout | --no-auto-checkout)
	autocheckout=$1
	;;

    --auto-pull | --no-auto-pull)
	autopull=$1
	;;

    ?*)
	echo "Usage: get-versions.sh  <topdir>"
        echo "                        [--auto-checkout | --no-auto-checkout]"
        echo "                        [--auto-pull | --no-auto-pull]"
	exit 1
	;;

    *)
	;;
esac
[ "x${opt}" = "x" ]
do
    shift
done

# Specify the default versions to use as a string <tool>:<branch>. Only
# actually matters if --auto-checkout is set.
binutils="binutils:epiphany-binutils-2.23"
cgen="cgen:epiphany-cgen-1.1"
gcc="gcc:epiphany-gcc-4.8"
gdb="gdb:jeremy-update"
newlib="newlib:epiphany-newlib-1.20"

for version in ${binutils} ${cgen} ${gcc} ${gdb} ${newlib}
do
    tool=`echo ${version} | cut -d ':' -f 1`
    branch=`echo ${version} | cut -d ':' -f 2`

    cd ${topdir}/${tool}

    if [ "x${autocheckout}" = "x--auto-checkout" ]
    then
	if ! git checkout ${branch}
	then
	    exit 1
	fi
    fi

    if [ "x${autopull}" = "x--auto-pull" ]
    then
	if ! git pull
	then
	    exit 1
	fi
    fi
done
