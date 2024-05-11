#!/bin/bash

# Set the base directory for the lh project
BASE_DIR="/root/lh"
RPM_BUILD_DIR="${BASE_DIR}/rpmbuild"
PACKAGE_NAME="lh"
VERSION="1.0.0"
RPM_PACKAGE_PATH="${RPM_BUILD_DIR}/RPMS/x86_64/${PACKAGE_NAME}-${VERSION}-1.el8.x86_64.rpm"

# Prepare environment function
prepare_environment() {
    echo "Cleaning up previous builds..."
    rm -rf "${RPM_BUILD_DIR}"
    echo "Installing necessary dependencies..."
    sudo dnf install -y rpm-build gcc json-c-devel readline-devel
}

# Prepare source function
prepare_source() {
    echo "Preparing source files..."
    mkdir -p "${RPM_BUILD_DIR}/SOURCES/lh-${VERSION}" && chmod 755 -R "${RPM_BUILD_DIR}"
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

# Install or Reinstall RPM function
install_or_reinstall_rpm() {
    echo "Checking if package is installed and taking appropriate action..."
    if rpm -q ${PACKAGE_NAME} &>/dev/null; then
        echo "Package is installed, attempting reinstall..."
        sudo dnf reinstall -y ${RPM_PACKAGE_PATH}
    else
        echo "Package not installed, attempting install..."
        sudo dnf install -y ${RPM_PACKAGE_PATH}
    fi
}

# Main function
main() {
    prepare_environment
    prepare_source
    build_rpm
    install_or_reinstall_rpm
}

# Execute the main function
main
