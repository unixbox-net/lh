#!/bin/bash

# Define the repository URL and clone directory
REPO_URL="https://github.com/unixbox-net/loghog.git"
CLONE_DIR="lh"

# Clean up any previous attempts
echo "Cleaning up existing directories..."
rm -rf "$CLONE_DIR"
mkdir "$CLONE_DIR"

# Enter the directory
cd "$CLONE_DIR"

# Clone the repository
echo "Cloning the repository from $REPO_URL..."
git clone "$REPO_URL" .

# Check and install required build dependencies
echo "Checking for required packages..."
REQUIRED_PACKAGES="gcc make rpm-build readline-devel json-c-devel"
for package in $REQUIRED_PACKAGES; do
    if ! rpm -q $package >/dev/null 2>&1; then
        echo "Installing $package..."
        sudo dnf install -y $package
    fi
done

# Compile the program
echo "Compiling the program..."
make || { echo "Compilation failed."; exit 1; }

# Prepare the sources for RPM build
echo "Preparing source tarball for RPM build..."
mkdir -p SOURCES
tar czf SOURCES/${CLONE_DIR}-1.0.0.tar.gz src

# Build the RPM
echo "Building the RPM package..."
rpmbuild -ba specs/lh.spec --define "_topdir $(pwd)" --define "_builddir $(pwd)/BUILD" --define "_rpmdir $(pwd)/RPMS" --define "_srcrpmdir $(pwd)/SRPMS" --define "_specdir $(pwd)/SPECS" --define "_sourcedir $(pwd)/SOURCES" || { echo "RPM build failed."; exit 1; }

# Check if RPM was created
echo "Checking for the RPM package..."
if ls RPMS/x86_64/*.rpm 1> /dev/null 2>&1; then
    echo "RPM package created successfully."
    echo "Installing the RPM package..."
    sudo dnf install -y RPMS/x86_64/*.rpm
else
    echo "No RPM package found. Exiting."
    exit 1
fi

echo "Installation complete."
