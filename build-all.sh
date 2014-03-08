#!/bin/sh

# Copyright (C) 2013 Embecosm Limited
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

#	       SCRIPT TO BUILD THE ADAPTEVA EPIPHANY TOOL CHAIN
#	       ================================================

# Invocation Syntax

#     build-all.sh [--build-dir <build_dir>]
#                  [--install-dir <install_dir>]
#                  [--symlink-dir <symlink_dir>]
#                  [--auto-pull | --no-auto-pull]
#                  [--auto-checkout | --no-auto-checkout]
#                  [--unisrc | --no-unisrc]
#                  [--datestamp-install]
#                  [--clean | --no-clean]
#                  [--enable-cgen-maint]
#                  [--jobs <count>] [--load <load>] [--single-thread]

# This script builds the Epiphany tool chain as held in git. It is assumed to be
# run from the toolchain directory (i.e. with binutils, cgen, gcc, newlib and
# gdb as peer directories).

# --build-dir <build_dir>

#     The directory in which the tool chain should be built. Default
#     ${topdir}/bd-${RELEASE}.

# --install-dir <install_dir>

#     The directory in which the tool chain should be installed. Default
#     /opt/adapteva/esdk/tools/e-gnu.

# --symlink-dir <symlink_dir>

#     If specified, the install directory will be symbolically linked to this
#     directory. Default not specified.

#     For example it may prove useful to install in a directory named with the
#     date and time when the tools were built, and then symbolically link to a
#     directory with a fixed name. By using the symbolic link in the users
#     PATH, the latest version of the tool chain will be used, while older
#     versions of the tool chains remain available under the dated
#     directories.

# --auto-checkout | --no-auto-checkout

#     If specified, a "git checkout" will be done in each component repository
#     to ensure the correct branch is checked out. Default is to checkout.

# --auto-pull | --no-auto-pull

#     If specified, a "git pull" will be done in each component repository
#     after checkout to ensure the latest code is in use. Default is to pull.

# --unisrc | --no-unisrc

#     If --unisrc is specified, rebuild the unified source tree. If
#     --no-unisrc is specified, do not rebuild it. The default is --unisrc.

# --datestamp-install

#     If specified, this will append a date and timestamp to the install
#     directory name. (see the comments under --symlink-dir above for reasons
#     why this might be useful).

# --clean | --no-clean

#     If specified, clean the build directory

# --enable-cgen-maint

#     If specified regenerate the CGEN related files. For developers only.

# --jobs <count>

#     Specify that parallel make should run at most <count> jobs. The default
#     is <count> equal to one more than the number of processor cores shown by
#     /proc/cpuinfo.

# --load <load>

#     Specify that parallel make should not start a new job if the load
#     average exceed <load>. The default is <load> equal to one more than the
#     number of processor cores shown by /proc/cpuinfo.

# --single-thread

#     Equivalent to --jobs 1 --load 1000. Only run one job at a time, but run
#     whatever the load average.

# Where directories are specified as arguments, they are relative to the
# current directory, unless specified as absolute names.

#------------------------------------------------------------------------------
#
#			       Shell functions
#
#------------------------------------------------------------------------------

# Determine the absolute path name. This should work for Linux, Cygwin and
# MinGW.
abspath ()
{
    sysname=`uname -o`
    case ${sysname} in

	Cygwin*)
	    # Cygwin
	    if echo $1 | grep -q -e "^[A-Za-z]:"
	    then
		echo $1		# Absolute directory
	    else
		echo `pwd`\\$1	# Relative directory
	    fi
	    ;;

	Msys*)
	    # MingGW
	    if echo $1 | grep -q -e "^[A-Za-z]:"
	    then
		echo $1		# Absolute directory
	    else
		echo `pwd`\\$1	# Relative directory
	    fi
	    ;;

	*)
	    # Assume GNU/Linux!
	    if echo $1 | grep -q -e "^/"
	    then
		echo $1		# Absolute directory
	    else
		echo `pwd`/$1	# Relative directory
	    fi
	    ;;
    esac
}


# Print a header to the log file and console

# @param[in] $1 String to use for header
header () {
    str=$1
    len=`expr length "${str}"`

    # Log file header
    echo ${str} >> ${logfile} 2>&1
    for i in $(seq ${len})
    do
	echo -n "=" >> ${logfile} 2>&1
    done
    echo "" >> ${logfile} 2>&1

    # Console output
    echo "${str} ..."
}


# Print a message to the log file and console

# @param[in] $1 String to use for the message
logmess () {
    str=$1
    len=`expr length "${str}"`

    # Log file message
    echo ${str} >> ${logfile} 2>&1
    # Console output
    echo "${str}"
}


# Displays a message if the build fails

# @param[in] $1 String to use for the error message
failedbuild() {
    logmess "$1"
    echo "- see ${logfile}"
    echo "********************************************************"
    echo "*  For support, visit the forums or documentation at:  *"
    echo "*   - http://forums.parallella.org/viewforum.php?f=13  *"
    echo "*   - https://github.com/adapteva/epiphany-sdk/wiki    *"
    echo "********************************************************"
}


#------------------------------------------------------------------------------
#
#		     Argument handling and initialization
#
#------------------------------------------------------------------------------

# Set the top level directory.
topdir=`(cd .. && pwd)`

# Generic release set up. This defines (and exports RELEASE, LOGDIR and
# RESDIR, creating directories named $LOGDIR and $RESDIR if they don't exist.
. "${topdir}"/sdk/define-release.sh

# Set defaults for some options
installdir="/opt/adapteva/esdk/tools/e-gnu"
builddir="${topdir}/bd-${RELEASE}"
unisrc="unisrc-${RELEASE}"
do_unisrc="--unisrc"
symlinkdir=""
datestamp=""
autocheckout="--auto-checkout"
autopull="--auto-pull"
doclean="--no-clean"
config_extra_opts=""

# Parse options
until
opt=$1
case ${opt} in
    --build-dir)
	shift
	builddir=`abspath $1`
	;;

    --install-dir)
	shift
	installdir=`abspath $1`
	;;

    --symlink-dir)
	shift
	symlinkdir=`abspath $1`
	;;

    --auto-checkout | --no-auto-checkout)
	autocheckout=$1
	;;

    --auto-pull | --no-auto-pull)
	autopull=$1
	;;

    --unisrc | --no-unisrc)
	do_unisrc=$1
	;;

    --datestamp-install)
	datestamp=-`date -u +%F-%H%M`
	;;

    --clean | --no-clean)
	doclean=$1
	;;

    --enable-cgen-maint)
	config_extra_opts="--enable-cgen-maint"
	;;

    --jobs)
	shift
	jobs=$1
	;;

    --load)
	shift
	load=$1
	;;

    --single-thread)
	jobs=1
	load=1000
	;;

    ?*)
	echo "Unknown argument $1"
	echo
	echo "Usage: ./build-all.sh [--build-dir <build_dir>]"
	echo "                      [--install-dir <install_dir>]"
	echo "                      [--symlink-dir <symlink_dir>]"
	echo "                      [--auto-checkout | --no-auto-checkout]"
        echo "                      [--auto-pull | --no-auto-pull]"
        echo "                      [--unisrc | --no-unisrc]"
	echo "                      [--datestamp-install]"
	echo "                      [--clean | --no-clean]"
	echo "                      [--enable-cgen-maint]"
        echo "                      [--jobs <count>] [--load <load>]"
        echo "                      [--single-thread]"
	exit 1
	;;

    *)
	;;
esac
[ "x${opt}" = "x" ]
do
    shift
done

# Set up a logfile
logfile="${LOGDIR}/all-build-$(date -u +%F-%H%M).log"
rm -f "${logfile}"

# Create and clean the build directory as required
if [ "x${doclean}" = "x--clean" ]
then
    header "Cleaning build directory"
    rm -rf ${builddir}
fi

mkdir -p ${builddir}

# Add a datestamp to the install directory if necessary. But if we are using
# --no-clean, we should reuse the installdir from the existing configuration.
if [ \( "x${doclean}" = "x--no-clean" \) -a \
     \( -e ${builddir}/config.log \) ]
then
    cd "${builddir}"
    datestamp=""
    installdir=`sed -n -e 's/.*--prefix=\([^ ]*\).*/\1/p' config.log | head -1`
fi

if [ "x${datestamp}" != "x" ]
then
    installdir="${installdir}-$datestamp"
fi

parallel="-j ${jobs} -l ${load}"

# Make sure we stop if something failed.
trap "echo ERROR: Failed due to signal ; date ; exit 1" \
    HUP INT QUIT SYS PIPE TERM

# Exit immediately if a command exits with a non-zero status (but note this is
# not effective if the result of the command is being tested for, so we can
# still have custom error handling).
set -e

header "START BUILD"
logmess "- started at $(date)"
logmess "- log: ${logfile}"
logmess "- build directory: ${builddir}"
logmess "- install directory: ${installdir}"
logmess "- unified source directory: ${unisrc}"

# Checkout the correct branch for each tool
header "Checking out GIT trees"
if ! ${topdir}/sdk/get-versions.sh ${topdir} ${autocheckout} \
         ${autopull} >> "${logfile}" 2>&1
then
    logmess "ERROR: Failed to checkout GIT versions of tools"
    echo "- see ${logfile}"
    exit 1
fi

# Make a unified source tree in the build directory. Note that later versions
# override earlier versions with the current symlinking version, with the
# exception of the gdb and sim directories in binutils, which do not override
# the versions from gdb.
if [ "x${do_unisrc}" = "x--unisrc" ]
then
    header "Linking unified tree"

    # Change to the root directory
    cd "${topdir}"

    component_dirs="cgen newlib gdb binutils gcc"
    rm -rf ${unisrc}

    if ! mkdir -p ${unisrc}
    then
	failedbuild "ERROR: Failed to create ${unisrc}"
	exit 1
    fi

    if ! ${topdir}/sdk/symlink-all.sh ${topdir} ${unisrc} \
	"${component_dirs}" >> "${logfile}" 2>&1
    then
	failedbuild "ERROR: Failed to symlink ${unisrc}"
	exit 1
    fi
fi

# We'll need the tool chain on the path.
PATH=${installdir}/bin:$PATH
export PATH

# Configure binutils, GCC, newlib and GDB
header "Configuring tools"
cd "${builddir}"
if "${topdir}/${unisrc}/configure" --target=epiphany-elf \
    --with-pkgversion="Epiphany toolchain (built `date +%Y%m%d`)" \
    --with-bugurl=support-sdk@adapteva.com \
    --enable-fast-install=N/A \
    --enable-languages=c,c++ --prefix="${installdir}" \
    --with-headers="$(abspath {topdir}/newlib/newlib/libc/include)" \
    --disable-gdbtk --disable-werror \
    ${config_extra_opts} >> "${logfile}" 2>&1
then
    logmess "- configure succeeded"
else
    failedbuild "ERROR: Configure failed"
    exit 1
fi

# Build binutils, GCC, newlib and GDB
header "Building tools"
if make $make_load all-build all-binutils all-gas all-ld all-gcc \
                   all-target-libgcc all-target-libgloss all-target-newlib \
                   all-target-libstdc++-v3 all-gdb all-sim >> "${logfile}" 2>&1
then
    logmess "- build succeeded"
else
  failedbuild "ERROR: Build failed"
  exit 1
fi

# Install binutils, GCC, newlib and GDB
header "Installing tools"
if make install-binutils install-gas install-ld install-gcc \
        install-target-libgcc install-target-libgloss install-target-newlib \
        install-target-libstdc++-v3 install-gdb install-sim >> "${logfile}" 2>&1
then
    logmess "- build succeeded"
else
    failedbuild "ERROR: Install failed"
    exit 1
fi


# Create symbolic links in install directory for e-gcc, etc.
header "Creating e-* symbolic links"
cd "${installdir}/bin"
for i in epiphany-elf-*
do
    ename=$(echo ${i} | sed 's/epiphany-elf-/e-/')
    if [ ! -e ${ename} ]
    then
      ln -s ${i} ${ename}
    fi
done

cd "${installdir}/share/man/man1"
for i in epiphany-elf-*
do
  ename=$(echo ${i} | sed 's/epiphany-elf-/e-/')
  if [ ! -e ${ename} ]
  then
      ln -s $i ${ename}
  fi
done

# Optionally add a symbolic link to the install directory. Note the
# introductory comments about the need to specify explicitly the install
# directory. Note also that ln -sf does not do what you think it might!
if [ "x${symlinkdir}" != "x" ]
then
    header "Symbolically linking ${installdir} to ${symlinkdir}"
    rm -f ${symlinkdir}
    ln -s ${installdir} ${symlinkdir}
fi

header "BUILD COMPLETE:"
logmess "- complete at $(date)"
logmess "- installed at {installdir}/bin"
echo "- see ${logfile} for full details"
echo "********************************************************"
echo "*  Please ensure the install directory is in your PATH *"
echo "*                                                      *"
echo "*  For support, visit the forums or documentation at:  *"
echo "*   - http://forums.parallella.org/viewforum.php?f=13  *"
echo "*   - https://github.com/adapteva/epiphany-sdk/wiki    *"
echo "********************************************************"
