#!/bin/bash

# Set environment variables
PACKAGE_NAME="lh"
VERSION="1.0.0"
RELEASE="1"
WORKDIR="/tmp/${PACKAGE_NAME}_build"
RPMBUILD_DIR="${WORKDIR}/rpmbuild"
SOURCE_DIR="${PACKAGE_NAME}-${VERSION}"
GIT_REPO="https://github.com/unixbox-net/lh"

# Clear previous build environment if it exists
echo "Checking and clearing previous build environment..."
[ -d "$WORKDIR" ] && rm -rf "$WORKDIR"
mkdir -p $WORKDIR

# Install necessary tools and libraries
echo "Installing necessary development tools and libraries..."
sudo dnf install -y gcc make rpm-build readline-devel json-c-devel git

# Create necessary directories
echo "Creating build directories..."
mkdir -p ${RPMBUILD_DIR}/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
mkdir -p ${WORKDIR}/${SOURCE_DIR}

# Clone the repository
echo "Cloning the repository..."
git clone ${GIT_REPO} ${WORKDIR}/repo

# Move to repository directory to compile
cd ${WORKDIR}/repo
echo "Compiling the source code from the correct directory..."
make

# Check if the executable exists before simulating 'make install'
if [ -f "lh" ]; then
    echo "Simulating 'make install'..."
    mkdir -p ${WORKDIR}/install/usr/bin
    cp lh ${WORKDIR}/install/usr/bin
    echo "'make install' simulated successfully."
else
    echo "Compilation failed, lh executable not found."
    exit 1
fi

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
make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}

%files
%{_bindir}/lh

%changelog
* $(date "+%a %b %d %Y") Your Name <you@example.com> - 1.0.0-1
- Initial RPM release
EOF

# Create the tarball for RPM build
echo "Creating source tarball for RPM build..."
cd ${WORKDIR}/install
tar czf ${RPMBUILD_DIR}/SOURCES/${SOURCE_DIR}.tar.gz usr

# Adjust tarball structure to include a root directory matching the spec expectation
echo "Adjusting tarball structure..."
cd ${RPMBUILD_DIR}/SOURCES
mkdir ${SOURCE_DIR}
tar xzf ${SOURCE_DIR}.tar.gz -C ${SOURCE_DIR}
tar czf ${SOURCE_DIR}.tar.gz ${SOURCE_DIR}
rm -rf ${SOURCE_DIR}

# Build the RPM package
echo "Building the RPM package..."
rpmbuild --define "_topdir ${RPMBUILD_DIR}" -ba ${RPMBUILD_DIR}/SPECS/${PACKAGE_NAME}.spec

# Install the RPM package
echo "Reinstalling the RPM package..."
sudo dnf reinstall -y ${RPMBUILD_DIR}/RPMS/x86_64/${PACKAGE_NAME}-${VERSION}-${RELEASE}.x86_64.rpm

# Output the location of built RPM for verification
echo "Build and installation complete. Package located in ${RPMBUILD_DIR}/RPMS/x86_64/"

# Uncomment below to enable cleanup after inspection
# echo "Cleaning up build environment..."
# rm -rf ${WORKDIR}
