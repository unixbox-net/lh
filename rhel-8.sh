#!/bin/bash

# Set the base directory for the lh project
BASE_DIR="/root/lh"
RPM_BUILD_DIR="${BASE_DIR}/rpmbuild"
PACKAGE_NAME="lh"
VERSION="1.0.0"

# Prepare environment function
prepare_environment() {
    echo "Cleaning up previous builds..."
    rm -rf "${RPM_BUILD_DIR}"
    #mkdir -p "${RPM_BUILD_DIR}/{BUILD,RPMS,SOURCES,SPECS,SRPMS}"

    echo "Installing necessary dependencies..."
    sudo dnf install -y rpm-build gcc json-c-devel readline-devel
}

# Prepare source function
prepare_source() {
    echo "Preparing source files..."
    mkdir -p "${RPM_BUILD_DIR}/SOURCES/lh-${VERSION}"
    cp -r "${BASE_DIR}/src/"* "${RPM_BUILD_DIR}/SOURCES/lh-${VERSION}/"
    cp "${BASE_DIR}/LICENSE" "${BASE_DIR}/README.md" "${RPM_BUILD_DIR}/SOURCES/lh-${VERSION}/"

    # Create tarball
    tar czf "${RPM_BUILD_DIR}/SOURCES/${PACKAGE_NAME}-${VERSION}.tar.gz" -C "${RPM_BUILD_DIR}/SOURCES/" lh-${VERSION}
    cp "${BASE_DIR}/${PACKAGE_NAME}.spec" "${RPM_BUILD_DIR}/SPECS/"
}

# Build RPM function
build_rpm() {
    echo "Building RPM package..."
    rpmbuild -ba "${RPM_BUILD_DIR}/SPECS/${PACKAGE_NAME}.spec" --define "_topdir ${RPM_BUILD_DIR}"
}

# Main function
main() {
    prepare_environment
    prepare_source
    build_rpm
}

# Execute the main function
main
