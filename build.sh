#!/bin/bash

# Prepare directories
BUILD_DIR="$(pwd)/build"
RPMBUILD_DIR="${BUILD_DIR}/rpmbuild"
SOURCE_DIR="${RPMBUILD_DIR}/SOURCES"
SPECS_DIR="${RPMBUILD_DIR}/SPECS"
BUILD_ROOT="${RPMBUILD_DIR}/BUILD"
RPM_OUTPUT_DIR="${BUILD_DIR}/rpms"
CONFIG_FILE="loghog.conf"

# Clean up and create necessary directories
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}" "${RPMBUILD_DIR}" "${SOURCE_DIR}" "${SPECS_DIR}" "${BUILD_ROOT}" "${RPM_OUTPUT_DIR}"

# Set version and name variables
VERSION="1.0.0"
PACKAGE_NAME="lh"
SPEC_FILE="${SPECS_DIR}/${PACKAGE_NAME}.spec"
TAR_NAME="${PACKAGE_NAME}-${VERSION}.tar.gz"
TAR_DIR="${PACKAGE_NAME}-${VERSION}"

# Clone or pull the repository
REPO_URL="https://github.com/unixbox-net/loghog.git"
GIT_CLONE_DIR="$(pwd)/loghog"

if [ -d "${GIT_CLONE_DIR}" ]; then
    rm -rf "${GIT_CLONE_DIR}"
fi

git clone "${REPO_URL}" "${GIT_CLONE_DIR}"

# Copy source files
cp -r "${GIT_CLONE_DIR}" "${BUILD_ROOT}/${TAR_DIR}"
tar czf "${SOURCE_DIR}/${TAR_NAME}" -C "${BUILD_ROOT}" "${TAR_DIR}"

# Create the spec file
cat > "${SPEC_FILE}" <<EOF
Name: ${PACKAGE_NAME}
Version: ${VERSION}
Release: 1%{?dist}
Summary: LogHOG - No-nonsense digital forensics
License: MIT
URL: ${REPO_URL}
Source0: %{name}-%{version}.tar.gz

BuildRequires: readline-devel, json-c-devel, gcc-c++
Requires: readline, json-c

%description
LogHOG is a comprehensive log search tool.

%prep
%setup -q

%build
g++ -o lh main.cpp logs.cpp json_export.cpp utils.cpp -lreadline -ljson-c

%install
rm -rf %{buildroot}
install -d %{buildroot}/usr/bin
install -m 0755 lh %{buildroot}/usr/bin/
install -d %{buildroot}/etc
install -m 0644 ${CONFIG_FILE} %{buildroot}/etc/${CONFIG_FILE}

%files
/usr/bin/lh
/etc/${CONFIG_FILE}

%changelog
* Thu May 09 2024 Scribe <scribe@localhost> - 1.0.0-1
- Initial RPM release
EOF

# Create a default config file
cat > "${BUILD_ROOT}/${TAR_DIR}/${CONFIG_FILE}" <<EOF
/var/lib/docker /var/log
EOF

# Build the RPM package
rpmbuild -ba "${SPEC_FILE}" --define "_topdir ${RPMBUILD_DIR}"

# Copy the resulting RPMs to the build directory
find "${RPMBUILD_DIR}/RPMS" -name "*.rpm" -exec cp {} "${RPM_OUTPUT_DIR}/" \;

# Reinstall the RPM package
dnf reinstall -y "${RPM_OUTPUT_DIR}/${PACKAGE_NAME}-${VERSION}-1.*.rpm"

echo "RPM package has been created successfully in ${RPM_OUTPUT_DIR}"
