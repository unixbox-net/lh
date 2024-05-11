#!/bin/bash
# build-lh-rpm.sh - Script to build an RPM package for lh

# Constants
VERSION="1.0.0"
RELEASE="1"
PACKAGE_NAME="lh"
GIT_REPO="https://github.com/unixbox-net/lh.git"
WORK_DIR="$HOME/lh-build"
GIT_CLONE_DIR="$WORK_DIR/lh"
RPMBUILD_DIR="$WORK_DIR/rpmbuild"
MAINTAINER="Your Name <you@example.com>"

# Setup environment and directories
setup_environment() {
    echo "Setting up environment..."
    sudo dnf install -y rpm-build gcc json-c-devel readline-devel git
    mkdir -p "$RPMBUILD_DIR"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
    echo "Environment setup complete."
}

# Cleanup environment
cleanup() {
    echo "Cleaning up previous builds..."
    rm -rf "$WORK_DIR"
    echo "Cleanup complete."
}

# Clone the GitHub repository
clone_repo() {
    echo "Cloning the lh repository..."
    git clone "$GIT_REPO" "$GIT_CLONE_DIR"
    echo "Repository cloned."
}

# Prepare the source directory and spec file
prepare_source() {
    echo "Preparing source directory..."
    tar -czf "$RPMBUILD_DIR/SOURCES/$PACKAGE_NAME-$VERSION.tar.gz" -C "$GIT_CLONE_DIR" .
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
    echo "Spec file created."
}

# Build RPM package
build_rpm() {
    echo "Building RPM package..."
    rpmbuild -ba "$RPMBUILD_DIR/SPECS/$PACKAGE_NAME.spec"
    echo "RPM package built."
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
