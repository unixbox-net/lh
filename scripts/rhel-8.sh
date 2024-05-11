#!/bin/bash

# Define variables
REPO_URL="https://github.com/unixbox-net/loghog.git"
BASE_DIR="$PWD/lh"

# Cleanup existing directory
echo "Cleaning up existing directories..."
rm -rf "$BASE_DIR"
mkdir -p "$BASE_DIR"

# Clone the repository
echo "Cloning the repository..."
git clone "$REPO_URL" "$BASE_DIR" || { echo "Failed to clone repository. Exiting."; exit 1; }

# Move into the base directory
cd "$BASE_DIR"

# Check and install required packages
REQUIRED_PACKAGES="gcc make rpm-build readline-devel json-c-devel git"
echo "Checking and installing required packages..."
sudo dnf install -y $REQUIRED_PACKAGES

# Compile the program using the Makefile
echo "Compiling the source code..."
make || { echo "Compilation failed. Exiting."; exit 1; }

# Setup RPM build environment
echo "Setting up RPM build environment..."
mkdir -p {BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
cp src/lh.c BUILD/
cp lh.spec SPECS/

# Create tarball of the source
tar czf SOURCES/lh-1.0.0.tar.gz -C BUILD lh.c

# Build the RPM package
echo "Building the RPM package..."
rpmbuild --define "_topdir $PWD" --define "_builddir $PWD/BUILD" --define "_rpmdir $PWD/RPMS" --define "_srcrpmdir $PWD/SRPMS" --define "_specdir $PWD/SPECS" --define "_sourcedir $PWD/SOURCES" -ba SPECS/lh.spec || { echo "RPM build failed. Exiting."; exit 1; }

# Find the RPM package
RPM_FILE=$(find RPMS -type f -name '*.rpm')
if [ -z "$RPM_FILE" ]; then
    echo "No RPM package found. Exiting."
    exit 1
fi

# Install the RPM package using DNF
echo "Installing the RPM package..."
sudo dnf install -y "$RPM_FILE"

# Cleanup and leave RPM for future use
echo "RPM package installed successfully. Cleanup not required to keep RPM."

# List final directory structure
tree "$BASE_DIR"
