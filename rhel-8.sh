#!/bin/bash

# Configuration
REPO_URL="https://github.com/unixbox-net/lh.git"
INSTALL_DIR="$HOME/lh"
BUILD_DIR="$INSTALL_DIR/build"
RPMBUILD_DIR="$BUILD_DIR/rpmbuild"
SOURCES_DIR="$RPMBUILD_DIR/SOURCES"
SPECS_DIR="$RPMBUILD_DIR/SPECS"

# Start the reset process
echo "Starting reset process..."
echo "Removing old installations and files..."
sudo dnf remove -y lh  # Assuming 'lh' is the package name

# Remove directories
rm -rf "$INSTALL_DIR"
rm -rf "$BUILD_DIR"
rm -rf /usr/local/bin/lh

# Clone the repository
echo "Cloning the lh project repository..."
git clone "$REPO_URL" "$INSTALL_DIR"
cd "$INSTALL_DIR"

# Create necessary build directories
echo "Creating necessary directories..."
mkdir -p "$BUILD_DIR"
mkdir -p "$SOURCES_DIR"
mkdir -p "$SPECS_DIR"
mkdir -p "$RPMBUILD_DIR/BUILD"
mkdir -p "$RPMBUILD_DIR/BUILDROOT"
mkdir -p "$RPMBUILD_DIR/RPMS"
mkdir -p "$RPMBUILD_DIR/SRPMS"

# Installing necessary development tools and libraries
echo "Installing necessary development tools and libraries..."
sudo dnf install -y gcc make rpm-build readline-devel json-c-devel git

# Prepare the source directory for RPM build
echo "Preparing the source directory for RPM build..."
cp -r * "$BUILD_DIR/"
cd "$BUILD_DIR"
rm -rf lh-1.0.0  # Clean any existing lh-1.0.0 directory
mkdir lh-1.0.0
cp -r * lh-1.0.0/
rm -rf lh-1.0.0/build  # Avoid recursion by not copying the build directory into itself

# Create the source tarball
echo "Creating source tarball..."
tar -czf "$SOURCES_DIR/lh-1.0.0.tar.gz" -C lh-1.0.0 .

# Generate the RPM spec file
echo "Generating RPM spec file..."
cat > "$SPECS_DIR/lh.spec" <<EOF
Summary: A brief description of your package
Name: lh
Version: 1.0.0
Release: 1%{?dist}
License: GPL
Source0: %{name}-%{version}.tar.gz

%description
A longer description of your package

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
* Date Initial build
EOF

# Build the RPM package
echo "Building the RPM package..."
rpmbuild -ba "$SPECS_DIR/lh.spec" --define "_topdir $RPMBUILD_DIR"

echo "Build and installation complete. Package located in $RPMBUILD_DIR/RPMS/x86_64/"

