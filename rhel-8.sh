#!/bin/bash
# build-lh-rpm.sh - Script to clone the lh repo and build an RPM package for lh

# Constants
VERSION="1.0.0"
RELEASE="1"
PACKAGE_NAME="lh"
GIT_REPO="https://github.com/unixbox-net/lh.git"
WORK_DIR="$HOME/lh-build"
SOURCE_DIR="$WORK_DIR/source"
RPMBUILD_DIR="$WORK_DIR/rpmbuild"
GIT_CLONE_DIR="$WORK_DIR/lh"
MAINTAINER="Your Name <you@example.com>"

# Setup environment and directories
setup_environment() {
    echo "Setting up environment..."
    mkdir -p "$SOURCE_DIR" "$RPMBUILD_DIR"{/BUILD,/RPMS,/SOURCES,/SPECS,/SRPMS}
    sudo dnf install -y rpm-build gcc json-c-devel readline-devel git
}

# Cleanup environment
cleanup() {
    echo "Cleaning up previous builds..."
    rm -rf "$WORK_DIR"
}

# Clone the GitHub repository
clone_repo() {
    echo "Cloning the lh repository..."
    git clone "$GIT_REPO" "$GIT_CLONE_DIR"
}

# Prepare the source directory and spec file
prepare_source() {
    echo "Preparing source directory..."
    cp -r "$GIT_CLONE_DIR/"* "$SOURCE_DIR"
    tar -czf "$RPMBUILD_DIR/SOURCES/$PACKAGE_NAME-$VERSION.tar.gz" -C "$SOURCE_DIR" .
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
URL:            $GIT_REPO
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
install -m 0755 lh %{buildroot}/usr/bin/

%files
/usr/bin/lh

%changelog
* $(date "+%a %b %d %Y") $MAINTAINER - $VERSION-$RELEASE
- Initial RPM release
EOF
}

# Build RPM package
build_rpm() {
    echo "Building RPM package..."
    rpmbuild -ba "$RPMBUILD_DIR/SPECS/$PACKAGE_NAME.spec"
}

# Main function
main() {
    cleanup
    setup_environment
    clone_repo
    prepare_source
    create_rpm_spec
    build_rpm
}

main
