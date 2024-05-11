#!/bin/bash
# build-lh-rpm.sh - Script to clone the loghog repo and build an RPM package for loghog

# Constants
VERSION="1.0.0"
RELEASE="1"
PACKAGE_NAME="lh"
GIT_REPO="https://github.com/unixbox-net/loghog.git"
GIT_CLONE_DIR="$HOME/lh"
SOURCE_DIR="${PACKAGE_NAME}-${VERSION}"
SOURCE_FILE="lh.c"
BIN_FILE="loghog"
LICENSE_FILE="LICENSE"
README_FILE="README.md"
RPMBUILD_ROOT="$HOME/rpmbuild"
MAINTAINER="Your Name <you@example.com>"

# Ensure dependencies are installed
sudo dnf install -y rpm-build gcc json-c-devel readline-devel git

# Cleanup previous RPM build
cleanup() {
    echo "Cleaning up previous builds..."
    rm -rf "${RPMBUILD_ROOT}" "${GIT_CLONE_DIR}"
}

# Prepare RPM build environment
prepare_rpm_env() {
    echo "Preparing RPM build environment..."
    mkdir -p "${RPMBUILD_ROOT}"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
}

# Clone the GitHub repository
clone_repo() {
    echo "Cloning the ${PACKAGE_NAME} repository..."
    git clone "${GIT_REPO}" "${GIT_CLONE_DIR}"
}

# Generate the spec file for RPM
create_rpm_spec() {
    echo "Creating RPM spec file..."
    SPEC_FILE="${RPMBUILD_ROOT}/SPECS/${PACKAGE_NAME}.spec"

    cat <<EOF > "${SPEC_FILE}"
Name:           ${PACKAGE_NAME}
Version:        ${VERSION}
Release:        ${RELEASE}%{?dist}
Summary:        A lightweight and simple log monitoring tool
License:        MIT
URL:            ${GIT_REPO}
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
    echo "Creating source tarball for RPM build..."
    tar czvf "${RPMBUILD_ROOT}/SOURCES/${PACKAGE_NAME}-${VERSION}.tar.gz" -C "${GIT_CLONE_DIR}" .
}

# Build RPM package
build_rpm() {
    echo "Building RPM package..."
    rpmbuild -ba "${RPMBUILD_ROOT}/SPECS/${PACKAGE_NAME}.spec"
}

# Main function
main() {
    cleanup
    prepare_rpm_env
    clone_repo
    create_rpm_spec
    create_source_tarball
    build_rpm
}

# Execute the main function
main
