#!/bin/sh

# Copyright (C) 2012, 2013, 2014 Embecosm Limited
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

#	       BUILD A SYMBOLICALLY LINKED UNIFIED SOURCE TREE
#	       ===============================================

# A wrapper for the GCC symlink-tree script, to build a symlink tree of all
# the component directories.

# Usage:

#   symlink-all.sh <rootdir> <unisrc> <component1> <component2> ...

# The <unisrc> directory is assumed to exist. symlink-tree is run in that
# directory for each top level directory in each <componentn> directory.

# We assume that the component directories are direct subdirs of <rootdir>. We
# also assume that the symlink-tree script can be found in <rootdir>/gcc


# Set the root directory
rootdir=$1
shift

# Change to the unisrc directory
unisrc=$1
shift
cd "${rootdir}/${unisrc}"

echo "Symlink-all" ${unisrc}

# Symlink each tree
for component in $*
do
    case $component in
	binutils)
	    ignore="gdb sim"
	    ;;

	*)
	    ignore=""
	    ;;
    esac

    if ! ${rootdir}/gcc/symlink-tree ${rootdir}/${component} "${ignore}"
    then
	exit 1
    fi
done

exit 0
