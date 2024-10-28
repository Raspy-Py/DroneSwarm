#!/bin/bash

script_real_path=$(realpath "$0")
script_root_dir=$(dirname "$script_real_path")
install_dir="$script_root_dir/bin"

scp $install_dir/* root@192.168.55.1:/tmp
