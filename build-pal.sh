#!/bin/sh

# Copyright (C) 2009 - 2014 Embecosm Limited
# Copyright (C) 2015 Adapteva

# Contributor Joern Rennecke <joern.rennecke@embecosm.com>
# Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>
# Contributor Simon Cook     <simon.cook@embecosm.com>
# Contributor Ola Jeppsson   <ola@adapteva.com>

# This file is a script to build pal

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

#     ./build-pal           [--build-dir <dir>]
#                           [--build-dir-build <dir>]
#                           [--build-dir-host <dir>]
#                           [--host <host-triplet>]
#                           [--install-dir <dir>]
#                           [--install-dir-host <dir>]
#                           [--install-dir-target <dir>]
#                           [--destdir <dir>]
#                           [--symlink-dir <dir>]
#                           [--datestamp-install]
#                           [--datestamp-install-build]
#                           [--datestamp-install-host]
#                           [--clean | --no-clean]
#                           [--auto-pull | --no-auto-pull]
#                           [--auto-checkout | --no-auto-checkout]
#                           [--jobs <count>] [--load <load>] [--single-thread]
#                           [--target-cflags <flags>]
#                           [--config-extra <flags>]
#                           [--disable-werror | --enable-werror]
#                           [--help | -h]

# A number of arguments specify directories relative to the base directory.
# This is the parent directory of the directory containing this script.  Other
# arguments use the release name, which is set up in the environment variable
# RELEASE.  The meanings of the various options and their defaults are as
# follows.

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

# --target <target-triplet>

#     The only allowed value now is 'epiphany-elf'.

# --install-dir <install_dir>
# --install-dir-host <install_dir>

#     The directory in which the tool chain to run on the *host* machine
#     should be installed if needed. If not specified, it will be installed in
#     the /opt/adapteva/edsk.$RELEASE/tools/host.<host-arch> directory, where
#     <host-arch> is the architecture name of the *host* machine (so should
#     usually be armv7l).  This argument is only relevant if --host is set to
#     a triplet whose architecture differs from the build machine.

# --install-dir-target <install_dir>

#     The install directory for e-lib

# --destdir <destdir>

#     If specified, all files will be installed to <destdir> staging
#     directory.

#     The install directory for BSPs

# --symlink-dir <symlink_dir>

#     If specified, the install directory will be symbolically linked to this
#     directory.  It must be a plain relative directory name (no
#     hierarchy). It defaults to host.  If a hierarchy is given, only the
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
logfile=${LOGDIR}/build-pal-$(date -u +%F-%H%M).log
rm -f "${logfile}"
echo "Logging to ${logfile}"


################################################################################
#                                                                              #
#			       Parse arguments                                 #
#                                                                              #
################################################################################

# Defaults
bd_host=
host=
target="epiphany-elf"
id_host=
id_target=
destdir=
symlink_dir=host
ds_build=
ds_host=
do_clean="--no-clean"
auto_pull="--auto-pull"
auto_checkout="--auto-checkout"
srcdir=${basedir}/pal
jobs=
load=

CFLAGS_FOR_TARGET="-O2 -g"
CFLAGS=${CFLAGS:-"-O2 -g"}
config_extra=""
disable_werror="--disable-werror"


until
opt=$1
case ${opt} in

    --build-dir | --build-dir-host)
	shift
	bd_host=`absdir "$1"`
	;;

    --host)
	shift
	host="$1"
	;;

    --target)
	shift
	target="$1"
	;;

    --install-dir | --install-dir-host)
	shift
	id_host=`absdir "$1"`
	;;

    --install-dir-target)
	shift
	id_target=`absdir "$1"`
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

    --clean | --no-clean)
	do_clean="$1"
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

    --config-extra)
	shift
	config_extra="${config_extra} $1"
	;;

    --disable-werror | --enable-werror)
	disable_werror="$1"
	;;

    ?*)
        echo "Usage: $0    [--build-dir <dir>]"
        echo "             [--build-dir-build <dir>]"
        echo "             [--build-dir-host <dir>]"
        echo "             [--host <host-triplet>]"
        echo "             [--target <target-triplet>]"
        echo "             [--install-dir <dir> ]"
        echo "             [--install-dir-host <dir> ]"
        echo "             [--install-dir-target <dir> ]"
        echo "             [--destdir <dir>]"
        echo "             [--symlink-dir <dir>]"
        echo "             [--datestamp-install]"
        echo "             [--datestamp-install-build]"
        echo "             [--datestamp-install-host]"
        echo "             [--clean | --no-clean]"
        echo "             [--auto-pull | --no-auto-pull]"
        echo "             [--auto-checkout | --no-auto-checkout]"
        echo "             [--jobs <count>] [--load <load>] [--single-thread]"
        echo "             [--target-cflags <flags>]"
        echo "             [--config-extra <flags>]"
        echo "             [--disable-werror | --enable-werror]"
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

if [ "x" != "x${target}" ]
then
    target_arch=`getarch ${target}`
else
    logterm "ERROR: target not set."
    failedbuild
fi

# Create build directory names if needed.
if [ "x${bd_host}" = "x" ]
then
    bd_host=${basedir}/builds/bd-pal-${host_arch}-${RELEASE}
fi


if [ "x${id_host}" = "x" ]
then
    if [ "x${ds_host}" = "x" ]
    then
	# Default install directory without datestamp
	id_host="/opt/adapteva/esdk.${RELEASE}/tools/host.${host_arch}"
	id_gnu="/opt/adapteva/esdk.${RELEASE}/tools/e-gnu.${host_arch}"
    else
	# Default install directory with datestamp
	id_host="/opt/adapteva/esdk.${RELEASE}-${ds_host}/tools/host.${host_arch}"
	id_gnu="/opt/adapteva/esdk.${RELEASE}-${ds_host}/tools/e-gnu.${host_arch}"
    fi
else
    if [ "x${ds_host}" = "x" ]
    then
	# Custom install directory without datestamp
	# Do nothing for id_host

	id_gnu="${id_host}/../e-gnu.${host_arch}"
    else
	# Custom install directory with datestamp
	id_host="${id_host}-${ds_host}"
	id_gnu="${id_host}-${ds_host}/../e-gnu.${host_arch}"
    fi
fi

if [ "x${id_target}" = "x" ]
then
    id_target_str=
else
    id_target_str="--with-target-prefix=${id_target}"
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

logonly "Build directory (host arch):         ${bd_host}"
logonly "Build architecture:                  ${build_arch}"
logonly "Host:                                ${host}"
logonly "Host architecture:                   ${host_arch}"
logonly "Host:                                ${target}"
logonly "Target architecture:                 ${target_arch}"
logonly "Install directory (host arch):       ${id_host}"
logonly "Staging directory (destdir):         ${destdir}"
logonly "Toolchain directory (host arch):     ${id_gnu}"
logonly "Install directory (target arch):     ${id_target}"
logonly "Symlink directory:                   ${symlink_dir}"
logonly "Datestamp (build arch):              ${ds_build}"
logonly "Datestamp (host arch):               ${ds_host}"
logonly "Clean:                               ${do_clean}"
logonly "Automatic pull:                      ${auto_pull}"
logonly "Automatic checkout:                  ${auto_checkout}"
logonly "Maximum jobs:                        ${jobs}"
logonly "Maximum load:                        ${load}"
logonly "Host CFLAGS:                         ${CFLAGS}"
logonly "Target CFLAGS:                       ${CFLAGS_FOR_TARGET}"
logonly "Extra config flags:                  ${config_extra}"


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

component_dirs="${infra_dir} ${component_dirs}"

regex="sdk\|parallella"
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
if [ "${do_clean}" = "--no-clean" ]
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
	do_clean="--clean"
    fi
fi

# Clean up old build directories if specified.
if [ "${do_clean}" = "--clean" ]
then
    rm -rf "${bd_host}"
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

# Configure the entire tool chain, but only if we are doing a clean build

# @todo Should we enable Python support in GDB? If so do we need to check
#       Python is available?
if [ ${do_clean} = "--clean" ]
then
    logterm "Configuring pal..."
    export CFLAGS_FOR_TARGET
    if ! (cd ${srcdir} && ./bootstrap >> "${logfile}" 2>&1)
    then
	logterm "ERROR: Failed bootstrapping source."
	failedbuild
    fi

    # Make toolchain host headers and libraries available in build
    if ! "${srcdir}/configure" \
	CFLAGS="-I${destdir}/${id_host}/include -I${destdir}${id_gnu}/include $CFLAGS" \
	LDFLAGS="-L${destdir}/${id_host}/lib -L${destdir}/${id_gnu}/lib $LDFLAGS" \
	${host_str} \
	--disable-benchmark \
	--disable-examples \
	--disable-tests \
	--enable-fast-install \
	--prefix="${id_host}" \
	--with-target="${target}" \
	"${id_target_str}" \
	${config_extra} >> "${logfile}" 2>&1
    then
	logterm "ERROR: pal configuration for host machine failed."
	failedbuild
    fi
fi

# Build everything
logterm "Building pal..."
if ! make ${parallel} all >> "${logfile}" 2>&1
then
  logterm "ERROR: pal build failed."
  failedbuild
fi

# Install everything
logterm "Installing pal..."
if ! make ${destdir_str} install >> "${logfile}" 2>&1
then
  logterm "Error: pal installation failed."
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
logterm "Creating symbolic links for pal"

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

logterm "pal installed at ${report_dir}/bin"
logterm "Ensure these directories are in your PATH"

logterm "BUILD COMPLETE: $(date)"
