#!/bin/bash

# Set the base directory for the lh project
BASE_DIR="/root/lh"
RPM_BUILD_DIR="${BASE_DIR}/rpmbuild"
PACKAGE_NAME="lh"
VERSION="1.0.0"

# Function to prepare the environment: create directories and install dependencies
prepare_environment() {
    echo "Cleaning up previous builds..."
    rm -rf "${RPM_BUILD_DIR}"
    mkdir -p "${RPM_BUILD_DIR}/BUILD" "${RPM_BUILD_DIR}/RPMS" "${RPM_BUILD_DIR}/SOURCES" "${RPM_BUILD_DIR}/SPECS" "${RPM_BUILD_DIR}/SRPMS"

    echo "Installing necessary dependencies..."
    sudo dnf install -y rpm-build gcc json-c-devel readline-devel git
}

# Function to prepare the source files for the RPM build
prepare_source() {
    echo "Preparing source files..."
    mkdir -p "${RPM_BUILD_DIR}/SOURCES"
    
    # Copy all source files and additional necessary files to the SOURCES directory
    cp -r "${BASE_DIR}/src/"* "${RPM_BUILD_DIR}/SOURCES/"
    cp "${BASE_DIR}/LICENSE" "${BASE_DIR}/README.md" "${RPM_BUILD_DIR}/SOURCES/"
    cp "${BASE_DIR}/Makefile" "${RPM_BUILD_DIR}/SOURCES/"  # Ensure the Makefile is also copied if it's in the root

    # Create a tarball of the entire SOURCES directory, correctly structuring it within a directory named after the package-version
    tar czf "${RPM_BUILD_DIR}/SOURCES/${PACKAGE_NAME}-${VERSION}.tar.gz" -C "${RPM_BUILD_DIR}/SOURCES" . --transform "s,^,${PACKAGE_NAME}-${VERSION}/,"
    cp "${BASE_DIR}/${PACKAGE_NAME}.spec" "${RPM_BUILD_DIR}/SPECS/"
}


# Function to build the RPM package
build_rpm() {
    echo "Building RPM package..."
    rpmbuild -ba "${RPM_BUILD_DIR}/SPECS/${PACKAGE_NAME}.spec" --define "_topdir ${RPM_BUILD_DIR}"
}

# Main function to orchestrate the script
main() {
    prepare_environment
    prepare_source
    build_rpm
}

# Execute the main function
main
