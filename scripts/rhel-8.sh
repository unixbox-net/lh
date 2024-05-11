#!/bin/bash

# Determine the current working directory
BASE_DIR=$(pwd)
BUILD_DIR="$BASE_DIR/lh"

echo "Cleaning up existing directories..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

echo "Cloning the repository into the build directory..."
git clone https://github.com/unixbox-net/lh.git "$BUILD_DIR"

cd "$BUILD_DIR"

echo "Checking and installing required packages..."
sudo dnf install -y gcc make rpm-build readline-devel json-c-devel git

echo "Compiling the source code..."
gcc -Wall -Wextra -std=c99 -g src/lh.c -o lh -lreadline -ljson-c

echo "Creating tarball for RPM build..."
tar czf SOURCES/lh-1.0.0.tar.gz --transform 's,^,lh-1.0.0/,' src/

echo "Setting up RPM build environment..."
mkdir -p BUILD RPMS SOURCES SPECS SRPMS
cp specs/lh.spec SPECS/

echo "Building the RPM package..."
rpmbuild -ba SPECS/lh.spec --define "_topdir $BUILD_DIR" --verbose

echo "Build process completed. Check the logs and RPMS directory for output and packages."
