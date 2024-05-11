#!/bin/bash

# Define directory structure
BUILD_DIR="$(pwd)/build"
RPMBUILD_DIR="$BUILD_DIR/rpmbuild"
SOURCES_DIR="$RPMBUILD_DIR/SOURCES"
SPECS_DIR="$RPMBUILD_DIR/SPECS"

# Create necessary directories
echo "Creating necessary directories..."
mkdir -p "$BUILD_DIR"
mkdir -p "$SOURCES_DIR"
mkdir -p "$SPECS_DIR"
mkdir -p "$RPMBUILD_DIR/BUILD"
mkdir -p "$RPMBUILD_DIR/BUILDROOT"
mkdir -p "$RPMBUILD_DIR/RPMS"
mkdir -p "$RPMBUILD_DIR/SRPMS"

# Install development tools and libraries
echo "Installing necessary development tools and libraries..."
sudo dnf install -y gcc make rpm-build readline-devel json-c-devel git

# Prepare the source directory for RPM build
echo "Preparing the source directory for RPM build..."
cp -a * "$BUILD_DIR/"
cd "$BUILD_DIR"
tar -czf "$SOURCES_DIR/lh-1.0.0.tar.gz" --exclude="$BUILD_DIR" --exclude=".git" .

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
* $(date +'%a %b %d %Y') Your Name <your-email@example.com> - 1.0.0-1
- Initial RPM release
EOF

# Build the RPM package
echo "Building the RPM package..."
rpmbuild -ba "$SPECS_DIR/lh.spec" --define "_topdir $RPMBUILD_DIR"

echo "Build and installation complete. Package located in $RPMBUILD_DIR/RPMS/x86_64/"
