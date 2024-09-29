#!/bin/bash

echo "PATH: $PATH"
echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"
echo "PKG_CONFIG_PATH: $PKG_CONFIG_PATH"
echo "Current directory: $(pwd)"

script_real_path=$(realpath "$0")
script_root_dir=$(dirname "$SCRIPT_REAL_PATH")

install_dir="$script_root_dir/bin"

# Default options
clean_build=0

# parse command line arguments
while getopts "hci" opt; do
  case ${opt} in
    c )
      clean_build=1
      ;;
    h )
      echo "Usage: build.sh [-c|i|h]"
      echo "  -c: clean build"
      exit 0
      ;;
    \? )
      echo "For help: build.sh -h"
      exit 1
      ;;
  esac
done

echo "[BUILD OPTIONS]:"
echo "  clean_build: $clean_build"
echo "  do_install: $do_install"

# Clean build directory
if [ $clean_build -eq 1 ]; then
  echo "[INFO]: Cleaning build directory."
  rm -rf build
fi

# Standard CMake build
rm -rf $install_dir
mkdir -p $install_dir
mkdir -p build
cd build
cmake -DCMAKE_INSTALL_PREFIX=$install_dir \
      -DCMAKE_PREFIX_PATH=/opt/vision-sdk/arm-buildroot-linux-gnueabihf/sysroot/usr \
      ..
make install
