#!/bin/bash
# Build and package lh in an RPM within the ~/lh directory

# Constants
WORK_DIR="$HOME/lh"
RPMBUILD_DIR="$WORK_DIR/rpmbuild"
PACKAGE_NAME="lh"
VERSION="1.0.0"
TARBALL_NAME="${PACKAGE_NAME}-${VERSION}.tar.gz"

# Prepare environment
prepare_environment() {
    echo "Preparing environment..."
    mkdir -p "$RPMBUILD_DIR"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
    echo "Directories created under $RPMBUILD_DIR"
}

# Cleanup previous builds
cleanup() {
    echo "Cleaning previous builds..."
    rm -rf "$RPMBUILD_DIR"/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}/*
    echo "Cleanup complete."
}

# Setup RPM building environment and dependencies
setup_dependencies() {
    echo "Installing necessary dependencies..."
    sudo dnf install -y rpm-build gcc json-c-devel readline-devel git
}

# Prepare source files and create a tarball
prepare_sources() {
    echo "Preparing source files..."
    local source_dir="$RPMBUILD_DIR/SOURCES/$PACKAGE_NAME-$VERSION"
    mkdir -p "$source_dir"
    cp "$WORK_DIR"/*.c "$WORK_DIR"/*.sh "$WORK_DIR"/LICENSE "$WORK_DIR"/README.md "$source_dir/"
    tar czf "$RPMBUILD_DIR/SOURCES/$TARBALL_NAME" -C "$RPMBUILD_DIR/SOURCES" "$PACKAGE_NAME-$VERSION"
    echo "Source tarball created at $RPMBUILD_DIR/SOURCES/$TARBALL_NAME"
}

# Create the RPM spec file
create_spec_file() {
    cat <<EOF >"$RPMBUILD_DIR/SPECS/$PACKAGE_NAME.spec"
Name:           $PACKAGE_NAME
Version:        $VERSION
Release:        1
Summary:        Lightweight log monitoring tool
License:        MIT
URL:            https://github.com/unixbox-net/lh
Source0:        $TARBALL_NAME
BuildRequires:  gcc, json-c-devel, readline-devel

%description
lh is a lightweight log monitoring tool.

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
* $(date "+%a %b %d %Y") - 1.0.0-1
- Initial release
EOF
    echo "Spec file created."
}

# Prepare source files and create a tarball
prepare_sources() {
    echo "Preparing source files..."
    local source_dir="$RPMBUILD_DIR/SOURCES/$PACKAGE_NAME-$VERSION"
    mkdir -p "$source_dir"
    
    # Copy files to a directory named after the package version
    cp "$WORK_DIR"/*.c "$WORK_DIR"/*.sh "$WORK_DIR"/LICENSE "$WORK_DIR"/README.md "$source_dir/"
    
    # Change to the parent directory of source_dir before creating the tarball
    cd "$RPMBUILD_DIR/SOURCES"
    tar czf "$TARBALL_NAME" "$(basename $source_dir)"
    cd -  # Return to the previous directory to avoid script errors due to changed directory
    
    echo "Source tarball created at $RPMBUILD_DIR/SOURCES/$TARBALL_NAME"
}


# Build the RPM package
build_rpm() {
    echo "Building RPM package..."
    rpmbuild -ba "$RPMBUILD_DIR/SPECS/$PACKAGE_NAME.spec" --define "_topdir $RPMBUILD_DIR"
    echo "RPM package built successfully."
}

# Main function to orchestrate steps
main() {
    cleanup
    prepare_environment
    setup_dependencies
    prepare_sources
    create_spec_file
    build_rpm
}

main
