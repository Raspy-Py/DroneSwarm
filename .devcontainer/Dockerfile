FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

# Update the package list and upgrade installed packages
RUN apt-get update && apt-get upgrade -y

# Install some common utilities (optional)
RUN apt-get install -y \
    curl \
    wget \
    nano

# Install vision-rv1126 dependencies
RUN apt-get install -y \
    bison texinfo flex cmake \
    libssl-dev liblz4-tool patchelf \
    chrpath gawk expect-dev python2 python \
    git git-lfs expect-dev debianutils sed \
    make binutils build-essential gcc g++ \
    bash patch gzip bzip2 perl tar cpio \
    unzip rsync file bc git device-tree-compiler

# rkdeveloptool dependencies
RUN apt-get install -y \
    libudev-dev libusb-1.0-0-dev dh-autoreconf \
    pkg-config libusb-1.0

# Set the working directory in the container
RUN mkdir /root/workdir
WORKDIR /root/workdir

# Command to run when the container starts
CMD ["/bin/bash"]