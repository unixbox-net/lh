#!/bin/bash

# Define the repository URL
REPO_URL="https://github.com/unixbox-net/loghog.git"

# Define the base directory
BASE_DIR="$PWD/lh-build"

# Create and enter the base directory
mkdir -p "$BASE_DIR"
cd "$BASE_DIR"

# Fetch the necessary files from the repository
echo "Fetching files from repository..."
git clone "$REPO_URL" . || { echo "Failed to clone repository. Exiting."; exit 1; }

# Check if required packages are installed and install them if they are not
REQUIRED_PACKAGES="gcc make rpm-build"
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

# Build the RPM package using the spec file
echo "Building the RPM package..."
rpmbuild -ba lh.spec --define "_topdir $PWD" --define "_builddir $PWD" --define "_rpmdir $PWD/RPMS" --define "_srcrpmdir $PWD/SRPMS" --define "_specdir $PWD" --define "_sourcedir $PWD" || { echo "RPM build failed. Exiting."; exit 1; }

# Move the RPM packages to a more accessible location
mkdir -p "$HOME/lh-rpms"
mv RPMS/*/*.rpm "$HOME/lh-rpms/"

echo "RPM packages moved to $HOME/lh-rpms/"

# Cleanup
cd ..
rm -rf "$BASE_DIR"

echo "Build completed successfully."

