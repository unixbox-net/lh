#!/bin/bash

# Define the repository URL and base directory
REPO_URL="https://github.com/unixbox-net/lh.git"
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

# Prepare for RPM build
echo "Setting up RPM build environment..."
mkdir -p {BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
cp src/lh.c BUILD/
tar czf SOURCES/lh-1.0.0.tar.gz -C BUILD lh.c
cp lh.spec SPECS/

# Build the RPM package
echo "Building the RPM package..."
rpmbuild --define "_topdir $PWD" -ba SPECS/lh.spec

# Find and reinstall the RPM package
RPM_FILE=$(find RPMS/ -name '*.rpm')
if [[ -f "$RPM_FILE" ]]; then
    echo "RPM package created: $RPM_FILE"
    sudo dnf reinstall "$RPM_FILE" -y
else
    echo "No RPM package found. Exiting."
    exit 1
fi

echo "Installation and cleanup complete."
