#!/bin/bash
# build-lh-rpm.sh - Script to build an RPM package for lh within the ~/lh directory

# Constants
VERSION="1.0.0"
RELEASE="1"
PACKAGE_NAME="lh"
WORK_DIR="$HOME/lh"
RPMBUILD_DIR="$WORK_DIR/rpmbuild"
MAINTAINER="Your Name <you@example.com>"

# Setup environment and directories
setup_environment() {
    echo "Setting up environment..."
    sudo dnf install -y rpm-build gcc json-c-devel readline-devel git
    mkdir -p "$RPMBUILD_DIR"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
    echo "Environment setup complete."
}

# Cleanup environment
cleanup() {
    echo "Cleaning up previous builds..."
    rm -rf "$RPMBUILD_DIR"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
    echo "Cleanup complete."
}

# Prepare the source directory and spec file
prepare_source() {
    echo "Preparing source directory..."
    mkdir -p "$RPMBUILD_DIR/SOURCES/$PACKAGE_NAME-$VERSION"
    cp "$WORK_DIR"/*.c "$WORK_DIR"/*.sh "$WORK_DIR"/LICENSE "$WORK_DIR"/README.md "$RPMBUILD_DIR/SOURCES/$PACKAGE_NAME-$VERSION/"
    tar -czf "$RPMBUILD_DIR/SOURCES/$PACKAGE_NAME-$VERSION.tar.gz" -C "$RPMBUILD_DIR/SOURCES" "$PACKAGE_NAME-$VERSION"
    rm -rf "$RPMBUILD_DIR/SOURCES/$PACKAGE_NAME-$VERSION"
    echo "Source tarball created."
}

# Create RPM spec file
create_rpm_spec() {
    echo "Creating RPM spec file..."
    cat > "$RPMBUILD_DIR/SPECS/$PACKAGE_NAME.spec" << EOF
Name:           $PACKAGE_NAME
Version:        $VERSION
Release:        $RELEASE%{?dist}
Summary:        Lightweight log monitoring tool
License:        MIT
URL:            https://github.com/unixbox-net/lh
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  gcc, json-c-devel, readline-devel

%description
A lightweight and simple log monitoring tool.

%prep
%setup -q

%build
gcc -o lh lh.c -Wall -lreadline -ljson-c

%install
mkdir -p %{buildroot}/usr/bin
install -m 0755 lh %{buildroot}/usr/bin/lh

%files
/usr/bin/lh

%changelog
* $(date "+%a %b %d %Y") $MAINTAINER - $VERSION-$RELEASE
- Initial RPM release
EOF
    echo "Spec file created."
}

# Build RPM package
build_rpm() {
    echo "Building RPM package..."
    rpmbuild --define "_topdir $RPMBUILD_DIR" -ba "$RPMBUILD_DIR/SPECS/$PACKAGE_NAME.spec"
    echo "RPM package built."
}

# Main function
main() {
    cleanup
    setup_environment
    prepare_source
    create_rpm_spec
    build_rpm
}

main
