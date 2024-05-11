#!/bin/bash

# Define the repository URL and clone directory
REPO_URL="https://github.com/unixbox-net/loghog.git"
CLONE_DIR="lh"

# Clean up any previous attempts
rm -rf "$CLONE_DIR"
mkdir "$CLONE_DIR"

# Enter the directory
cd "$CLONE_DIR"

# Clone the repository
echo "Cloning the repository..."
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

# Build the RPM
echo "Building the RPM package..."
rpmbuild -ba specs/lh.spec --define "_topdir $(pwd)" --define "_builddir $(pwd)/BUILD" --define "_rpmdir $(pwd)/RPMS" --define "_srcrpmdir $(pwd)/SRPMS" --define "_specdir $(pwd)/SPECS" --define "_sourcedir $(pwd)/SOURCES" || { echo "RPM build failed."; exit 1; }

# Navigate to the RPM directory
cd RPMS/x86_64

# Install the RPM package
RPM_PACKAGE=$(ls *.rpm)
if [ -f "$RPM_PACKAGE" ]; then
    echo "Installing/reinstalling the RPM package..."
    sudo dnf reinstall "$RPM_PACKAGE" -y || sudo dnf install "$RPM_PACKAGE" -y
else
    echo "No RPM package found. Exiting."
    exit 1
fi

echo "Installation complete. Cleaning up..."
cd ../../..
rm -rf BUILD RPMS SRPMS BUILDROOT

echo "Process completed. Your RPM is in the '$(pwd)/RPMS/x86_64' directory."
