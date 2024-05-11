#!/bin/bash

# Set environment variables
PACKAGE_NAME="lh"
VERSION="1.0.0"
RELEASE="1"
WORKDIR="/tmp/${PACKAGE_NAME}_build"
RPMBUILD_DIR="${WORKDIR}/rpmbuild"
SOURCE_DIR="${PACKAGE_NAME}-${VERSION}"
GIT_REPO="https://github.com/unixbox-net/lh"

# Install necessary tools and libraries
sudo dnf install -y gcc make rpm-build readline-devel json-c-devel git

# Prepare environment
mkdir -p ${RPMBUILD_DIR}/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
mkdir -p ${WORKDIR}/${SOURCE_DIR}

# Clone the repository
git clone ${GIT_REPO} ${WORKDIR}/repo
cd ${WORKDIR}/repo

# Compile the source code
make

# Move binary to the expected location (emulate 'make install')
mkdir -p ${WORKDIR}/install/usr/bin
cp ${PACKAGE_NAME} ${WORKDIR}/install/usr/bin

# Create spec file
cat <<EOF > ${RPMBUILD_DIR}/SPECS/${PACKAGE_NAME}.spec
Name:           ${PACKAGE_NAME}
Version:        ${VERSION}
Release:        ${RELEASE}%{?dist}
Summary:        A new Linux command tool
License:        MIT
URL:            ${GIT_REPO}
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  gcc, make, readline-devel, json-c-devel
Requires:       readline, json-c

%description
lh (LogHog) is a new Linux command tool for monitoring and managing logs efficiently.

%prep
echo "Starting prep stage..."
%setup -q
echo "Finished prep stage."

%build
echo "Starting build stage..."
make %{?_smp_mflags}
echo "Finished build stage."

%install
echo "Starting install stage..."
make install DESTDIR=%{buildroot}
echo "Finished install stage."

%files
%{_bindir}/lh

%changelog
* $(date "+%a %b %d %Y") Your Name <you@example.com> - 1.0.0-1
- Initial RPM release
EOF

# Create the tarball for RPM build
cd ${WORKDIR}/install
tar czf ${RPMBUILD_DIR}/SOURCES/${SOURCE_DIR}.tar.gz usr

# Adjust tarball structure to include a root directory lh-1.0.0
cd ${RPMBUILD_DIR}/SOURCES
mkdir ${SOURCE_DIR}
tar xzf ${SOURCE_DIR}.tar.gz -C ${SOURCE_DIR}
tar czf ${SOURCE_DIR}.tar.gz ${SOURCE_DIR}
rm -rf ${SOURCE_DIR}

# Build the RPM package
rpmbuild --define "_topdir ${RPMBUILD_DIR}" -ba ${RPMBUILD_DIR}/SPECS/${PACKAGE_NAME}.spec

# Install the RPM package
sudo dnf reinstall -y ${RPMBUILD_DIR}/RPMS/x86_64/${PACKAGE_NAME}-${VERSION}-${RELEASE}.x86_64.rpm

# Output the location of built RPM for verification
echo "Build complete, package located in ${RPMBUILD_DIR}/RPMS/x86_64/"

# Comment out the cleanup for debugging purposes
# echo "Cleanup disabled for debugging."
# rm -rf ${WORKDIR}
