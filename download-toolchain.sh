#!/bin/sh

# Copyright (C) 2013 Embecosm Limited

# Contributor Simon Cook <simon.cook@embecosm.com>

# This file is a script to download toolchain source.

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

# Function to either download a tool or clone a git repository, checking out
# the relevant branch.
download_tool() {
    tool=$1
    user=$2
    repo=$3
    branch=$4

    if [ ${clone} = "true" ]
    then
	# If old source exists, delete
	if [ ${force} = "true" ]
	then
	    if ! rm -rf ${tool} >> ${log} 2>&1
	    then
		echo "Unable to delete old ${tool}" | tee -a ${log}
		return 0
	    fi	
	fi

	# Clone git repository if it does not already exist
	if [ -e ${tool} ]
	then
	    echo "Skipping ${tool} already exists..." | tee -a ${log}
	else
	    echo "Cloning ${tool}..."
	    if ! git clone -q -b ${branch} https://github.com/${user}/${repo} ${tool} \
	        >> ${log} 2>&1
	    then
		echo "Unable to clone ${tool}" | tee -a ${log}
		return 1
	    fi
	fi

    else
	# If old source exists, delete
	if [ ${force} = "true" ]
	then
	    if ! rm -rf ${tool} ${branch}.zip >> ${log} 2>&1
	    then
		echo "Unable to delete old ${tool}" | tee -a ${log}
		return 0
	    fi	
	fi

    	# Download and unzip source if it does not already exist
	if [ -e ${tool} ]
	then
	    echo "Skipping ${tool}, already exists..." | tee -a ${log}
	else
	    echo "Downloading ${tool}..."
	    if ! wget https://github.com/${user}/${repo}/archive/${branch}.zip \
		>> ${log} 2>&1
	    then
		echo "Unable to download ${tool}" | tee -a ${log}
		return 1
	    fi
	    if ! unzip ${branch}.zip >> ${log} 2>&1
	    then
		echo "Unable to unzip ${tool}" | tee -a ${log}
		return 1
	    fi
	    if ! mv ${repo}-${branch} ${tool} >> ${log} 2>&1
	    then
		echo "Unable to move ${tool}" | tee -a ${log}
		return 1
	    fi
	    if ! rm ${branch}.zip >> ${log} 2>&1
	    then
		echo "Unable to remove zip file for ${tool}" | tee -a ${log}
		return 1
	    fi
	fi
    fi
}

# Check for relative directory and makes it absolute
absolutedir() {
  case ${1} in
    /*) echo "${1}" ;;
    *)  echo "${PWD}/${1}";;
  esac
}

# Parse arguments
force="false"
clone="false"
checkout="${PWD}"
until
opt=$1
case ${opt} in
    --force)
	force="true" ;;
    --clone)
	clone="true" ;;
    --location)
	checkout=$(absolutedir "$2"); shift ;;
    ?*)
	echo "Usage: $0 [--force] [--clone] [--location <source location>]"
	exit 1 ;;
    *)
	;;
esac
[ "x${opt}" = "x" ]
do
    shift
done

# Create and move to checkout location
mkdir -p "${checkout}"
cd "${checkout}"

# Set up a log file
log="${PWD}/clone-$(date -u +%F-%H%M).log"
rm -f "${log}"

# Clone repositories
download_tool gcc           adapteva epiphany-gcc      epiphany-gcc-4.8
download_tool binutils      adapteva epiphany-binutils epiphany-binutils-2.23
download_tool gdb           adapteva epiphany-gdb      epiphany-gdb-7.6
download_tool newlib        adapteva epiphany-newlib   epiphany-newlib-1.20
download_tool cgen          adapteva epiphany-cgen     epiphany-cgen-1.1

# BSC - we no longer checkout the SDK or LIBS here
#download_tool sdk           adapteva epiphany-sdk      master
#download_tool epiphany-libs adapteva epiphany-libs     master

echo "Download complete"
