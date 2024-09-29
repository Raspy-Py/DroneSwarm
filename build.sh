#!/bin/bash


install_dir="../bin"

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
cmake -DCMAKE_INSTALL_PREFIX=$install_dir ..
make install
