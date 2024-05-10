#!/bin/bash

# Variables
REPO_DIR="${HOME}/loghog"
BUILD_DIR="${REPO_DIR}/build"
RPMBUILD_DIR="${BUILD_DIR}/rpmbuild"
VERSION="1.0.0"
PACKAGE_NAME="lh"
SOURCE_DIR="${PACKAGE_NAME}-${VERSION}"

# Ensure the script starts from the correct directory
cd "${REPO_DIR}" || { echo "Directory ${REPO_DIR} not found."; exit 1; }

# Create the build directory structure
mkdir -p "${BUILD_DIR}/${SOURCE_DIR}"
mkdir -p "${RPMBUILD_DIR}"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

# Copy files into the source directory, excluding the build directory itself
find . -maxdepth 1 -type f -exec cp {} "${BUILD_DIR}/${SOURCE_DIR}" \;

# Create a loghog.conf file
cat <<EOF > "${BUILD_DIR}/${SOURCE_DIR}/loghog.conf"
/var/lib/docker /var/log
EOF

# Create a source tarball
pushd "${BUILD_DIR}"
tar -czf "${PACKAGE_NAME}-${VERSION}.tar.gz" "${SOURCE_DIR}"
popd

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

BuildRequires:  gcc-c++, readline-devel, json-c-devel
Requires:       readline, json-c

%description
LogHOG is a FAST comprehensive log search tool.

%prep
%setup -q

%build
g++ -o lh main.cpp logs.cpp json_export.cpp utils.cpp -lreadline -ljson-c

%install
rm -rf %{buildroot}
install -d %{buildroot}/usr/bin
install -m 0755 lh %{buildroot}/usr/bin/
install -d %{buildroot}/etc
install -m 0644 loghog.conf %{buildroot}/etc/

%files
/usr/bin/lh
/etc/loghog.conf

%changelog
* Thu May 09 2024 Your Name <you@example.com> - ${VERSION}-1
- Initial release
EOF

# Build the RPM
rpmbuild --define "_topdir ${RPMBUILD_DIR}" -ba "${RPMBUILD_DIR}/SPECS/${PACKAGE_NAME}.spec"

# Create a directory to store the RPMs
mkdir -p "${BUILD_DIR}/rpms"

# Copy the RPM to the build directory
RPM_PATH=$(find "${RPMBUILD_DIR}/RPMS/x86_64" -name "${PACKAGE_NAME}-${VERSION}-1.*.rpm")
if [[ -f "$RPM_PATH" ]]; then
    cp "$RPM_PATH" "${BUILD_DIR}/rpms/"
    echo "RPM package has been created successfully in ${BUILD_DIR}/rpms"
    # Install or reinstall the package
    sudo dnf reinstall -y "${BUILD_DIR}/rpms/${PACKAGE_NAME}-${VERSION}-1.el8.x86_64.rpm"
else
    echo "Error: RPM package was not found!"
fi
