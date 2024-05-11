#!/bin/bash

# Assuming this script is being run from the INSTALL_DIR which contains lh source

# Configuration variables
REPO_URL="https://github.com/unixbox-net/lh.git"
INSTALL_DIR="$HOME/lh"
BUILD_DIR="${INSTALL_DIR}/build"
RPMBUILD_DIR="${BUILD_DIR}/rpmbuild"
SOURCES_DIR="${RPMBUILD_DIR}/SOURCES"
SPECS_DIR="${RPMBUILD_DIR}/SPECS"
RPMS_DIR="${RPMBUILD_DIR}/RPMS"
SRPMS_DIR="${RPMBUILD_DIR}/SRPMS"
BUILDROOT_DIR="${RPMBUILD_DIR}/BUILDROOT"

# Prepare the source directory for RPM build
echo "Preparing the source directory for RPM build..."
mkdir -p "$BUILD_DIR" "$RPMBUILD_DIR" "$SOURCES_DIR" "$SPECS_DIR" "$RPMS_DIR" "$SRPMS_DIR" "$BUILDROOT_DIR"
mkdir -p "${BUILD_DIR}/lh-1.0.0"  # Make sure the directory exists

# Copy all necessary files to the BUILD_DIR
cp -r ./* "${BUILD_DIR}/lh-1.0.0/"  # Ensure it doesn't copy build directory recursively
cd "${BUILD_DIR}"

# Creating the source tarball
echo "Creating source tarball..."
tar -czf "${SOURCES_DIR}/lh-1.0.0.tar.gz" "lh-1.0.0"  # Create tarball of lh-1.0.0

# Generate the RPM spec file
echo "Generating RPM spec file..."
cat > "${SPECS_DIR}/lh.spec" <<EOF
Name: lh
Version: 1.0.0
Release: 1%{?dist}
Summary: Log Helper

License: GPL
URL: $REPO_URL

Source0: %{name}-%{version}.tar.gz

BuildRequires: gcc, make, rpm-build, readline-devel, json-c-devel
Requires: readline, json-c

%description
LH (Log Helper) is a tool to analyze log files.

%prep
%setup -q

%build
gcc -Wall -o lh lh.c -lreadline -ljson-c

%install
mkdir -p %{buildroot}/usr/bin
install -m 755 lh %{buildroot}/usr/bin/lh

%files
/usr/bin/lh

%changelog
* Sat May 11 2024 Your Name <your.email@example.com> - 1.0.0-1
- Initial RPM release
EOF

# Building the RPM package
echo "Building the RPM package..."
rpmbuild -ba "${SPECS_DIR}/lh.spec" --define "_topdir ${RPMBUILD_DIR}"

echo "Build and installation complete. Package located in ${RPMS_DIR}/x86_64/"
