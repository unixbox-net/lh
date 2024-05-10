#!/bin/bash

# Variables
REPO_URL="https://github.com/unixbox-net/loghog.git"
REPO_NAME="loghog"
BUILD_DIR="${HOME}/build"
RPMBUILD_DIR="${HOME}/rpmbuild"
VERSION="1.0.0"
PACKAGE_NAME="lh"
SOURCE_DIR="${PACKAGE_NAME}-${VERSION}"

# Cleanup old build and clone the repo
rm -rf "${HOME}/${REPO_NAME}" "${BUILD_DIR}"
git clone "${REPO_URL}" "${HOME}/${REPO_NAME}"

# Create a new build directory
mkdir -p "${BUILD_DIR}/${SOURCE_DIR}"
cp -R "${HOME}/${REPO_NAME}/." "${BUILD_DIR}/${SOURCE_DIR}"

# Create a source tarball
pushd "${BUILD_DIR}"
tar -czf "${PACKAGE_NAME}-${VERSION}.tar.gz" "${SOURCE_DIR}"
popd

# Setup RPM build directory structure
rpmdev-setuptree

# Copy the source tarball to the SOURCES directory
cp "${BUILD_DIR}/${PACKAGE_NAME}-${VERSION}.tar.gz" "${RPMBUILD_DIR}/SOURCES/"

# Generate a spec file
cat <<EOF > "${RPMBUILD_DIR}/SPECS/${PACKAGE_NAME}.spec"
%global debug_package %{nil}

Name:           ${PACKAGE_NAME}
Version:        ${VERSION}
Release:        1%{?dist}
Summary:        Log searching tool

License:        MIT
URL:            https://github.com/unixbox-net/loghog
Source0:        ${PACKAGE_NAME}-${VERSION}.tar.gz

BuildRequires:  gcc, readline-devel, json-c-devel
Requires:       readline, json-c

%description
LogHOG is a FAST comprehensive log search tool.

%prep
%setup -q

%build
gcc -o lh main.c logs.c json_export.c utils.c -lreadline -ljson-c

%install
rm -rf %{buildroot}
install -d %{buildroot}/usr/bin
install -m 0755 lh %{buildroot}/usr/bin/

%files
/usr/bin/lh

%changelog
* Thu May 09 2024 Your Name <you@example.com> - ${VERSION}-1
- Initial release
EOF

# Build the RPM
rpmbuild -ba "${RPMBUILD_DIR}/SPECS/${PACKAGE_NAME}.spec"

# Create a directory to store the RPMs
mkdir -p "${BUILD_DIR}/rpms"

# Copy the RPM to the build directory
RPM_PATH=$(find "${RPMBUILD_DIR}/RPMS/x86_64" -name "${PACKAGE_NAME}-${VERSION}-1.*.rpm")
if [[ -f "$RPM_PATH" ]]; then
    cp "$RPM_PATH" "${BUILD_DIR}/rpms/"
    echo "RPM package has been created successfully in ${BUILD_DIR}/rpms"
else
    echo "Error: RPM package was not found!"
fi

dnf reinstall -y /root/build/rpms/lh-1.0.0-1.el8.x86_64.rpm
