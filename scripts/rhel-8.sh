#!/bin/bash

# Set up base directory and capture current working directory
BASE_DIR=$(pwd)
BUILD_DIR="${BASE_DIR}/lh"

# Ensure directory exists and is clean
echo "Setting up build environment..."
mkdir -p "$BUILD_DIR"
rm -rf "${BUILD_DIR:?}/*"

# Clone the repository
echo "Cloning the repository into the build directory..."
git clone "https://github.com/unixbox-net/lh.git" "$BUILD_DIR"
cd "$BUILD_DIR" || exit

# Install required packages
echo "Installing required packages..."
sudo dnf install -y gcc make rpm-build readline-devel json-c-devel git

# Compile the source code
echo "Compiling the source code..."
gcc -Wall -Wextra -std=c99 -g src/lh.c -o lh -lreadline -ljson-c

# Create directories for RPM build
echo "Creating directories for RPM build..."
mkdir -p {BUILD,BUILDROOT,RPMS,SPECS,SRPMS,SOURCES}

# Create the tarball including all necessary files
echo "Creating tarball for RPM build..."
find . -type f ! -name '*.tar.gz' -print0 | tar --null -czf SOURCES/lh-1.0.0.tar.gz --no-recursion --transform 's,^,lh-1.0.0/,' -T -

# Prepare the spec file (ensure it's properly formatted and in the right location)
echo "Preparing spec file..."
cp specs/lh.spec SPECS/

# Build the RPM
echo "Building the RPM package..."
rpmbuild -ba SPECS/lh.spec \
  --define "_topdir $BUILD_DIR" \
  --define "_builddir $BUILD_DIR/BUILD" \
  --define "_rpmdir $BUILD_DIR/RPMS" \
  --define "_srcrpmdir $BUILD_DIR/SRPMS" \
  --define "_specdir $BUILD_DIR/SPECS" \
  --define "_sourcedir $BUILD_DIR/SOURCES"

# Check if RPM build succeeded
if [ $? -eq 0 ]; then
    echo "RPM build completed successfully. Check $BUILD_DIR/RPMS for the output."
else
    echo "RPM build failed. Please check the logs for details."
    exit 1
fi

