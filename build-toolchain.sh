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

# There are two possible ways of using this script

# 1.  The compiler is intended to run on the same machine on which it is built
#     (typically an Intel based PC or the ARM based Parallella board). The
#     compiler will generate code for the Epiphany processor.  Since the code
#     is to run on a different machine to the platform on which the compiler
#     is run, this is known as a *cross compiler*.

# 2.  The compiler is intended to run on a *different* machine (known as the
#     host) to that on which it is built.  For Parallella this is invariably
#     done on an Intel based PC to create a compiler which will run on the ARM
#     based Parallella board where it will generate code for the Epiphany
#     processor.  The compiler is built on one machine to be hosted on a
#     second machine to generate code for a third machine.  This is known as a
#     *Canadian cross compiler".

# In this script method 2 is trigged by setting the --host parameter to
# something other than the build architecture.

# When reading the comments in this script it is useful to be clear about
# three terms:

# build machine

#     The platform on which this script is being run to build the compiler.

# host machine

#     The platform on which the generated compiler will be run (invariably the
#     Parallella ARM platform).

# target machine

#     The platform for which the compiler will generate code. In this case for
#     the Epiphany processor.

# On this notation, method 1 of using this script has build and host machines
# identical.

# Invocation:

#     ./build-toolchain.sh [--build-dir <dir>]
#                          [--build-dir-build <dir>]
#                          [--build-dir-host <dir>]
#                          [--host <host-triplet>]
#                          [--install-dir <dir>]
#                          [--install-dir-build <dir>]
#                          [--install-dir-host <dir>]
#                          [--symlink-dir <dir>]
#                          [--destdir <dir>]
#                          [--datestamp-install]
#                          [--datestamp-install-build]
#                          [--datestamp-install-host]
#                          [--clean | --no-clean]
#                          [--clean-build | --no-clean-build]
#                          [--clean-host | --no-clean-host]
#                          [--preserve-unisrc | --rebuild-unisrc]
#                          [--unified-dir <dir>]
#                          [--auto-pull | --no-auto-pull]
#                          [--auto-checkout | --no-auto-checkout]
#                          [--jobs <count>] [--load <load>] [--single-thread]
#                          [--gmp | --no-gmp]
#                          [--mpfr | --no-mpfr]
#                          [--mpc | --no-mpc]
#                          [--isl | --no-isl]
#                          [--cloog | --no-cloog]
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

# --build-dir-build <dir>

#     The directory in which the tool chain to run on the *build* machine will
#     be built.  The default is builds/bd-epiphany-$RELEASE in the base
#     directory.

# --build-dir <dir>
# --build-dir-host <dir>

#     The directory in which the tool chain to run on the *host* machine will
#     be built if needed.  The default is builds/bd-epiphany-host-$RELEASE in
#     the base directory.  This argument is only relevant if --host is set to
#     a triplet whose architecture differs from the build machine.

# --host <host-triplet>

#     If specified, the tool chain will be built to run on the designated
#     host.  This is of most value to users wishing to build the toolchain on
#     a PC, but to run on the Parallella board.  Useful values are
#     "arm-linux-gnu" on Fedora/Red Hat systems "arm-linux-gnueabihf" or
#     "arm-linux-gnueabihf" on Ubuntu (the hf at the end signifying hardware
#     floating point support, which Parallella has, and will yield a faster
#     compiler).

#     An Epiphany cross-compiler for the build machine is still needed (to
#     build libraries). This will trigger building of GCC and binutils for the
#     host unless those tools have been previously built, or are found in the
#     existing search path.

# --install-dir-build <install_dir>

#     The directory in which the tool chain to run on the *build* machine
#     should be installed. If not specified, it will be installed in the
#     /opt/adapteva/edsk.$RELEASE/tools/e-gnu.<build-arch> directory, where
#     <build-arch> is the architecture name of the *build* machine (so should
#     be one of x86, x86_64 or armv7l).

# --install-dir <install_dir>
# --install-dir-host <install_dir>

#     The directory in which the tool chain to run on the *host* machine
#     should be installed if needed. If not specified, it will be installed in
#     the /opt/adapteva/edsk.$RELEASE/tools/e-gnu.<host-arch> directory, where
#     <host-arch> is the architecture name of the *host* machine (so should
#     usually be armv7l).  This argument is only relevant if --host is set to
#     a triplet whose architecture differs from the build machine.

# --destdir <destdir>

#     If specified, all files will be installed to <destdir> staging
#     directory.

# --symlink-dir <symlink_dir>

#     If specified, the install directory will be symbolically linked to this
#     directory.  It must be a plain relative directory name (no
#     hierarchy). It defaults to e-gnu.  If a hierarchy is given, only the
#     basename will be used.

# --datestamp-install-build

#     If specified, this will insert a date and timestamp in the install
#     directory name for the *build* architecture.  If the install directory
#     has the default name, the datestamp will be inserted after the $RELEASE,
#     otherwise it will be appended to the name.

# --datestamp-install
# --datestamp-install-host

#     If specified, this will insert a date and timestamp in the install
#     directory name for the *host* architecture.  If the install directory
#     has the default name, the datestamp will be inserted after the $RELEASE,
#     otherwise it will be appended to the name.

# --clean-build | --no-clean-build

#     If --clean-build is specified, delete any previous build directory for
#     the *build* machine. Default --no-clean-build.

# --clean | --no-clean
# --clean-host | --no-clean-host

#     If --clean or --clean-host is specified, delete any previous host
#     directory for the *host* machine. Default --no-clean-host.  This
#     argument is only relevant if --host is set to a triplet whose
#     architecture differs from the build machine.

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
# --ncurses | --no-ncurses

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
#     omitted.  Specifying --no-ncurses when the developer cross-compiled
#     package is not available will cause a Canadian Cross build (i.e. with
#     different build and host architectures) to fail.

#     Defaults --gmp --mpfr --mpc --isl --cloog --ncurses.

# --expat | --no-expat

#     Indicate that the corresponding GDB infrastructure component exists as a
#     source directory within the base directory and should be linked into the
#     unified source directory.  With the "--no-" versions, indicate the
#     component is not available as a source directory, and that the compiler
#     should rely on the installed developer package for the relevant headers
#     and libraries.

#     Defaults --expat

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
# otherwise.  Note that some errors do not prevent a tool chain being built,
# so will generate warning messages in the log, but the script will still
# return 0.


# Define the basedir
d=`dirname "$0"`
basedir=`(cd "$d/.." && pwd)`

# Common functions
. ${basedir}/sdk/common-functions

# Set the release parameters
. ${basedir}/sdk/define-release.sh

# Set up a clean log
logfile=${LOGDIR}/build-$(date -u +%F-%H%M).log
rm -f "${logfile}"
echo "Logging to ${logfile}"


################################################################################
#                                                                              #
#			       Parse arguments                                 #
#                                                                              #
################################################################################

# Defaults
bd_build=
bd_host=
host=
id_build=
id_host=
staging_host=
destdir=
symlink_dir=e-gnu
ds_build=
ds_host=
do_clean_build="--no-clean-build"
do_clean_host="--no-clean-host"
auto_pull="--auto-pull"
auto_checkout="--auto-checkout"
rebuild_unisrc="--preserve-unisrc"
unisrc_dir=${basedir}/unisrc-${RELEASE}
jobs=
load=
# TODO: Get rid of all these flags. If you don't want something comment out the
# line in the components file.
do_gmp="--gmp"
do_mpfr="--mpfr"
do_mpc="--mpc"
do_isl="--isl"
do_cloog="--no-cloog"
do_ncurses="--ncurses"
do_expat="--expat"

# The assembler and/or linker are broken so that constant merging doesn't
# work.
CFLAGS_FOR_TARGET=${CFLAGS_FOR_TARGET:-"-O2 -g"}
CXXFLAGS_FOR_TARGET=${CXXFLAGS_FOR_TARGET:-"-O2 -g"}
CFLAGS=${CFLAGS:-"-O2 -g"}
CXXFLAGS=${CXXFLAGS:-"-O2 -g"}
config_extra=""
disable_werror="--disable-werror"


until
opt=$1
case ${opt} in

    --build-dir-build)
	shift
	bd_build=`absdir "$1"`
	;;

    --build-dir | --build-dir-host)
	shift
	bd_host=`absdir "$1"`
	;;

    --host)
	shift
	host="$1"
	;;

    --install-dir-build)
	shift
	id_build=`absdir "$1"`
	;;

    --install-dir | --install-dir-host)
	shift
	id_host=`absdir "$1"`
	;;

    --destdir)
	shift
	destdir="$(absdir $1 | sed s,[/]*$,,g)/"
	;;

    --symlink-dir)
	shift
	# Force it to be simple name
	symlink_dir="`basename $1`"
	;;

    --datestamp-install-build)
	ds_build=-`date -u +%F-%H%M`
	;;

    --datestamp-install | --datestamp-install-host)
	ds_host=-`date -u +%F-%H%M`
	;;

    --clean-build | --no-clean-build)
	do_clean_build="$1"
	;;

    --clean)
	do_clean_host="--clean-host"
	;;

    --no-clean)
	do_clean_host="--no-clean-host"
	;;

    --clean-host | --no-clean-host)
	do_clean_host="$1"
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

    --ncurses | --no-ncurses)
	do_ncurses="$1"
	;;

    --expat | --no-expat)
	do_expat="$1"
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
	disable_werror="$1"
	;;

    --enable-cgen-maint)
	config_extra="${config_extra} --enable-cgen-maint"
	;;

    ?*)
        echo "Usage: ./build-toolchain.sh [--build-dir <dir>]"
        echo "             [--build-dir-build <dir>]"
        echo "             [--build-dir-host <dir>]"
        echo "             [--host <host-triplet>]"
        echo "             [--install-dir <dir> ]"
        echo "             [--install-dir-build <dir> ]"
        echo "             [--install-dir-host <dir> ]"
        echo "             [--symlink-dir <dir>]"
        echo "             [--datestamp-install]"
        echo "             [--datestamp-install-build]"
        echo "             [--datestamp-install-host]"
        echo "             [--clean | --no-clean]"
        echo "             [--clean-build | --no-clean-build]"
        echo "             [--clean-host | --no-clean-host]"
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
        echo "             [--ncurses | --no-ncurses]"
        echo "             [--expat | --no-expat]"
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

# Sort out Canadian cross stuff. First is it really a canadian cross
build_arch=`uname -m`

if [ "x" != "x${host}" ]
then
    host_arch=`getarch ${host}`
    if [ "x${host_arch}" = "x${build_arch}" ]
    then
	# Not really a Canadian Cross
	host=
    fi
else
    host_arch=${build_arch}
fi

# Create build directory names if needed.
if [ "x${bd_build}" = "x" ]
then
    bd_build=${basedir}/builds/bd-${build_arch}-${RELEASE}
fi
if [ "x${bd_host}" = "x" ]
then
    bd_host=${basedir}/builds/bd-${host_arch}-${RELEASE}
fi

# Set up staging directory. This is where we install static libraries, include
# files etc. that is needed for building, but we don't want in the SDK.
staging_host=${bd_host}/staging


# Set up install dirs. Four cases each time according to whether and install
# dir and a datestamp have been set.
if [ "x${id_build}" = "x" ]
then
    if [ "x${ds_build}" = "x" ]
    then
	# Default install directory without datestamp
	id_build="/opt/adapteva/esdk.${RELEASE}/tools/e-gnu.${build_arch}"
    else
	# Default install directory with datestamp
	id_build="/opt/adapteva/esdk.${RELEASE}-${ds_build}/tools/e-gnu.${build_arch}"
    fi
else
    if [ "x${ds_build}" = "x" ]
    then
	# Custom install directory without datestamp
	# No nothing
	true
    else
	# Custom install directory with datestamp
	id_build="${id_build}-${ds_build}"
    fi
fi

if [ "x${id_host}" = "x" ]
then
    if [ "x${ds_host}" = "x" ]
    then
	# Default install directory without datestamp
	id_host="/opt/adapteva/esdk.${RELEASE}/tools/e-gnu.${host_arch}"
    else
	# Default install directory with datestamp
	id_host="/opt/adapteva/esdk.${RELEASE}-${ds_host}/tools/e-gnu.${host_arch}"
    fi
else
    if [ "x${ds_host}" = "x" ]
    then
	# Custom install directory without datestamp
	# No nothing
	true
    else
	# Custom install directory with datestamp
	id_host="${id_host}-${ds_host}"
    fi
fi

if [ "x${destdir}" = "x" ]
then
    destdir_str=""
else
    destdir_str="DESTDIR=\"${destdir}\""
fi


# Add ${disable_werror} to ${config_extra}. Note that this argument takes
# precedence, so don't try setting disable-werror in config_extra.
config_extra="${config_extra} ${disable_werror}"

# Default parallellism if none set.
mem=`sed < /proc/meminfo -n -e 's/MemTotal:[ \n\t] *\([0-9]*\).*$/\1/p'`
if [ "0${mem}" -le 2097152 ]
then
    # No parallelism if memory is small.
    make_load=1
else
    make_load="`(echo processor; cat /proc/cpuinfo 2>/dev/null || echo processor) \
           | grep -c processor`"
fi

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

logonly "Build directory (build arch):   ${bd_build}"
logonly "Build directory (host arch):    ${bd_host}"
logonly "Build architecture:             ${build_arch}"
logonly "Host:                           ${host}"
logonly "Host architecture:              ${host_arch}"
logonly "Install directory (build arch): ${id_build}"
logonly "Install directory (host arch):  ${id_host}"
logonly "Staging directory (destdir):    ${destdir}"
logonly "Symlink directory:              ${symlink_dir}"
logonly "Datestamp (build arch):         ${ds_build}"
logonly "Datestamp (host arch):          ${ds_host}"
logonly "Clean (build arch):             ${do_clean_build}"
logonly "Clean (host arch):              ${do_clean_host}"
logonly "Automatic pull:                 ${auto_pull}"
logonly "Automatic checkout:             ${auto_checkout}"
logonly "Rebuild unified source:         ${rebuild_unisrc}"
logonly "Unified source directory:       ${unisrc_dir}"
logonly "Maximum jobs:                   ${jobs}"
logonly "Maximum load:                   ${load}"
logonly "Use GMP source:                 ${do_gmp}"
logonly "Use MPFR source:                ${do_mpfr}"
logonly "Use MPC source:                 ${do_mpc}"
logonly "Use ISL source:                 ${do_isl}"
logonly "Use Cloog source:               ${do_cloog}"
logonly "Use ncurses source:             ${do_ncurses}"
logonly "Use expat source:               ${do_expat}"
logonly "Target CFLAGS:                  ${CFLAGS_FOR_TARGET}"
logonly "Target CXXFLAGS:                ${CXXFLAGS_FOR_TARGET}"
logonly "CFLAGS:                         ${CFLAGS}"
logonly "CXXFLAGS:                       ${CXXFLAGS}"
logonly "Extra config flags:             ${config_extra}"


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

# Sanity check if we are trying to build a Canadian cross
if [ "x" != "x${host}" ]
then
    if ! which ${host}-gcc >> ${logfile} 2>&1
    then
	logterm "ERROR: No cross-compiler for ${host} found"
	failedbuild
    fi

    host_str="--host=${host}"
else
    host_str=""
fi

# Sanity check that we have everything we need. Take the opportunity to set up
# the component lists at the same time. Note that later items in the list
# override earlier items.
res="success"

# First the main components, which we *must* have.
check_dir_exists "gcc"      || res="failure"
check_dir_exists "binutils" || res="failure"
check_dir_exists "gdb"      || res="failure"
check_dir_exists "newlib"   || res="failure"
check_dir_exists "cgen"     || res="failure"
check_dir_exists "sdk"      || res="failure"

component_dirs="newlib gdb binutils cgen gcc"

# Optional GCC infrastructure components
infra_dir=""
infra_exclude=""

if [ "${do_gmp}" = "--gmp" ]
then
    check_dir_exists "gcc-infrastructure/gmp" || res="failure"
    infra_dir="gcc-infrastructure"
else
    infra_exclude="gmp ${infra_exclude}"
fi

if [ "${do_mpfr}" = "--mpfr" ]
then
    check_dir_exists "gcc-infrastructure/mpfr" || res="failure"
    infra_dir="gcc-infrastructure"
else
    infra_exclude="mpfr ${infra_exclude}"
fi

if [ "${do_mpc}" = "--mpc" ]
then
    check_dir_exists "gcc-infrastructure/mpc" || res="failure"
    infra_dir="gcc-infrastructure"
else
    infra_exclude="mpc ${infra_exclude}"
fi

if [ "${do_isl}" = "--isl" ]
then
    check_dir_exists "gcc-infrastructure/isl" || res="failure"
    infra_dir="gcc-infrastructure"
else
    infra_exclude="isl ${infra_exclude}"
fi

if [ "${do_cloog}" = "--cloog" ]
then
    check_dir_exists "gcc-infrastructure/cloog" || res="failure"
    infra_dir="gcc-infrastructure"
else
    infra_exclude="cloog ${infra_exclude}"
fi

if [ "${do_ncurses}" = "--ncurses" ]
then
    check_dir_exists "gcc-infrastructure/ncurses" || res="failure"
    infra_dir="gcc-infrastructure"
else
    infra_exclude="ncurses ${infra_exclude}"
fi

if [ "${do_expat}" = "--expat" ]
then
    check_dir_exists "gcc-infrastructure/expat" || res="failure"
    infra_dir="gcc-infrastructure"
else
    infra_exclude="expat ${infra_exclude}"
fi


if [ "${res}" != "success" ]
then
    failedbuild
fi

component_dirs="${infra_dir} ${component_dirs}"

regex="toolchain"
# Checkout and pull repos if necessary
if ! ${basedir}/sdk/get-versions.sh ${basedir} sdk/components.conf \
                                    ${logfile} ${auto_pull} \
                                    ${auto_checkout} \
				    --regex ${regex}
then
    logterm "ERROR: Could not get correct versions of tools"
    failedbuild
fi

# We need to force a clean build if there was not a previous successful
# configure.
if [ "${do_clean_host}" = "--no-clean-host" ]
then
    if [ -d ${bd_host} -a -e ${bd_host}/config.log ] \
	&& grep "configure: exit 0" ${bd_host}/config.log > /dev/null 2>&1
    then
        # We did have a previous successful config we can reuse. This then
	# overrides any specified install directory.
	id_host=`sed -n -e 's/^.*\-\-prefix=\([^ \t\n]*\).*$/\1/p' \
                         < ${bd_host}/config.log | head -1`
	logterm "Reusing previous build and installing at ${id_host}."
    else
	logterm "Forcing clean build"
	do_clean_host="--clean-host"
    fi
fi

# Clean up old build directories if specified.
if [ "${do_clean_build}" = "--clean-build" ]
then
    rm -rf "${bd_build}"
fi

# Clean up old build directories if specified.
if [ "${do_clean_host}" = "--clean-host" ]
then
    rm -rf "${bd_host}"
    rm -rf "${staging_host}"
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

  # Apply patches. Symlink will be overwritten, not the underlying file.
  for p in ${basedir}/sdk/patches/*; do
    logterm "Applying patch: $p"
    if ! patch -d ${unisrc_dir} --follow-symlinks -Np1 < $p
    then
      logterm "ERROR: Failed to apply patch \"${p}\"."
      failedbuild
    fi
  done
fi


# Ensure the staging directory exists.
if ! mkdir -p "$staging_host"
then
    logterm "ERROR: Failed to create staging directory ${staging_host}."
    failedbuild
fi


################################################################################
#                                                                              #
#   Make sure we have an epiphany tool chain on the build machine if needed    #
#                                                                              #
################################################################################

# Put the build directory toolchain on our PATH. Then we will find it
# if we have already built it.
PATH=${id_build}/bin:$PATH
export PATH

if [ "x${host}" != "x" ]
then
    if  which epiphany-elf-gcc && which epiphany-elf-as \
	&& which epiphany-elf-ld && which epiphany-elf-ar
    then
	logterm "Epiphany tools found on build machine"
    else
        # A cross compiler also needs a compiler on the build machine to build
        # libraries.

        # Ensure the build directory exists. We don't do fancy stuff about
	# trying to preserve previous builds. If we get here, by definition
	# they were no good.
	rm -rf "${bd_build}"
	if ! mkdir -p "${bd_build}"
	then
	    logterm "ERROR: Failed to create build directory ${bd_build}."
	    failedbuild
	fi

        # Change to the build directory
	if ! cd ${bd_build}
	then
	    logterm "ERROR: Could not change to build directory ${bd_build}."
	    failedbuild
	fi

	# Force guile 1.8 in build if available.
	guile18=`which guile1.8 2>/dev/null`
	[ "x$guile18" = x ] && guile18=`which guile-1.8 2>/dev/null`
	if ! [ "x${guile18}" = "x" ]
	then
	    mkdir -p guile/libguile
	    ln -sf ${guile18} guile/libguile/guile
	fi
	unset guile18

        # Configure the required components of the tool chain. We only need
        # binutils, as, ld and gcc. We need to temporarily move
        # CFLAGS_FOR_TARGET out of the way.
	# Append -fPIC to CFLAGS. Needed for building simulator shared library.
	logterm "Configuring build machine tool chain..."
	OLD_CFLAGS_FOR_TARGET=${CFLAGS_FOR_TARGET}
	CFLAGS_FOR_TARGET=
	OLD_CXXFLAGS_FOR_TARGET=${CXXFLAGS_FOR_TARGET}
	CXXFLAGS_FOR_TARGET=
	if ! "${unisrc_dir}/configure" \
	    CFLAGS="${CFLAGS} -fPIC" \
	    CXXFLAGS="${CXXFLAGS} -fPIC" \
	    --target=epiphany-elf \
	    --with-pkgversion="Epiphany toolchain ${RELEASE}" \
	    --with-bugurl=support-sdk@adapteva.com \
	    --enable-fast-install=N/A \
	    --enable-languages=c,c++ --prefix="${id_build}" \
	    --with-newlib --disable-gdbtk ${config_extra} >> "${logfile}" 2>&1
	then
	    logterm "ERROR: Tool chain configuration for build machine failed."
	    failedbuild
	fi

        # Build the parts of the tool chain we need: binutils and GCC
	logterm "Building build machine tool chain..."
	if ! make ${parallel} all-build all-binutils all-gas all-ld all-gcc \
	    all-target-libgcc all-target-libgloss all-target-newlib \
	    all-target-libstdc++-v3 >> "${logfile}" 2>&1
	then
	    logterm "ERROR: Tool chain build for build machine failed."
	    failedbuild
	fi

        # Install binutils, GCC, newlib and GDB
	logterm "Installing build machine tool chain..."
	if ! make install-binutils install-gas install-ld install-gcc \
	    install-target-libgcc install-target-libgloss install-target-newlib \
	    install-target-libstdc++-v3 >> "${logfile}" 2>&1
	then
	    logterm "Error: Tool chain installation for build machine failed."
	    failedbuild
	fi

        # Sanity check that we now do really have the tools we need.
	if ! ( which epiphany-elf-gcc && which epiphany-elf-as \
	    && which epiphany-elf-ld && which epiphany-elf-ar )
	then
	    logterm "Error: Failed to created tools for build machine."
	    failedbuild
	fi
	CFLAGS_FOR_TARGET=${OLD_CFLAGS_FOR_TARGET}
	CXXFLAGS_FOR_TARGET=${OLD_CXXFLAGS_FOR_TARGET}
    fi

    # We also first build ncurses in the case of cross compilation such a
    # suitable termcap library is available for GDB.
    if [ "${do_ncurses}" = "--ncurses" ]
    then
	bd_ncurses="${bd_host}-ncurses"
	rm -rf "${bd_ncurses}"
	if ! mkdir -p "${bd_ncurses}"
	then
	    logterm "ERROR: Failed to create ncurses build directory."
	fi

	if ! cd "${bd_ncurses}"; then
	    logterm "ERROR: Could not change to build directory ${bd_ncurses}."
	    failedbuild
	fi

	logterm "Building ncurses for host..."
	if ! "${unisrc_dir}/ncurses/configure" ${host_str} --prefix="${staging_host}" \
	    --without-progs --without-ada --without-manpages --without-tests \
	    --with-abi-version=5 --with-shared \
	    --with-terminfo-dirs="${staging_host}/install/share/terminfo:/usr/share/terminfo"\
	    >> "${logfile}" 2>&1
	then
	    logterm "ERROR: Unable to configure ncurses for host"
	    failedbuild
	fi

	if ! make ${parallel} >> "${logfile}" 2>&1
	then
	    logterm "ERROR: Unable to build ncurses for host"
	    failedbuild
	fi

	if ! make install.libs install.includes >> "${logfile}" 2>&1
	then
	    logterm "ERROR: Unable to install ncurses for host"
	    failedbuild
	fi

    fi

    # We also first build expat in the case of cross compilation so a
    # suitable XML library is available for GDB.
    if [ "${do_expat}" = "--expat" ]
    then
	bd_expat="${bd_host}-expat"
	rm -rf "${bd_expat}"
	if ! mkdir -p "${bd_expat}"
	then
	    logterm "ERROR: Failed to create expat build directory."
	fi

	if ! cd "${bd_expat}"; then
	    logterm "ERROR: Could not change to build directory ${bd_expat}."
	    failedbuild
	fi

	logterm "Building expat for host..."
	if ! "${unisrc_dir}/expat/configure" ${host_str} --prefix="${staging_host}" \
	    >> "${logfile}" 2>&1
	then
	    logterm "ERROR: Unable to configure expat for host"
	    failedbuild
	fi

	if ! make ${parallel} buildlib >> "${logfile}" 2>&1
	then
	    logterm "ERROR: Unable to build expat for host"
	    failedbuild
	fi

	if ! make installlib >> "${logfile}" 2>&1
	then
	    logterm "ERROR: Unable to install expat for host"
	    failedbuild
	fi
    fi

    # We add the include and library paths to CFLAGS/CXXFLAGS/CPPFLAGS/LDFLAGS
    # respectively to make them available for the real build.
    CFLAGS="-I${staging_host}/include -I${destdir}${id_host}/include $CFLAGS"
    CXXFLAGS="-I${staging_host}/include -I${destdir}${id_host}/include $CXXFLAGS"
    CPPFLAGS="-I${staging_host}/include -I${destdir}${id_host}/include $CPPFLAGS"
    LDFLAGS="-L${staging_host}/lib -L${destdir}${id_host}/lib $LDFLAGS"
    export CFLAGS
    export CXXFLAGS
    export CPPFLAGS
    export LDFLAGS
fi


################################################################################
#                                                                              #
#	       Configure, build and install the host tool chain                #
#                                                                              #
################################################################################

# Ensure the build directory exists. We build in the host build directory.
if ! mkdir -p "${bd_host}"
then
    logterm "ERROR: Failed to create build directory ${bd_host}."
    failedbuild
fi

# Change to the build directory
if ! cd ${bd_host}
then
    logterm "ERROR: Could not change to build directory ${bd_host}."
    failedbuild
fi

# Force guile 1.8 in build if available.
guile18=`which guile1.8 2>/dev/null`
[ "x$guile18" = x ] && guile18=`which guile-1.8 2>/dev/null`
if ! [ "x${guile18}" = "x" ]
then
    mkdir -p guile/libguile
    ln -sf ${guile18} guile/libguile/guile
fi
unset guile18

# Configure the entire tool chain, but only if we are doing a clean build

export CFLAGS_FOR_TARGET
export CXXFLAGS_FOR_TARGET

# @todo Should we enable Python support in GDB? If so do we need to check
#       Python is available?
if [ ${do_clean_host} = "--clean-host" ]
then
    logterm "Configuring tool chain..."
    # Append -fPIC to CFLAGS. Needed for building simulator shared library.
    if ! "${unisrc_dir}/configure" \
	CFLAGS_FOR_TARGET="${CFLAGS_FOR_TARGET}" \
	CXXFLAGS_FOR_TARGET="${CXXFLAGS_FOR_TARGET}" \
	CFLAGS="${CFLAGS} -fPIC" \
	CXXFLAGS="${CXXFLAGS} -fPIC" \
	--target=epiphany-elf ${host_str} \
	--with-pkgversion="Epiphany toolchain ${RELEASE}" \
	--with-bugurl=support-sdk@adapteva.com \
	--enable-fast-install=N/A \
	--enable-languages=c,c++ --prefix="${id_host}" \
	--with-newlib --disable-gdbtk ${config_extra} >> "${logfile}" 2>&1
    then
	logterm "ERROR: Tool chain configuration for host machine failed."
	failedbuild
    fi
fi

# Build the entire tool chain: binutils, GCC, newlib and GDB
logterm "Building tool chain..."
if ! make ${parallel} all-build all-binutils all-gas all-ld all-gcc \
        all-target-libgcc all-target-libgloss all-target-newlib \
        all-target-libstdc++-v3 all-gdb all-sim >> "${logfile}" 2>&1
then
  logterm "ERROR: Tool chain build for host machine failed."
  failedbuild
fi

# Install binutils, GCC, newlib and GDB
logterm "Installing tool chain..."
if ! make ${destdir_str} install-binutils install-gas install-ld install-gcc \
        install-target-libgcc install-target-libgloss install-target-newlib \
        install-target-libstdc++-v3 install-gdb install-sim >> "${logfile}" 2>&1
then
  logterm "Error: Tool chain installation for host machine failed."
  failedbuild
fi


# If the toolchain was built for this arch we can now use it
if [ "x${host_arch}" = "x${build_arch}" ]
then
    PATH=${destdir}${id_host}/bin:$PATH
    export PATH
fi


################################################################################
#                                                                              #
#			 Tidy up installed tool chain                          #
#                                                                              #
################################################################################


# Create symbolic links in install directory for epiphany executables and man
# pages.
logterm "Creating symbolic links for tools"

if ! cd "${destdir}${id_host}/bin"
then
    logterm "ERROR: Unable to select bin directory in ${destdir}${id_host}"
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

if ! cd "${destdir}${id_host}/share/man/man1"
then
    logterm "ERROR: Unable to select share/man/man1 directory in ${destdir}${id_host}"
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
    report_dir="${destdir}${id_host}"
else
    echo "Setting up the main symlink directory..."
    id_base=`basename ${id_host}`
    cd "${destdir}${id_host}/.."
    report_dir="`pwd`/${symlink_dir}"
    rm -f "${symlink_dir}"
    ln -s ${id_base} "${symlink_dir}"
fi

logterm "Tools installed at ${report_dir}/bin"
logterm "Manual pages installed at ${report_dir}/share/man"
logterm "Ensure these directories are in your PATH and MANPATH"

# Make the top level link if appropriate
release_dir="esdk.${RELEASE}"
if `echo ${id_host} | grep /opt/adapteva/${release_dir} > /dev/null 2>&1` &&
    [ -d "${destdir}opt/adapteva/${release_dir}" ]
then
    cd ${destdir}opt/adapteva

    if rm -f esdk && ln -s "${release_dir}" esdk
    then
	logterm "Top level ${destdir}opt/adapteva/esdk linked to ${destdir}opt/adapteva/${release_dir}"
    else
	logterm "Unable to create link to ${destdir}opt/adapteva/esdk"
    fi
fi

logterm "BUILD COMPLETE: $(date)"
