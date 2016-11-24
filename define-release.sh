#!/bin/sh

# Copyright (C) 2013,2014 Embecosm Limited

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

# Usage:

#		SCRIPT TO DEFINE RELEASE SPECIFIC INFORMATION
#               =============================================

# Script must be sourced, since it sets up environment variables for the
# parent script.

# Defines the RELEASE, LOGDIR and RESDIR environment variables, creating the
# LOGDIR and RESDIR directories if they don't exist.

# The following pre-requisites must be defined

# basedir

#     The directory containing all the sources. Log and results files are
#     created within this directory.

# Epiphany releases have the naming convention:

#   <year>.<month>[.<patch>]
RELEASE_MAJOR="2016"
RELEASE_MINOR="11"
RELEASE_PATCH=""

# The full release version
RELEASE="${RELEASE_MAJOR}.${RELEASE_MINOR}${RELEASE_PATCH}"
#RELEASE="master"

# Create a common log directory for all logs in this and sub-scripts
LOGDIR=${basedir}/logs/${RELEASE}
mkdir -p ${LOGDIR}

# Create a common results directory in which sub-directories will be created
# for each set of tests.
RESDIR=${basedir}/results/${RELEASE}
mkdir -p ${RESDIR}

# Export the environment variables
export RELEASE
export LOGDIR
export RESDIR
