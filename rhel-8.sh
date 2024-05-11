#!/bin/bash

# Define constants
BASE_DIR="/root/lh"
RPM_BUILD_DIR="${BASE_DIR}/rpmbuild"
PACKAGE_NAME="lh"
VERSION="1.0.0"

# Function to prepare directories
prepare_directories() {
    local directories=(
        "$RPM_BUILD_DIR"
        "$RPM_BUILD_DIR/BUILD"
        "$RPM_BUILD_DIR/BUILDROOT"
        "$RPM_BUILD_DIR/RPMS"
        "$RPM_BUILD_DIR/SOURCES"
        "$RPM_BUILD_DIR/SPECS"
        "$RPM_BUILD_DIR/SRPMS"
        "$RPM_BUILD_DIR/SOURCES/${PACKAGE_NAME}-${VERSION}"
    )

    echo "Creating directories..."
    for dir in "${directories[@]}"; do
        mkdir -p "$dir"
    done
    chmod 755 -R "$BASE_DIR"  # Ensure proper permissions
}

# Function to install necessary dependencies
install_dependencies() {
    echo "Installing necessary dependencies..."
    sudo dnf install -y rpm-build gcc json-c-devel readline-devel
}

# Function to prepare source files
prepare_source() {
    echo "Preparing source files..."
    cp -r "${BASE_DIR}/src/"* "${RPM_BUILD_DIR}/SOURCES/${PACKAGE_NAME}-${VERSION}/"
    cp "${BASE_DIR}/LICENSE" "${BASE_DIR}/README.md" "${RPM_BUILD_DIR}/SOURCES/${PACKAGE_NAME}-${VERSION}/"
    tar czf "${RPM_BUILD_DIR}/SOURCES/${PACKAGE_NAME}-${VERSION}.tar.gz" -C "${RPM_BUILD_DIR}/SOURCES" "${PACKAGE_NAME}-${VERSION}"
    cp "${BASE_DIR}/${PACKAGE_NAME}.spec" "${RPM_BUILD_DIR}/SPECS/"
}

# Function to build RPM
build_rpm() {
    echo "Building RPM package..."
    rpmbuild -ba "${RPM_BUILD_DIR}/SPECS/${PACKAGE_NAME}.spec" --define "_topdir ${RPM_BUILD_DIR}"
}

# Function to install or reinstall RPM
install_or_reinstall_rpm() {
    local rpm_package="${RPM_BUILD_DIR}/RPMS/x86_64/${PACKAGE_NAME}-${VERSION}-1.el8.x86_64.rpm"
    if rpm -q ${PACKAGE_NAME}; then
        echo "Package is already installed, attempting reinstall..."
        sudo dnf reinstall -y ${rpm_package}
    else
        echo "Package is not installed, attempting install..."
        sudo dnf install -y ${rpm_package}
    fi
}

# Main function
main() {
    prepare_directories
    install_dependencies
    prepare_source
    build_rpm
    install_or_reinstall_rpm
}

# Execute the main function
main
