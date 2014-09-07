#!/bin/sh

# Script to set RPATHs to be relative in shared binaries

# Copyright (C) 2012, 2013 Synopsys Inc.
# Copyright (C) 2014 Embecosm Limited.

# Contributor Simon Cook <simon.cook@embecosm.com>

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
# This should be run in the INSTALL directory, so there should be a bin
# subdirectory.

# Usage:

#     ./rel-paths.sh <installdir>

# Exits with 0 on success, 1 otherwise.

install_dir=$1

if ! cd ${install_dir}
then
    echo "ERROR: rel-rpaths.sh: Can't cd to install directory ${install_dir}"
    exit 1
fi

if ! [ -d bin ]; then
    echo "\`$INSTALLDIR' is not a toolchain installation directory."
    exit 1
fi

# We need patchelf for this to install
if ! which patchelf > /dev/null 2>&1
then
    echo "ERROR: rel-rpaths.sh: patchelf not found."
    exit 1
fi

# Get list of executables
files=`find -type f -exec file {} \; | grep 'ELF ??-bit .* executable' | \
           sed -e 's/:.*$//'`

for f in ${files}
do
    # Do we have a RPATH to do?
    rpath=$(readelf -d "${f}" | grep 'Library rpath')
    if [ $? -gt 0 ]; then
	continue
    fi

    echo "Making RPATH relative for $f"
    # Build a relative directory
    reldir=${f:2}
    reldir=$(echo ${reldir//[^\/]})
    reldir=$(echo ${reldir//\//\/..})

    rpath=`echo "${rpath}" | \
	       sed "s#.*\[${install_dir}\(.*\)\]#\$ORIGIN${reldir}\1#"`
    patchelf --set-rpath "${RPATH}" ${f}
done
