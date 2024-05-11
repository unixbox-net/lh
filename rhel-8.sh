#!/bin/bash
# build-loghog-rpm.sh - Script to clone the loghog repo and build an RPM package for loghog

# Constants
VERSION="1.0.0"
RELEASE="1"
PACKAGE_NAME="lh"
GIT_REPO="https://github.com/unixbox-net/loghog.git"
GIT_CLONE_DIR="$HOME/loghog"
SOURCE_DIR="${PACKAGE_NAME}-${VERSION}"
SOURCE_FILE="lh.c"
BIN_FILE="loghog"
LICENSE_FILE="LICENSE"
README_FILE="README.md"
RPMBUILD_ROOT="$HOME/rpmbuild"
MAINTAINER="Your Name <you@example.com>"

# Prepare RPM build environment
prepare_rpm_env() {
    mkdir -p "${RPMBUILD_ROOT}"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
}

# Generate the spec file for RPM
create_rpm_spec() {
    SPEC_FILE="${RPMBUILD_ROOT}/SPECS/${PACKAGE_NAME}.spec"

    cat <<EOF > "${SPEC_FILE}"
Name:           ${PACKAGE_NAME}
Version:        ${VERSION}
Release:        ${RELEASE}%{?dist}
Summary:        A lightweight and simple log monitoring tool
License:        MIT
URL:            https://github.com/unixbox-net/loghog
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  gcc, json-c-devel, readline-devel

%description
A lightweight and simple log monitoring tool.

%global _enable_debug_packages 0
%global debug_package %{nil}

%prep
%setup -q

%build
gcc -o ${BIN_FILE} ${SOURCE_FILE} -Wall -lreadline -ljson-c

%install
mkdir -p %{buildroot}/usr/bin
install -m 0755 ${BIN_FILE} %{buildroot}/usr/bin/${BIN_FILE}
mkdir -p %{buildroot}/usr/share/licenses/%{name}
install -m 0644 ${LICENSE_FILE} %{buildroot}/usr/share/licenses/%{name}/
mkdir -p %{buildroot}/usr/share/doc/%{name}
install -m 0644 ${README_FILE} %{buildroot}/usr/share/doc/%{name}/

%files
/usr/bin/${BIN_FILE}
/usr/share/licenses/%{name}/${LICENSE_FILE}
/usr/share/doc/%{name}/${README_FILE}

%changelog
* $(date "+%a %b %d %Y") ${MAINTAINER} - ${VERSION}-${RELEASE}
- Initial build
EOF
}

# Create tarball for source files
create_source_tarball() {
    mkdir -p "${SOURCE_DIR}"
    cp "${GIT_CLONE_DIR}"/{${SOURCE_FILE},${LICENSE_FILE},${README_FILE}} "${SOURCE_DIR}/"
    tar czvf "${RPMBUILD_ROOT}/SOURCES/${PACKAGE_NAME}-${VERSION}.tar.gz" -C "${SOURCE_DIR}" .
    rm -rf "${SOURCE_DIR}"
}

# Clone the GitHub repository
clone_repo() {
    rm -rf "${GIT_CLONE_DIR}"
    git clone "${GIT_REPO}" "${GIT_CLONE_DIR}"
}

# Build RPM package
build_rpm() {
    rpmbuild -ba "${RPMBUILD_ROOT}/SPECS/${PACKAGE_NAME}.spec"
}

# Cleanup previous RPM build
cleanup() {
    rm -rf "${RPMBUILD_ROOT}" "${GIT_CLONE_DIR}"
}

# Main function
main() {
    echo "Cleaning up previous builds..."
    cleanup

    echo "Cloning the ${PACKAGE_NAME} repository..."
    clone_repo

    echo "Preparing RPM build environment..."
    prepare_rpm_env

    echo "Creating RPM spec file..."
    create_rpm_spec

    echo "Creating source tarball for RPM build..."
    create_source_tarball

    echo "Building RPM package..."
    build_rpm
}

# Ensure dependencies are installed
sudo dnf install -y rpm-build gcc json-c-devel readline-devel

# Execute the main function
main
