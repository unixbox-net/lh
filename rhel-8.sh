#!/bin/bash

# Set the base directory
BASE_DIR="/root/lh"
RPM_BUILD_DIR="${BASE_DIR}/rpmbuild"

# Prepare environment function
prepare_environment() {
    echo "Cleaning up previous builds..."
    rm -rf "${RPM_BUILD_DIR}"
    mkdir -p "${RPM_BUILD_DIR}/{BUILD,RPMS,SOURCES,SPECS,SRPMS}"

    echo "Installing necessary dependencies..."
    sudo dnf install -y rpm-build gcc json-c-devel readline-devel git
}

# Prepare source function
prepare_source() {
    echo "Preparing source files..."
    cp -rp "${BASE_DIR}/"*.c "${BASE_DIR}/"*.sh "${BASE_DIR}/LICENSE" "${BASE_DIR}/README.md" "${RPM_BUILD_DIR}/SOURCES/"
    tar czf "${RPM_BUILD_DIR}/SOURCES/${PACKAGE_NAME}-${VERSION}.tar.gz" -C "${RPM_BUILD_DIR}/SOURCES/" .
}

# Build RPM function
build_rpm() {
    echo "Building RPM package..."
    rpmbuild -ba "${RPM_BUILD_DIR}/SPECS/lh.spec" --define "_topdir ${RPM_BUILD_DIR}"
}

# Main function
main() {
    prepare_environment
    prepare_source
    build_rpm
}

# Execute the main function
main
