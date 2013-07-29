#!/bin/sh

# Copyright (C) 2009 - 2013 Embecosm Limited

# Contributor Joern Rennecke <joern.rennecke@embecosm.com>
# Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>
# Contributor Simon Cook <simon.cook@embecosm.com>

# This file is a script to build key elements of the Epiphany tool chain

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

# Usage: ./build-toolchain.sh [ --build-dir <build_directory>]
#                 [--enable-cgen-maint]
#                 [ --unified-dir <unified_directory> ]
#                 [ --install-dir <install_directory> ]
#                 [ --preserve-unisrc ]
# --build-dir: specify name for build directory.
# --enable-cgen-maint: pass down to configure, for rebuilding
#                    opcodes / binutils after a change to the cgen description.
# --unified-dir: specify name for unified src directory.
# --install-dir: specify name for install directory.
# --preserve-unisrc: don't delete old unisrc directories.

# Check for relative directory and makes it absolute
absolutedir() {
  case ${1} in
    /*) echo "${1}" ;;
    *)  echo "${PWD}/${1}";;
  esac
}

# Find the absolute path of our directory
basedir=`dirname "$0"`
basedir=`(cd "${basedir}/.." && pwd)`

component_dirs="${basedir}/gcc ${basedir}/binutils ${basedir}/gdb ${basedir}/newlib ${basedir}/cgen"
unified_src="${basedir}/srcw"
build_dir="${basedir}/bld-epiphany"
install_dir="${basedir}/INSTALL"
preserveunisrc=0

# The assembler and/or linker are broken so that constant merging doesn't work.
export CFLAGS_FOR_TARGET='-O2 -g'


# Prints usage to terminal
usage() {
  echo " Usage: $0 [ --build-dir <build_directory>]"
  echo "                 [--enable-cgen-maint]"
  echo "                 [ --unified-dir <unified_directory> ]"
  echo "                 [ --install-dir <install_directory> ]"
  echo " --build-dir: specify name for build directory."
  echo " --enable-cgen-maint: pass down to configure, for rebuilding"
  echo "                    opcodes / binutils after a change to the cgen description."
  echo " --unified-dir: specify name for unified src directory."
  echo " --install-dir: specify name for install directory."
  echo " --preserve-unisrc: don't delete unisrc directories."
}

# Displays a message if the build fails
failedbuild() {
  echo " **************************************************"
  echo " The SDK build has failed."
  echo " The log for this build can be found at"
  echo " ${logfile}"
  echo " For support, visit the forums or documentation at:"
  echo "  * http://forums.parallella.org/viewforum.php?f=13"
  echo "  * https://github.com/adapteva/epiphany-sdk/wiki"
  echo " **************************************************"
}

# We choose the amount of parallelism to use depending on the architecture
case $(uname -m 2>&1) in
  x86_64|i?86)
    make_load="-j -l `(echo processor;cat /proc/cpuinfo 2>/dev/null || echo processor)|grep -c processor`" ;;
  armv*)
    make_load="" ;;
  ?*)
    make_load="-j2 -l2" ;;
esac

CONFIG_EXTRA_OPTS=""
# Parse Options
until
  opt=$1
  case ${opt} in
    --build-dir)
      build_dir=$(absolutedir "$2"); shift ;;
    --enable-cgen-maint)
      CONFIG_EXTRA_OPTS="$CONFIG_EXTRA_OPTS --enable-cgen-maint" ;;
    --unified-dir)
      unified_src=$(absolutedir "$2"); shift ;;
    --install-dir)
      install_dir=$(absolutedir "$2"); shift ;;
    --preserve-unisrc)
      preserveunisrc=1 ;;
    ?*)
      usage; exit 0 ;;
    *)
      opt="";;
  esac;
  [ -z "${opt}" ]; do 
    shift
done

# Clean up old builds
rm -rf "${build_dir}" "${install_dir}"

if test ${preserveunisrc} -eq 0; then
  rm -rf "${unified_src}"
fi

# Set up a log
logfile=${basedir}/build-$(date -u +%F-%H%M).log
rm -f "${logfile}"

echo "START BUILD: $(date)" >> ${logfile}
echo "Build Started at $(date)"
echo "Build Log: ${logfile}"
echo " * This can be watched in another terminal via 'tail -f ${logfile}'"
echo "Build Directory: ${build_dir}"
echo "Install Directory: ${install_dir}"

# Create unified source directory
echo "Creating unified source" >> ${logfile}
echo "=======================" >> ${logfile}
echo "Creating unified source..."
./symlink-all "${unified_src}" ${component_dirs} >> "${logfile}" 2>&1
if [ $? != 0 ]; then
  echo "Failed to create ${unified_src}."
  failedbuild
  exit 1
fi

# Configure binutils, GCC, newlib and GDB
echo "Configuring tools" >> "${logfile}"
echo "=================" >> "${logfile}"
echo "Configuring tools..."
mkdir -p "${build_dir}" && cd "${build_dir}" \
  && "${unified_src}/configure" --target=epiphany-elf \
    --with-pkgversion="Epiphany toolchain (built `date +%Y%m%d`)" \
    --with-bugurl=support-sdk@adapteva.com \
    --enable-fast-install=N/A \
    --enable-languages=c,c++ --prefix="${install_dir}" \
    --with-headers="$(absolutedir ../newlib/newlib/libc/include)" \
    --disable-gdbtk --disable-werror \
    $CONFIG_EXTRA_OPTS >> "${logfile}" 2>&1
if [ $? != 0 ]; then
  echo "Error: Configure failed."
  failedbuild
  exit 1
fi

# Build binutils, GCC, newlib and GDB
echo "Building tools" >> "${logfile}"
echo "==============" >> "${logfile}"
echo "Building tools..."
make $make_load all-build all-binutils all-gas all-ld all-gcc \
    all-target-libgcc all-target-libgloss all-target-newlib \
    all-target-libstdc++-v3 all-gdb all-sim >> "${logfile}" 2>&1
if [ $? != 0 ]; then
  echo "Error: Build failed."
  failedbuild
  exit 1
fi

# Install binutils, GCC, newlib and GDB
echo "Installing tools" >> "${logfile}"
echo "================" >> "${logfile}"
echo "Installing tools..."
make install-binutils install-gas install-ld install-gcc \
    install-target-libgcc install-target-libgloss install-target-newlib \
    install-target-libstdc++-v3 install-gdb install-sim >> "${logfile}" 2>&1
if [ $? != 0 ]; then
  echo "Error: Install failed."
  failedbuild
  exit 1
fi

#

# Create symbolic links in install directory for e-gcc, etc.
cd "${install_dir}/bin"
for i in epiphany-elf-*; do
  ENAME=$(echo $i | sed 's/epiphany-elf-/e-/')
  if ! [ -e $ENAME ]; then ln -s $i $ENAME; fi
done
cd "${install_dir}/share/man/man1"
for i in epiphany-elf-*; do
  ENAME=$(echo $i | sed 's/epiphany-elf-/e-/')
  if ! [ -e $ENAME ]; then ln -s $i $ENAME; fi
done

echo "BUILD COMPLETE: $(date)" >> "${logfile}"
echo "Build Complete at $(date)"
echo " **************************************************"
echo " The build is complete."
echo " The tools have been installed at: ${install_dir}/bin"
echo " Please ensure that this directory is in your PATH."
echo " **************************************************"
