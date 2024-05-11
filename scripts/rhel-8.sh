#!/bin/bash

# Set up the working directory
WORK_DIR="${PWD}/lh"
REPO_URL="https://github.com/unixbox-net/loghog.git"

# Clean up any existing work directory and recreate it
echo "Cleaning up the existing directories..."
rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR"
cd "$WORK_DIR"

# Clone the repository
echo "Cloning the repository..."
git clone "$REPO_URL" .

# Install required packages
REQUIRED_PACKAGES="gcc make rpm-build readline-devel json-c-devel git"
echo "Checking and installing required packages..."
sudo dnf install -y $REQUIRED_PACKAGES

# Compilation step
echo "Compiling the source code..."
make || { echo "Compilation failed. Exiting."; exit 1; }

# Prepare the directory structure for RPM build
echo "Setting up RPM build environment..."
mkdir -p {BUILD,RPMS,SOURCES,SPECS,SRPMS}

# Copy spec file and source to appropriate directories
cp specs/lh.spec SPECS/
tar czf SOURCES/lh-1.0.0.tar.gz src

# Build RPM package
echo "Building the RPM package..."
rpmbuild --define "_topdir $PWD" -ba SPECS/lh.spec || { echo "RPM build failed. Exiting."; exit 1; }

# Locate RPM package
RPM_FILE=$(find RPMS/ -name '*.rpm')
echo "RPM package built at: $RPM_FILE"

# Install the RPM package
echo "Installing the RPM package..."
sudo dnf reinstall -y "$RPM_FILE" || sudo dnf install -y "$RPM_FILE"

# Clean up the build directories if needed
echo "Build completed successfully, cleaning up..."
cd ..
rm -rf "$WORK_DIR"

echo "Installation complete. lh has been installed."
