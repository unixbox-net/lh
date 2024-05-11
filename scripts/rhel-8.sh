#!/bin/bash

# Set the base directory to the current working directory
export BASE_DIR=$(pwd)

echo "Setting up the build environment in $BASE_DIR"

# Create necessary directories for the build process
echo "Creating necessary directories for the build process..."
mkdir -p "$BASE_DIR/BUILD" "$BASE_DIR/BUILDROOT" "$BASE_DIR/RPMS" "$BASE_DIR/SPECS" "$BASE_DIR/SRPMS" "$BASE_DIR/SOURCES"

# Cloning the repository into the specified base directory
echo "Cloning the repository..."
git clone https://github.com/unixbox-net/lh.git "$BASE_DIR/lh"

# Navigate into the cloned directory to perform operations
cd "$BASE_DIR/lh" || exit

# Create a tarball for the RPM build
echo "Creating tarball for RPM build..."
tar -czf "$BASE_DIR/SOURCES/lh-1.0.0.tar.gz" -C "$BASE_DIR/lh/src" .

if [ $? -ne 0 ]; then
    echo "Failed to create tarball"
    exit 1
fi
echo "Tarball created successfully at $BASE_DIR/SOURCES/lh-1.0.0.tar.gz"

# Prepare RPM build environment variables
echo "Setting up RPM build environment..."
cp "$BASE_DIR/lh/specs/lh.spec" "$BASE_DIR/SPECS"

# Build the RPM
echo "Building the RPM package..."
rpmbuild -ba "$BASE_DIR/SPECS/lh.spec" --define "_topdir $BASE_DIR" --define "_builddir $BASE_DIR/BUILD" --define "_rpmdir $BASE_DIR/RPMS" --define "_srcrpmdir $BASE_DIR/SRPMS" --define "_specdir $BASE_DIR/SPECS" --define "_sourcedir $BASE_DIR/SOURCES"

# Check if RPM build was successful
if [ $? -ne 0 ]; then
    echo "RPM build failed. Please check the logs for details."
    exit 1
else
    echo "RPM build completed successfully. Check $BASE_DIR/RPMS for the output."
fi
