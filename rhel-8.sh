#!/bin/bash

# Set base directory
BASE_DIR="/root/lh"
RPM_BUILD_DIR="${BASE_DIR}/rpmbuild"

# Prepare environment function
prepare_environment() {
    echo "Cleaning up previous builds..."
    rm -rf "${RPM_BUILD_DIR}"
    mkdir -p "${RPM_BUILD_DIR}/BUILD" "${RPM_BUILD_DIR}/RPMS" "${RPM_BUILD_DIR}/SOURCES" "${RPM_BUILD_DIR}/SPECS" "${RPM_BUILD_DIR}/SRPMS"

    echo "Installing necessary dependencies..."
    sudo dnf install -y rpm-build gcc json-c-devel readline-devel git
}

# Prepare source function
prepare_source() {
    echo "Preparing source files..."
    # Ensure source files are copied correctly
    cp "${BASE_DIR}/src/"*.c "${RPM_BUILD_DIR}/SOURCES/"
    cp "${BASE_DIR}/LICENSE" "${BASE_DIR}/README.md" "${RPM_BUILD_DIR}/SOURCES/"
    tar czf "${RPM_BUILD_DIR}/SOURCES/${PACKAGE_NAME}-${VERSION}.tar.gz" -C "${BASE_DIR}/src" .
    cp "${BASE_DIR}/lh.spec" "${RPM_BUILD_DIR}/SPECS/"
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
