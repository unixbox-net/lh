#!/bin/bash

# Set environment variables
PACKAGE_NAME="lh"
VERSION="1.0.0"
RELEASE="1"
WORKDIR="$(pwd)/${PACKAGE_NAME}_build"
RPMBUILD_DIR="${WORKDIR}/rpmbuild"
SOURCE_DIR="${PACKAGE_NAME}-${VERSION}"
GIT_REPO="https://github.com/unixbox-net/lh"

# Clear previous build environment if it exists
echo "Checking and clearing previous build environment..."
[ -d "$WORKDIR" ] && rm -rf "$WORKDIR"
mkdir -p $WORKDIR/{install/usr/bin,rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}}

# Install necessary tools and libraries
echo "Installing necessary development tools and libraries..."
sudo dnf install -y gcc make rpm-build readline-devel json-c-devel git

# Clone the repository
echo "Cloning the repository..."
git clone ${GIT_REPO} ${WORKDIR}/repo

# Compile the source code from the repository
cd ${WORKDIR}/repo
echo "Compiling the source code..."
gcc -Wall -Wextra -std=c99 -g lh.c -o lh -lreadline -ljson-c

if [ -f "lh" ]; then
    echo "Compilation successful."
    cp lh ${WORKDIR}/install/usr/bin
else
    echo "Compilation failed, executable not found."
    exit 1
fi

# Prepare the source directory for the tarball
mkdir -p ${WORKDIR}/${SOURCE_DIR}
cp lh.c ${WORKDIR}/${SOURCE_DIR}

# Create the tarball for RPM build
echo "Creating source tarball for RPM build..."
cd ${WORKDIR}
tar czf ${RPMBUILD_DIR}/SOURCES/${SOURCE_DIR}.tar.gz -C ${WORKDIR} ${SOURCE_DIR}

# Create spec file
echo "Creating RPM spec file..."
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
%setup -q

%build
gcc -Wall -Wextra -std=c99 -g ${SOURCE_DIR}/lh.c -o lh -lreadline -ljson-c

%install
cp lh %{buildroot}/usr/bin/lh

%files
/usr/bin/lh

%changelog
* $(date "+%a %b %d %Y") Your Name <you@example.com> - 1.0.0-1
- Initial RPM release
EOF

# Build the RPM package
echo "Building the RPM package..."
rpmbuild --define "_topdir ${RPMBUILD_DIR}" -ba ${RPMBUILD_DIR}/SPECS/${PACKAGE_NAME}.spec

# Install the RPM package
echo "Reinstalling the RPM package..."
sudo dnf reinstall -y ${RPMBUILD_DIR}/RPMS/x86_64/${PACKAGE_NAME}-${VERSION}-${RELEASE}.x86_64.rpm

echo "Build and installation complete. Package located in ${RPMBUILD_DIR}/RPMS/x86_64/"
