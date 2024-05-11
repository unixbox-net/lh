#!/bin/bash

# Set up base directory
BASE_DIR=$(pwd)/lh
REPO_URL="https://github.com/unixbox-net/lh.git"

# Clean up existing directories
echo "Cleaning up existing directories..."
rm -rf "$BASE_DIR"

# Clone the repository
echo "Cloning the repository into the build directory..."
git clone "$REPO_URL" "$BASE_DIR"
if [ $? -ne 0 ]; then
    echo "Failed to clone repository."
    exit 1
fi

# Check and install required packages
echo "Checking and installing required packages..."
sudo dnf install -y gcc make rpm-build readline-devel json-c-devel

# Compile the source code
echo "Compiling the source code..."
gcc -Wall -Wextra -std=c99 -g "$BASE_DIR/src/lh.c" -o "$BASE_DIR/lh" -lreadline -ljson-c
if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi

# Creating necessary build directories
echo "Creating necessary directories for the build process..."
mkdir -p "$BASE_DIR/BUILD" "$BASE_DIR/BUILDROOT" "$BASE_DIR/RPMS" "$BASE_DIR/SPECS" "$BASE_DIR/SRPMS" "$BASE_DIR/SOURCES"

# Creating the tarball
echo "Creating tarball for RPM build..."
tar -czf "$BASE_DIR/SOURCES/lh-1.0.0.tar.gz" --transform 's,^,lh-1.0.0/,' -C "$BASE_DIR/src" .
if [ $? -ne 0 ]; then
    echo "Failed to create tarball"
    exit 1
fi
echo "Tarball created successfully."

# Prepare the spec file
echo "Preparing the spec file..."
cp "$BASE_DIR/specs/lh.spec" "$BASE_DIR/SPECS"

# Build the RPM
echo "Building the RPM package..."
rpmbuild -ba "$BASE_DIR/SPECS/lh.spec" \
  --define "_topdir $BASE_DIR" \
  --define "_builddir $BASE_DIR/BUILD" \
  --define "_rpmdir $BASE_DIR/RPMS" \
  --define "_srcrpmdir $BASE_DIR/SRPMS" \
  --define "_specdir $BASE_DIR/SPECS" \
  --define "_sourcedir $BASE_DIR/SOURCES"

if [ $? -ne 0 ]; then
    echo "RPM build failed. Please check the logs for details."
    exit 1
else
    echo "RPM build completed successfully. Check $BASE_DIR/RPMS for the output."
fi
