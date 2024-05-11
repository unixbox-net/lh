#!/bin/bash

# Define the repository URL and the directory for the build
REPO_URL="https://github.com/unixbox-net/lh.git"
BUILD_DIR="/root/lh"

# Cleanup existing directory
echo "Cleaning up existing directories..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Clone the repository
echo "Cloning the repository into $BUILD_DIR..."
git clone "$REPO_URL" "$BUILD_DIR" || { echo "Failed to clone repository. Exiting."; exit 1; }

# Navigate to the build directory
cd "$BUILD_DIR" || { echo "Failed to navigate to build directory. Exiting."; exit 1; }

# Checking and installing required packages
echo "Checking and installing required packages..."
REQUIRED_PACKAGES="gcc make rpm-build readline-devel json-c-devel git"
sudo dnf install -y $REQUIRED_PACKAGES

# Compile the program using Makefile
echo "Compiling the source code..."
make || { echo "Compilation failed. Exiting."; exit 1; }

# Setup for RPM build
echo "Setting up RPM build environment..."
mkdir -p {BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
cp src/lh.c BUILD/ || { echo "Failed to copy source files. Exiting."; exit 1; }
tar czf SOURCES/lh-1.0.0.tar.gz -C BUILD lh.c || { echo "Failed to create source tarball. Exiting."; exit 1; }

# Ensure the spec file exists and copy it to the SPECS directory
if [ -f specs/lh.spec ]; then
    cp specs/lh.spec SPECS/
else
    echo "Spec file not found. Exiting."
    exit 1
fi

# Build the RPM package
echo "Building the RPM package..."
rpmbuild --define "_topdir $PWD" -ba SPECS/lh.spec || { echo "RPM build failed. Exiting."; exit 1; }

# Find and install the RPM package
RPM_FILE=$(find RPMS/ -type f -name '*.rpm')
if [[ -f "$RPM_FILE" ]]; then
    echo "Installing RPM package from $RPM_FILE..."
    sudo dnf reinstall "$RPM_FILE" -y
else
    echo "No RPM package found. Exiting."
    exit 1
fi

echo "Build and installation completed successfully."
