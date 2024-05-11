#!/bin/bash

# Base setup
REPO_URL="https://github.com/unixbox-net/loghog"
BASE_DIR="$PWD/lh-build"

# Create and enter the base directory
mkdir -p "$BASE_DIR"
cd "$BASE_DIR"

# Clone the repository
echo "Fetching files from repository..."
git clone "$REPO_URL" .

# Install necessary packages
REQUIRED_PACKAGES="gcc make rpm-build readline-devel json-c-devel"
echo "Checking for required packages..."
for package in $REQUIRED_PACKAGES; do
    if ! rpm -q $package >/dev/null 2>&1; then
        echo "Installing $package..."
        sudo dnf install -y $package
    fi
done

# Build the program
echo "Compiling the program..."
make || { echo "Compilation failed. Exiting."; exit 1; }

# Build the RPM package
echo "Building the RPM package..."
rpmbuild -ba specs/lh.spec --define "_topdir $PWD" --define "_builddir $PWD" --define "_rpmdir $PWD/RPMS" --define "_srcrpmdir $PWD/SRPMS" --define "_specdir $PWD/specs" --define "_sourcedir $PWD" || { echo "RPM build failed. Exiting."; exit 1; }

# Clean up
echo "Cleaning up..."
cd ..
rm -rf "$BASE_DIR"

echo "Build completed successfully."
