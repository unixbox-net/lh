#!/bin/bash

# Determine the base directory for all operations
BASE_DIR=$(pwd)/lh

# Clean up existing directories
echo "Cleaning up existing directories..."
rm -rf "$BASE_DIR"
mkdir -p "$BASE_DIR"
cd "$BASE_DIR"

# Clone the repository
echo "Cloning the repository into the build directory..."
git clone https://github.com/unixbox-net/lh.git "$BASE_DIR"
if [ $? -ne 0 ]; then
    echo "Failed to clone repository."
    exit 1
fi

# Create necessary directories for the RPM build
echo "Creating necessary directories for the build process..."
mkdir -p "$BASE_DIR/BUILD" "$BASE_DIR/BUILDROOT" "$BASE_DIR/RPMS" "$BASE_DIR/SPECS" "$BASE_DIR/SRPMS" "$BASE_DIR/SOURCES"
if [ $? -ne 0 ]; then
    echo "Failed to create build directories."
    exit 1
fi
# Checking and installing required packages
echo "Checking and installing required packages..."
sudo dnf install -y gcc make rpm-build readline-devel json-c-devel git
if [ $? -ne 0 ]; then
    echo "Failed to install required packages."
    exit 1
fi

# Compile the source code
echo "Compiling the source code..."
gcc -Wall -Wextra -std=c99 -g src/lh.c -o lh -lreadline -ljson-c
if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi

# Creating the tarball
echo "Creating tarball for RPM build..."
tar -czf "$BASE_DIR/SOURCES/lh-1.0.0.tar.gz" -C "$BASE_DIR" .
if [ $? -ne 0 ]; then
    echo "Failed to create tarball."
    exit 1
fi
echo "Tarball created successfully."

# Prepare RPM build environment
echo "%_topdir $BASE_DIR" > ~/.rpmmacros
cp specs/lh.spec "$BASE_DIR/SPECS"

# Build the RPM
echo "Building the RPM package..."
rpmbuild -ba "$BASE_DIR/SPECS/lh.spec"
if [ $? -ne 0 ]; then
    echo "RPM build failed."
    exit 1
fi

echo "Build process completed. Check the logs and RPMS directory for output and packages."
