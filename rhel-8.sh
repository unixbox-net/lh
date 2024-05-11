#!/bin/bash

# Setup logfile
LOGFILE="/tmp/lh.log"
exec > >(tee "$LOGFILE") 2>&1

echo "Starting the build process..."

# Define main directory based on the current script location
MAIN_DIR=$(pwd)
BUILD_DIR="$MAIN_DIR/build"
RPMBUILD_DIR="$BUILD_DIR/rpmbuild"
SOURCES_DIR="$RPMBUILD_DIR/SOURCES"
SPECS_DIR="$RPMBUILD_DIR/SPECS"
BUILDROOT_DIR="$RPMBUILD_DIR/BUILDROOT"

# Clean old build directories if they exist
echo "Cleaning old build directories..."
rm -rf "$BUILD_DIR"

# Create necessary directories
echo "Creating build directories..."
mkdir -p "$BUILD_DIR" "$RPMBUILD_DIR" "$SOURCES_DIR" "$SPECS_DIR" "$BUILDROOT_DIR"

# Check for required packages and install if missing
echo "Checking and installing required packages..."
sudo dnf install -y gcc make rpm-build readline-devel json-c-devel git

# Prepare the source directory for RPM build
echo "Copying source files to $SOURCES_DIR..."
cp -a * "$SOURCES_DIR" || { echo "Failed to copy source files."; exit 1; }

echo "Creating source tarball..."
tar -czf "$SOURCES_DIR/lh-1.0.0.tar.gz" -C "$SOURCES_DIR" . || { echo "Failed to create source tarball."; exit 1; }

# Generate the RPM spec file
echo "Generating RPM spec file..."
cat > "$SPECS_DIR/lh.spec" <<EOF
Summary: Lightweight log handler
Name: lh
Version: 1.0.0
Release: 1%{?dist}
License: GPL
Source0: %{name}-%{version}.tar.gz

BuildRequires: gcc, make, readline-devel, json-c-devel

%description
lh is a tool for handling logs efficiently.

%prep
%setup -q

%build
gcc -Wall -o lh lh.c -lreadline -ljson-c

%install
mkdir -p %{buildroot}/usr/bin
install -m 755 lh %{buildroot}/usr/bin

%files
/usr/bin/lh

%changelog
* $(date +"%a %b %d %Y") - 1.0.0-1
- Initial release
EOF

# Build the RPM package
echo "Building the RPM package..."
rpmbuild -ba "$SPECS_DIR/lh.spec" --define "_topdir $RPMBUILD_DIR" || { echo "RPM build failed."; exit 1; }

echo "Build and installation complete. RPM package and executable are located in:"
echo "$RPMBUILD_DIR/RPMS/x86_64/"

echo "Complete log of this run can be found in $LOGFILE"
