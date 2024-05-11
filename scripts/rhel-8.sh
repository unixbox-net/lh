#!/bin/bash

# Define the repository URL
REPO_URL="https://github.com/unixbox-net/loghog.git"

# Define a specific build directory and ensure it's clean
BUILD_DIR="$HOME/lh-build"
rm -rf "$BUILD_DIR"  # Clear any previous build attempts
mkdir -p "$BUILD_DIR"

# Enter the build directory
cd "$BUILD_DIR"

# Clone the repository into a specific subdirectory
echo "Fetching files from repository..."
git clone "$REPO_URL" loghog || { echo "Failed to clone repository. Exiting."; exit 1; }

# Navigate into the repository
cd loghog

# Check and install required packages
REQUIRED_PACKAGES="gcc make rpm-build readline-devel json-c-devel"
echo "Checking for required packages..."
for package in $REQUIRED_PACKAGES; do
    if ! rpm -q $package >/dev/null 2>&1; then
        echo "Installing $package..."
        sudo dnf install -y $package
    fi
done

# Compile the program using the Makefile
echo "Compiling the program..."
make || { echo "Compilation failed. Exiting."; exit 1; }

# Ensure the rpmbuild directory structure is correct
mkdir -p {BUILD,RPMS,SOURCES,SPECS,SRPMS}

# Use cp to handle lh.spec and sources
cp specs/lh.spec SPECS/
cp src/lh.c SOURCES/

# Build the RPM package using the spec file
echo "Building the RPM package..."
rpmbuild -ba SPECS/lh.spec --define "_topdir $PWD" --define "_builddir $PWD/BUILD" --define "_rpmdir $PWD/RPMS" --define "_srcrpmdir $PWD/SRPMS" --define "_specdir $PWD/SPECS" --define "_sourcedir $PWD/SOURCES" || { echo "RPM build failed. Exiting."; exit 1; }

# Optional: Move the RPM packages to a more accessible location
mkdir -p "$HOME/lh-rpms"
mv RPMS/*/*.rpm "$HOME/lh-rpms/"

echo "RPM packages moved to $HOME/lh-rpms/"

# Cleanup
echo "Cleaning up..."
cd "$HOME"
rm -rf "$BUILD_DIR"

echo "Build completed successfully."
