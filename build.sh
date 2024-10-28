#!/bin/bash

script_real_path=$(realpath "$0")
script_root_dir=$(dirname "$script_real_path")
install_dir="$script_root_dir/bin"

# Default options
clean_build=0

# parse command line arguments
while getopts "hc" opt; do
  case ${opt} in
    c )
      clean_build=1
      ;;
    h )
      echo "Usage: build.sh [-c|h]"
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
