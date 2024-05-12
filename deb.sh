#!/bin/bash

# Check if BASE_DIR is set, if not, set it to a default location
BASE_DIR="${BASE_DIR:-/root/lh}"
DEB_BUILD_DIR="${BASE_DIR}/debbuild"
PACKAGE_NAME="lh"
VERSION="1.0.0"

# Ensuring the script is running as root
if [ "$(id -u)" -ne 0 ]; then
    echo "This script must be run as root. Please use sudo or switch to root." >&2
    exit 1
fi

# Clone git repository
git clone https://github.com/unixbox-net/lh.git ${BASE_DIR}

# Function to prepare directories
prepare_directories() {
    local directories=(
        "$DEB_BUILD_DIR"
        "$DEB_BUILD_DIR/BUILD"
        "$DEB_BUILD_DIR/BUILDROOT"
        "$DEB_BUILD_DIR/DEBS"
        "$DEB_BUILD_DIR/SOURCES"
        "$DEB_BUILD_DIR/SPECS"
        "$DEB_BUILD_DIR/SRPMS"
        "$DEB_BUILD_DIR/SOURCES/${PACKAGE_NAME}-${VERSION}"
    )

    echo "Creating directories..."
    for dir in "${directories[@]}"; do
        mkdir -p "$dir"
    done
    chmod 755 -R "$DEB_BUILD_DIR"  # Ensure proper permissions
}

# Function to install necessary dependencies
install_dependencies() {
    echo "Installing necessary dependencies..."
    apt update
    apt install -y build-essential libjson-c-dev libreadline-dev fakeroot devscripts
}

# Function to prepare source files
prepare_source() {
    echo "Preparing source files..."
    if [ -d "${BASE_DIR}/src" ]; then
        cp -r "${BASE_DIR}/src/"* "${DEB_BUILD_DIR}/SOURCES/${PACKAGE_NAME}-${VERSION}/"
        cp "${BASE_DIR}/LICENSE" "${BASE_DIR}/README.md" "${DEB_BUILD_DIR}/SOURCES/${PACKAGE_NAME}-${VERSION}/"
    else
        echo "Source directory not found. Please ensure the lh source is available in ${BASE_DIR}/src."
        exit 1
    fi
}

# Function to build DEB package
build_deb() {
    echo "Building DEB package..."
    cd "${DEB_BUILD_DIR}/SOURCES/${PACKAGE_NAME}-${VERSION}"
    dh_make -s -c gpl -y  # Initialize debian directory with standard files
    dpkg-buildpackage -rfakeroot -uc -us
}

# Function to install or reinstall DEB
install_or_reinstall_deb() {
    local deb_package="${DEB_BUILD_DIR}/DEBS/${PACKAGE_NAME}_${VERSION}-1_amd64.deb"
    if dpkg -i ${deb_package} &> /dev/null; then
        echo "Package is already installed, attempting reinstall..."
        apt install --reinstall -y ${deb_package}
    else
        echo "Package is not installed, attempting install..."
        dpkg -i ${deb_package}
    fi
}

# Main function
main() {
    prepare_directories
    install_dependencies
    prepare_source
    build_deb
    install_or_reinstall_deb
}

# Execute the main function
main
