#!/bin/sh

# Copyright (C) 2009 - 2014 Embecosm Limited

# Contributor Joern Rennecke <joern.rennecke@embecosm.com>
# Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>
# Contributor Simon Cook <simon.cook@embecosm.com>

# This file is a script to build the Epiphany tool chain

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

# This is a convenience wrapper for the GNU toolchain build files.

# Invocation:

#     ./build-toolchain.sh [--build-dir <dir>]
#                          [--install-dir <dir> ]
#                          [--symlink-dir <dir>]
#                          [--datestamp-install]
#                          [--clean | --no-clean]
#                          |--preserve-unisrc | --rebuild-unisrc]
#                          [--unified-dir <dir>]
#                          [--auto-pull | --no-auto-pull]
#                          [--auto-checkout | --no-auto-checkout]
#                          [--jobs <count>] [--load <load>] [--single-thread]
#                          [--gmp | --no-gmp]
#                          [--mpfr | --no-mpfr]
#                          [--mpc | --no-mpc]
#                          [--isl | --no-isl]
#                          [--cloog | --no-cloog]
#                          [--rel-rpaths | --no-rel-rpaths]
#                          [--target-cflags <flags>]
#                          [--config-extra <flags>]
#                          [--disable-werror | --enable-werror]
#                          [--enable-cgen-maint]
#                          [--help | -h]

# A number of arguments specify directories relative to the base directory.
# This is the parent directory of the directory containing this script.  Other
# arguments use the release name, which is set up in the environment variable
# RELEASE.  The meanings of the various options and their defaults are as
# follows.

# --build-dir <build_dir>

#     The directory in which the build directory will be created.  The default
#     is  builds/bd-epiphany-$RELEASE in the base directory.

# --install-dir <install_dir>

#     The directory in which the tool chain should be installed. If not
#     specified, it will be installed in the INSTALL sub-directory of the
#     base directory.

# --symlink-dir <symlink_dir>

#     If specified, the install directory will be symbolically linked to this
#     directory.

#     For example it may prove useful to install in a directory named with the
#     date and time when the tools were built, and then symbolically link to a
#     directory with a fixed name. By using the symbolic link in the users
#     PATH, the latest version of the tool chain will be used, while older
#     versions of the tool chains remain available under the dated
#     directories.

# --datestamp-install

#     If specified, this will append a date and timestamp to the install
#     directory name.  See the comments under --symlink-dir above for reasons
#     why this might be useful.

# --clean | --no-clean

#     If --clean is specified, delete any previous build directory. Default
#     --no-clean.

# --auto-pull | --no-auto-pull

#     If specified, a "git pull" will be attempted in each component
#     repository after checkout to ensure the latest code is in use.  Default
#     is --auto-pull.  Note that pulling will only be attempted in components
#     which are git repositories, so this may safely be used, even if some
#     component trees are not git repositories.

# --auto-checkout | --no-auto-checkout

#     If specified, a "git checkout" will be attempted in each component
#     repository to ensure the correct branch is checked out.  Default is
#     --auto-checkout.  Note that checkout will only be attempted in
#     components which are git repositories, so this may safely be used, even
#     if some component trees are not git repositories.

# --preserve-unisrc | --rebuild-unisrc

#     If --preserve-unisrc is specified, use an existing unified source tree
#     if it exists. If --rebuild-unisrc is specified blow away any existing
#     source tree and rebuild it.  Default --preserve-unisrc.

# --unified-dir <dir>

#     Specify the name of the unified directory.  The default is
#     unisrc-<release>, where release is the name of the release being built
#     (defined by the branch of this repository) in the base directory.

# --jobs <count>

#     Specify that parallel make should run at most <count> jobs. The default
#     is one more than the number of processor cores shown by /proc/cpuinfo.

# --load <load>

#     Specify that parallel make should not start a new job if the load
#     average exceed <load>. The default is one more than the number of
#     processor cores shown by /proc/cpuinfo.

# --single-thread

#     Equivalent to --jobs 1 --load 1000. Only run one job at a time, but run
#     whatever the load average.

# --gmp | --no-gmp
# --mpfr | --no-mpfr
# --mpc | --no-mpc
# --isl | --no-isl
# --cloog | --no-cloog

#     Indicate that the corresponding GCC infrastructure component exists as a
#     source directory within the base directory and should be linked into the
#     unified source directory.  With the "--no-" versions, indicate the
#     component is not available as a source directory, and that the compiler
#     should rely on the installed developer package for the relevant headers
#     and libraries.

#     Note that in this case it is the *developer* version of the package that
#     must be installed.  Specifiying --no-gmp, --no-mpfr or --no-mpc if the
#     corresponding developer package is not available will cause tool chain
#     building to fail. Specifying --no-isl or --no-cloog when the developer
#     package is not available will cause some GCC optimizatiosn to be
#     omitted.

#     Defaults --gmp --mpfr --mpc --isl --cloog.

# --rel-rpaths | --no-rel-rpaths

#     If --rel-rpaths is specified, the RPATHs of tools are set to be relative
#     to the INSTALL directory and thus become portable, something that is not
#     usually possible with GNU tool chains.  Note that this option requires
#     patchelf to be installed.  Default --rel-paths if patchelf is installed
#     and on the search path and --no-rel-paths otherwise.

# --target-cflags <flags>

#     Specify C flags to be used when building target libraries (libgcc.a,
#     libc.a and libm.a).  For example specifying -Os -g would cause libraries
#     to be built optimized for small size and suitable for debugging.

#     The default "-O2 -g" is because constant merging in the assembler/linker
#     does not work.

# --config-extra <flags>

#     This is an option for developers, allowing additional flags to be
#     specified when configuring the tool chain.

# --disable-werror | --enable-werror

#     This is an option for developers, allowing them to compile the tool
#     chain with the -Werror option.  This is generally good practice for such
#     users, to ensures any new code is as robust as possible.  However it is
#     not recommended for general use, since the wide variety and age of
#     compilers used mean that errors will be triggered, just due to historic
#     incompatibilities, but which have no impact on the quality of the
#     compiler.

# --enable-cgen-maint

#     This is an option for developers allowing the CGEN tables to be
#     rebuilt.  This is required after a change to the CGEN description, so
#     the new specifications are included in the assembler/disassembler and
#     simulator.

# --help | -h

#     Print out a short message about usage.

# The script returns 0 if the tool chain was successfully built, and 1
# otherwise.  Note that some errors (for example in RPATH) do not prevent a
# tool chain being built, so will generate warning messages in the log, but
# the script will still return 0.


################################################################################
#                                                                              #
#			       Shell functions                                 #
#                                                                              #
################################################################################

# Function to check for relative directory and makes it absolute

# @param[in] $1  The directory to make absolute if necessary.
absdir() {
    case ${1} in

	/*)
	    echo "${1}"
	    ;;

	*)
	    echo "${PWD}/${1}"
	    ;;
    esac
}


# Check a component directory exists in the basedir

# @param[in] $1 the directory to check

# @return  0 (success) if the directory is there and readable, 1 (failure)
#          otherwise.
check_dir_exists () {
    if [ ! -d "${basedir}/${1}" ]
    then
	logterm "ERROR: Component directory ${basedir}/${1} missing."
	return;
    fi
}


# Convenience function to copy a message to the log and terminal

# @param[in] $1  The message to log
logterm () {
    echo $1 | tee -a ${logfile}
}


# Convenience function to copy a message to the log only

# @param[in] $1  The message to log
logonly () {
    echo $1 >> ${logfile}
}


# Convenience function to exit with a suitable message.
failedbuild () {
  echo "Build failed. See ${logfile} for details."
  exit 1
}


################################################################################
#                                                                              #
#			       Parse arguments                                 #
#                                                                              #
################################################################################

# Define the basedir
d=`dirname "$0"`
basedir=`(cd "$d/.." && pwd)`

# Set the release parameters
. ${basedir}/sdk/define-release.sh

# Set up a clean log
logfile=${LOGDIR}/build-$(date -u +%F-%H%M).log
rm -f "${logfile}"
echo "Logging to ${logfile}"

# Defaults
build_dir=${basedir}/builds/bd-epiphany-${RELEASE}
install_dir=${basedir}/INSTALL
symlink_dir=
datestamp=
do_clean="--no-clean"
auto_pull="--auto-pull"
auto_checkout="--auto-checkout"
rebuild_unisrc="--preserve-unisrc"
unisrc_dir=${basedir}/unisrc-${RELEASE}
jobs=
load=
do_gmp="--gmp"
do_mpfr="--mpfr"
do_mpc="--mpc"
do_isl="--isl"
do_cloog="--cloog"

if [ `which patchelf` > /dev/null 2>&1 ]
then
    do_rel_rpaths="--rel-rpaths"
else
    do_rel_rpaths="--no-rel-rpaths"
fi

# The assembler and/or linker are broken so that constant merging doesn't
# work.
CFLAGS_FOR_TARGET="-O2 -g"
config_extra=""
disable_werror="--disable-werror"


until
opt=$1
case ${opt} in

    --build-dir)
	shift
	build_dir=`absdir "$1"`
	;;

    --install-dir)
	shift
	install_dir=`absdir "$1"`
	;;

    --symlink-dir)
	shift
	symlink_dir=`absdir "$1"`
	;;

    --datestamp-install)
	datestamp=`date -u +%F-%H%M`
	;;

    --clean)
	do_clean="$1"
	;;

    --no-clean)
	do_clean="$1"
	;;

    --preserve-unisrc | --rebuild-unisrc)
	rebuild_unisrc="$1"
	;;

    --unified-dir)
	shift
	unisrc_dir="$1"
	;;

    --auto-pull | --no-auto-pull)
	auto_pull="$1"
	;;

    --auto-checkout | --no-auto-checkout)
	auto_checkout="$1"
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

    --gmp | --no-gmp)
	do_gmp="$1"
	;;

    --mpfr | --no-mpfr)
	do_mpfr="$1"
	;;

    --mpc | --no-mpc)
	do_mpc="$1"
	;;

    --isl | --no-isl)
	do_isl="$1"
	;;

    --cloog | --no-cloog)
	do_cloog="$1"
	;;

    --rel-rpaths | --no-rel-rpaths)
	do_rel_rpaths="$1"
	;;

    --target-cflags)
	shift
	CFLAGS_FOR_TARGET="$1"
	;;

    --config-extra)
	shift
	config_extra="${config_extra} $1"
	;;

    --disable-werror | --enable-werror)
	config_extra="${config_extra} $1"
	;;

    --enable-cgen-maint)
	config_extra="${config_extra} --enable-cgen-maint"
	;;

    ?*)
        echo "Usage: ./build-toolchain.sh [--build-dir <dir>]"
        echo "             [--install-dir <dir> ]"
        echo "             [--symlink-dir <dir>]"
        echo "             [--datestamp-install]"
        echo "             |--preserve-unisrc | --rebuild-unisrc]"
        echo "             [--unified-dir <dir>]"
        echo "             [--auto-pull | --no-auto-pull]"
        echo "             [--auto-checkout | --no-auto-checkout]"
        echo "             [--jobs <count>] [--load <load>] [--single-thread]"
        echo "             [--gmp | --no-gmp]"
        echo "             [--mpfr | --no-mpfr]"
        echo "             [--mpc | --no-mpc]"
        echo "             [--isl | --no-isl]"
        echo "             [--cloog | --no-cloog]"
        echo "             [--rel-rpaths | --no-rel-rpaths]"
        echo "             [--target-cflags <flags>]"
        echo "             [--config-extra <flags>]"
        echo "             [--disable-werror | --enable-werror]"
        echo "             [--enable-cgen-maint]"
        echo "             [--help | -h]"
	exit 0
	;;

    *)
	;;
esac
[ "x${opt}" = "x" ]
do
    shift
done

# Add the datestamp to the install dir if set
if [ "x$datestamp" != "x" ]
then
    install_dir="${install_dir}-${datestamp}"
fi

# Default parallellism if none set
make_load="`(echo processor; cat /proc/cpuinfo 2>/dev/null echo processor) \
           | grep -c processor`"

if [ "x${jobs}" = "x" ]
then
    jobs=${make_load}
fi

if [ "x${load}" = "x" ]
then
    load=${make_load}
fi

parallel="-j ${jobs} -l ${load}"


logterm "START BUILD: $(date)"

logonly "Build Directory:          ${build_dir}"
logonly "Install Directory:        ${install_dir}"
logonly "SymlinkeDirectory:        ${symlink_dir}"
logonly "Datestamp:                ${datestamp}"
logonly "Clean:                    ${do_clean}"
logonly "Automatic pull:           ${auto_pull}"
logonly "Automatic checkout:       ${auto_checkout}"
logonly "Rebuild unified source:   ${rebuild_unisrc}"
logonly "Unified source directory: ${unisrc_dir}"
logonly "Maximum jobs:             ${jobs}"
logonly "Maximum load:             ${load}"
logonly "Use GMP source:           ${do_gmp}"
logonly "Use MPFR source:          ${do_mpfr}"
logonly "Use MPC source:           ${do_mpc}"
logonly "Use ISL source:           ${do_isl}"
logonly "Use Cloog source:         ${do_cloog}"
logonly "Use relative RPATH:       ${do_rel_rpaths}"
logonly "Target CFLAGS:            ${CFLAGS_FOR_TARGET}"
logonly "Extra config flags:       ${config_extra}"


################################################################################
#                                                                              #
#	     Validate source and build unified source directory.               #
#                                                                              #
################################################################################


# Work from the base directory.

if ! cd ${basedir}
then
    logterm "ERROR: Unable to change to base directory ${basedir}."
    failedbuild
fi

# Sanity check that we have everything we need. Take the opportunity to set up
# the component lists at the same time. Note that later items in the list
# override earlier items.

# First the main components, which we *must* have.
res="success"
check_dir_exists "gcc" | res="failure"
check_dir_exists "binutils" | res="failure"
check_dir_exists "gdb" | res="failure"
check_dir_exists "newlib" | res="failure"
check_dir_exists "cgen" | res="failure"
check_dir_exists "sdk" | res="failure"

component_dirs="newlib gdb cgen binutils gcc"

# Optional GCC infrastructure components
infra_dir=""
infra_exclude=""

if [ "${do_gmp}" = "--do-gmp" ]
then
    check_dir_exists "gcc-infrastructure/gmp" | res="failure"
    infra_dir="gcc-infrastructure"
else
    infra_exclude="gmp ${infra_exclude}"
fi

if [ "${do_mpfr}" = "--mpfr" ]
then
    check_dir_exists "gcc-infrastructure/mpfr" | res="failure"
    infra_dir="gcc-infrastructure"
else
    infra_exclude="mpfr ${infra_exclude}"
fi

if [ "${do_mpc}" = "--mpc" ]
then
    check_dir_exists "gcc-infrastructure/mpc" | res="failure"
    infra_dir="gcc-infrastructure"
else
    infra_exclude="mpc ${infra_exclude}"
fi

if [ "${do_isl}" = "--isl" ]
then
    check_dir_exists "gcc-infrastructure/isl" | res="failure"
    infra_dir="gcc-infrastructure"
else
    infra_exclude="isl ${infra_exclude}"
fi

if [ "${do_cloog}" = "--cloog" ]
then
    check_dir_exists "gcc-infrastructure/cloog" | res="failure"
    infra_dir="gcc-infrastructure"
else
    infra_exclude="cloog ${infra_exclude}"
fi

if [ "${res}" != "success" ]
then
    failedbuild
fi

component_dirs="${infra_dir} ${component_dirs}"

# Checkout and pull repos if necessary
if ! ${basedir}/sdk/get-versions.sh ${basedir} ${logfile} ${auto_pull} \
                                    ${auto_checkout}
then
    logterm "ERROR: Could not get correct versions of tools"
    failedbuild
fi

# We need to force a clean build if there was not a previous successful
# configure.
if [ "${do_clean}" == "--no-clean" ]
then
    if [ -d ${build_dir} -a -e ${build_dir}/config.log ]
    then
	if grep -q "configure: exit 0" ${build_dir}/config.log
	then
	    # We did have a previous successful config we can reuse. This then
	    # overrides any specified install directory.
	    install_dir=`sed -n -e 's/^.*\-\-prefix=\([^ \t\n]*\).*$/\1/p' \
                             < ${build_dir}/config.log | head -1`
	    logterm "Reusing previous build and installing at ${install_dir}."
	else
	    do_clean="--clean"
	fi
    else
	do_clean="--clean"
    fi
fi

# Clean up old build directory if specified.
if [ "${do_clean}" = "--clean" ]
then
    rm -rf "${build_dir}"
fi

# Ensure the build directory exists
if ! mkdir -p "${build_dir}"
then
    logterm "ERROR: Failed to create build directory ${build_dir}."
    failedbuild
fi

# Blow away the unified source directory if requested
if [ "${rebuild_unisrc}" = "--rebuild-unisrc" ]
then
    rm -rf "${unisrc_dir}" >> ${logfile} 2>&1
fi

# Create unified source directory if it doesn't exist
if ! [ -d "${unisrc_dir}" ]
then
  # Create a unified source directory
  if ! mkdir "${unisrc_dir}" >> ${logfile} 2>&1
  then
      logterm "ERROR: Could not create unified source dir ${unisrc_dir}."
      failedbuild
  fi

  logterm "Creating unified source tree..."
  if ! ${basedir}/sdk/symlink-all.sh "${basedir}" "${logfile}" \
           "${infra_exclude}" "${unisrc_dir}" "${component_dirs}"
  then
      logterm "ERROR: Failed to build unified source tree in ${unisrc_dir}."
      failedbuild
  fi
fi


################################################################################
#                                                                              #
#		 Configure, build and install the tool chain                   #
#                                                                              #
################################################################################


# Change to the build directory
if ! cd ${build_dir}
then
    logterm "ERROR: Could not change to build directory ${build_dir}."
    failedbuild
fi

# Configure the entire tool chain, but only if we are doing a clean build

# @todo Should we enable Python support in GDB? If so do we need to check
#       Python is available?
if [ ${do_clean} = "--clean" ]
then
    logterm "Configuring tool chain..."
    export CFLAGS_FOR_TARGET
    if ! "${unisrc_dir}/configure" --target=epiphany-elf \
	--with-pkgversion="Epiphany toolchain ${RELEASE}" \
	--with-bugurl=support-sdk@adapteva.com \
	--enable-fast-install=N/A \
	--enable-languages=c,c++ --prefix="${install_dir}" \
	--with-newlib --disable-gdbtk ${config_extra} >> "${logfile}" 2>&1
    then
	logterm "ERROR: Tool chain configuration failed."
	failedbuild
    fi
fi

# Build the entire tool chain: binutils, GCC, newlib and GDB
logterm "Building tool chain..."
if ! make ${parallel} all-build all-binutils all-gas all-ld all-gcc \
        all-target-libgcc all-target-libgloss all-target-newlib \
        all-target-libstdc++-v3 all-gdb all-sim >> "${logfile}" 2>&1
then
  logterm "ERROR: Tool chain build failed."
  failedbuild
fi

# Install binutils, GCC, newlib and GDB
logterm "Installing tool chain..."
if ! make install-binutils install-gas install-ld install-gcc \
        install-target-libgcc install-target-libgloss install-target-newlib \
        install-target-libstdc++-v3 install-gdb install-sim >> "${logfile}" 2>&1
then
  logterm "Error: Tool chain installation failed."
  failedbuild
fi


################################################################################
#                                                                              #
#			 Tidy up installed tool chain                          #
#                                                                              #
################################################################################


# Patch RPATHs so they are relative
logterm "Setting relative RPATHs..."
if [ "x${rel_rpaths}" = "x--rel-rpaths" ]
then
    if ! "${basedir}"/sdk/rel-rpaths.sh >> "${logfile}" 2>&1
    then
	logterm "ERROR: Failed to set relative RPATHs"
	failedbuild
	exit 1
    fi
fi

# Create symbolic links in install directory for epiphany executables and man
# pages.
logterm "Creating symbolic links for tools"

if ! cd "${install_dir}/bin"
then
    logterm "ERROR: Unable to select bin directory in ${install_dir}"
    failedbuild
fi

for executable in epiphany-elf-*
do
    ename=`echo ${executable} | sed 's/epiphany-elf-/e-/'`
    rm -f ${ename}
    if ! ln -s ${executable} ${ename}
    then
	logterm "Warning: Unable to create symbolic link to ${ename}"
    fi
done

logterm "Creating symbolic links for man pages"

if ! cd "${install_dir}/share/man/man1"
then
    logterm "ERROR: Unable to select share/man/man1 directory in ${install_dir}"
    failedbuild
fi

for manpage in epiphany-elf-*
do
    ename=`echo ${manpage} | sed 's/epiphany-elf-/e-/'`
    rm -f ${ename}
    if ! ln -s ${manpage} ${ename}
    then
	logterm "Warning: Unable to create symbolic link to ${ename}"
    fi
done

# Set up the symlink directory if specified. This will be the reporting
# directory.
if [ "x${symlink_dir}" = "x" ]
then
    report_dir="${install_dir}"
else
    echo "Setting up the main symlink directory..."
    report_dir="${symlink_dir}"
    rm -f ${symlink_dir}
    ln -s ${install_dir} ${symlink_dir}
fi

logterm "Tools installed at ${report_dir}/bin"
logterm "Manual pages installed at ${report_dir}/share/man"
logterm "Ensure these directories are in your PATH and MANPATH"

logterm "BUILD COMPLETE: $(date)"
