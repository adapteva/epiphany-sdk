#!/usr/bin/env bash

# Host machine architecture
ARCH="armv7l"

# Revision number of Epiphany SDK
REV=${1:-"DevBuild"}

# Git branch name for build
BRANCH=${2:-"master"}

ESDK="${EPIPHANY_BUILD_HOME}/esdk.${REV}"
HOSTNAME="host.${ARCH}"
HOST="${ESDK}/tools/${HOSTNAME}"
GNUNAME="e-gnu.${ARCH}"
GNU="${ESDK}/tools/${GNUNAME}"
EPIPHANY_HOME=$ESDK

echo '********************************************'
echo '************ Epiphany SDK Build ************'
echo '********************************************'

echo ""
echo "Environment settings:"
echo ""
echo "    EPIPHANY_BUILD_HOME=$EPIPHANY_BUILD_HOME"
echo "    EPIPHANY_HOME=$EPIPHANY_HOME"
echo "    ESDK=$ESDK"
echo ""

if [ -z $EPIPHANY_BUILD_HOME ]; then
	echo "Please define the environment variable EPIPHANY_BUILD_HOME and "
	echo "rerun the build-epiphany-sdk.sh script."
	exit 1
fi

cd $EPIPHANY_BUILD_HOME

## Get the toolchain download script
if ! wget https://raw.github.com/adapteva/epiphany-sdk/master/download-toolchain.sh; then
	printf "Failed to get the download-toolchain script from "
	printf "https://raw.github.com/adapteva/epiphany-sdk/master/download-toolchain.sh\n"
	printf "\nAborting...\n"
	exit 1
fi


# Clone the github repositories
if ! bash ./download-toolchain.sh --clone; then
	printf "Failed to get the download-toolchain script from "
	printf "https://raw.github.com/adapteva/epiphany-sdk/master/download-toolchain.sh\n"
	printf "\nAborting...\n"
	exit 1
fi

# Get the additional GCC modules that we need
pushd gcc >& /dev/null

if ! wget http://multiprecision.org/mpc/download/mpc-1.0.1.tar.gz; then
		printf "Failed to get the mpc package from "
		printf "http://multiprecision.org/mpc/download/mpc-1.0.1.tar.gz\n"
		printf "\nAborting...\n"
		exit 1
fi

if ! wget http://www.mpfr.org/mpfr-current/mpfr-3.1.2.tar.xz; then
		printf "Failed to get the mpfr package from "
		printf "wget http://www.mpfr.org/mpfr-current/mpfr-3.1.2.tar.xz\n"
		printf "\nAborting...\n"
		exit 1
fi

if ! wget ftp://ftp.gmplib.org/pub/gmp-5.1.2/gmp-5.1.2.tar.lz; then
		printf "Failed to get the gmp package from "
		printf "\nftp://ftp.gmplib.org/pub/gmp-5.1.2/gmp-5.1.2.tar.lz"
		printf "\nAborting...\n"
		exit 1
fi

##
## Untar the GCC modules
##

if ! tar xf mpc-1.0.1.tar.gz; then
		printf "Failed to untar the mpc package. Please ensure that\n"
		printf "the gzip package is installed on the host\n"
		printf "\nAborting...\n"
		exit 1
fi

if ! tar xf mpfr-3.1.2.tar.xz; then
		printf "Failed to untar the mpfr package. Please ensure that\n"
		printf "the xzip package is installed on the host\n"
		printf "\nAborting...\n"
		exit 1
fi

if ! tar xf gmp-5.1.2.tar.lz; then
		printf "Failed to untar the gmp package. Please ensure that\n"
		printf "the lzip package is installed on the host\n"
		printf "\nAborting...\n"
		exit 1
fi

# Symlink the modules (the build expects them without version numbers)
ln -sf ./mpc-1.0.1 ./mpc
ln -sf ./mpfr-3.1.2 ./mpfr
ln -sf ./gmp-5.1.2 ./gmp
popd >& /dev/null

pushd sdk >& /dev/null

#Build the toolchain (this will take a while)
if ! ./build-toolchain.sh; then
		printf "The toolchain build failed!\n"
		printf "\nAborting...\n"
		exit 1
fi

if [ ! -d ../esdk/tools/${GNUNAME}/ ]; then
	# Create the SDK tree and set default symlinks
	# Note the install-sdk.sh script will attempt to create
	# this toolchain tree but it does not Install the
	# toolchain!!
	echo "Creating the eSDK directory tree..."

	mkdir -p ${ESDK}
	ln -sT "esdk.${REV}" ${EPIPHANY_BUILD_HOME}/esdk
	mkdir -p ${ESDK}/bsps
	mkdir -p ${ESDK}/tools

	mkdir -p ${HOST}
	mkdir -p ${GNU}
	ln -s ${HOSTNAME} ${ESDK}/tools/host
	ln -s ${GNUNAME}  ${ESDK}/tools/e-gnu
	
	mkdir -p ${HOST}/lib
	mkdir -p ${HOST}/include
	mkdir -p ${HOST}/bin
fi

# Copy the toolchain
echo "Copying the toolchain into ../esdk/tools/${GNUNAME}"
mv ../INSTALL/* ../esdk/tools/${GNUNAME}/

# Clone the parallella Linux source tree
if ! git clone https://github.com/parallella/parallella-linux-adi.git; then
	printf "The Epiphany SDK build failed!\n"
	printf "\nAborting...\n"
fi

export PARALLELLA_LINUX_HOME=$PWD/parallella-linux-adi

# build the epiphany-libs and install the SDK
if ! ./install-sdk.sh -n $REV -x $BRANCH; then
	printf "The Epiphany SDK build failed!\n"
	printf "\nAborting...\n"
	exit 1
fi

popd >& /dev/null

printf "The Epiphany SDK Build Completed successfully\n"
exit 0

