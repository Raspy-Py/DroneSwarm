#!/bin/bash

if [ -f /opt/vision-sdk/environment-setup ]; then
    source /opt/vision-sdk/environment-setup
fi

# Configure toolchain
mkdir -p /root/.local
mkdir -p /root/.local/share
mkdir -p /root/.local/share/CMakeTools

file_path="/root/.local/share/CMakeTools/cmake-tools-kits.json"
> $file_path
cat <<EOL > $file_path
{
    "name": "Vision compiler",
    "toolchainFile": "/opt/vision-sdk/share/buildroot/toolchainfile.cmake",
    "isTrusted": true
}
EOL
