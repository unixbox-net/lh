#!/bin/bash

# Define environment variables
export PACKAGE_NAME="lh"
export VERSION="1.0.0"
export RELEASE="1"
export BASE_DIR="$(dirname "$(realpath "$0")")"
export WORKDIR="${BASE_DIR}/build"
export RPMBUILD_DIR="${WORKDIR}/rpmbuild"
export INSTALL_DIR="${WORKDIR}/install/usr/bin"

# Ensure directories are set up properly
echo "Setting up build environment..."
mkdir -p "${INSTALL_DIR}"
mkdir -p "${RPMBUILD_DIR}/{BUILD,RPMS,SOURCES,SPECS,SRPMS}"
mkdir -p "${WORKDIR}/${PACKAGE_NAME}-${VERSION}"

# Install necessary development tools and libraries
echo "Installing necessary development tools and libraries..."
sudo dnf install -y gcc make rpm-build readline-devel json-c-devel git

# Update repository and compile the source code
echo "Updating repository and compiling the source code..."
cd "${BASE_DIR}" || exit
git pull
gcc -Wall -Wextra -std=c99 -g "${BASE_DIR}/lh.c" -o "${BASE_DIR}/lh" -lreadline -ljson-c

if [ -f "${BASE_DIR}/lh" ]; then
    echo "Compilation successful."
    cp "${BASE_DIR}/lh" "${INSTALL_DIR}"
else
    echo "Compilation failed, executable not found."
    exit 1
fi

# Prepare the source directory for the tarball
echo "Preparing source directory for the RPM build..."
cp -a "${BASE_DIR}"/* "${WORKDIR}/${PACKAGE_NAME}-${VERSION}/"

# Create the source tarball for the RPM build
echo "Creating source tarball..."
tar czf "${RPMBUILD_DIR}/SOURCES/${PACKAGE_NAME}-${VERSION}.tar.gz" -C "${WORKDIR}" "${PACKAGE_NAME}-${VERSION}"

# Generate the RPM spec file
echo "Generating RPM spec file..."
cat <<EOF > "${RPMBUILD_DIR}/SPECS/${PACKAGE_NAME}.spec"
Name:           ${PACKAGE_NAME}
Version:        ${VERSION}
Release:        ${RELEASE}%{?dist}
Summary:        Comprehensive log management tool
License:        MIT
URL:            https://github.com/unixbox-net/lh
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  gcc, make, readline-devel, json-c-devel
Requires:       libc.so.6(GLIBC_2.14)(64bit), readline, json-c

%description
${PACKAGE_NAME} is a versatile tool for managing and analyzing system logs.

%prep
%setup -q

%build
gcc -Wall -Wextra -std=c99 -g lh.c -o lh -lreadline -ljson-c

%install
mkdir -p %{buildroot}/usr/bin
cp lh %{buildroot}/usr/bin/lh

%files
/usr/bin/lh

%changelog
* $(date "+%a %b %d %Y") Your Name <you@example.com> - ${VERSION}-${RELEASE}
- Initial release
EOF

# Build and install the RPM package
echo "Building and installing the RPM package..."
rpmbuild --define "_topdir ${RPMBUILD_DIR}" -ba "${RPMBUILD_DIR}/SPECS/${PACKAGE_NAME}.spec"
sudo dnf install -y "${RPMBUILD_DIR}/RPMS/x86_64/${PACKAGE_NAME}-${VERSION}-${RELEASE}.x86_64.rpm"

echo "Build and installation complete. Package located in ${RPMBUILD_DIR}/RPMS/x86_64/"
