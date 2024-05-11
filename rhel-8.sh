#!/bin/bash

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

# Create necessary directories
echo "Creating necessary directories..."
mkdir -p "$BUILD_DIR" "$RPMBUILD_DIR" "$SOURCES_DIR" "$SPECS_DIR" "$RPMS_DIR" "$SRPMS_DIR" "$BUILDROOT_DIR"

# Install necessary development tools and libraries
echo "Installing necessary development tools and libraries..."
sudo dnf install -y gcc make rpm-build readline-devel json-c-devel git

# Assuming that lh.c and other project files are already in $INSTALL_DIR
cd "$INSTALL_DIR"

# Compile the source code
echo "Compiling the source code..."
gcc -Wall -o lh lh.c -lreadline -ljson-c

# Prepare the source directory for RPM build
echo "Preparing the source directory for RPM build..."
# Copy project files to BUILD_DIR but do not include the build directory itself
rsync -av --exclude='build' ./* "$BUILD_DIR/"

# Creating the source tarball
echo "Creating source tarball..."
tar -czf "${SOURCES_DIR}/lh-1.0.0.tar.gz" -C "$BUILD_DIR" .

# Generating the RPM spec file
echo "Generating RPM spec file..."
cat > "${SPECS_DIR}/lh.spec" <<EOF
Name: lh
Version: 1.0.0
Release: 1%{?dist}
Summary: No-nonsense digital forensics tool

License: GPL
URL: $REPO_URL

Source0: %{name}-%{version}.tar.gz

BuildRequires: gcc, make, rpm-build, readline-devel, json-c-devel
Requires: readline, json-c

%description
lh (LogHog) is a no-nonsense digital forensics tool.

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
