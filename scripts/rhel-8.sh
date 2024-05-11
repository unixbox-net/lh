#!/bin/bash

# Clean up and prepare environment
echo "Cleaning up existing directories..."
rm -rf "$PWD/lh"
mkdir -p "$PWD/lh"
cd "$PWD/lh"

# Clone the repository
echo "Cloning the repository..."
git clone https://github.com/unixbox-net/lh.git .
if [ $? -ne 0 ]; then
    echo "Failed to clone repository. Exiting."
    exit 1
fi

# Check and install required packages
echo "Checking and installing required packages..."
sudo dnf install -y gcc make rpm-build readline-devel json-c-devel git

# Verbose compile the source code
echo "Compiling the source code..."
gcc -Wall -Wextra -std=c99 -g src/lh.c -o lh -lreadline -ljson-c 2>&1 | tee compile.log

# Setup RPM build environment
echo "Setting up RPM build environment..."
mkdir -p {BUILD,RPMS,SOURCES,SPECS,SRPMS}
cp specs/lh.spec SPECS/
cp src/lh.c SOURCES/

# Creating tarball for RPM build
echo "Creating tarball for RPM build..."
tar czf SOURCES/lh-1.0.0.tar.gz -C src lh.c

# Build the RPM package with verbose output
echo "Building the RPM package..."
rpmbuild -ba SPECS/lh.spec --define "_topdir $PWD" 2>&1 | tee build.log

echo "Build process completed. Check the logs and RPMS directory for output and packages."
