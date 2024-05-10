#!/bin/bash

# Ensure required packages are installed
REQUIRED_PACKAGES=(git rpm-build readline-devel json-c-devel gcc-c++)

echo "Checking for required packages..."
for package in "${REQUIRED_PACKAGES[@]}"; do
    if ! rpm -q "$package" >/dev/null 2>&1; then
        echo "Installing missing package: $package"
        sudo dnf install -y "$package"
    fi
done

# Define variables
VERSION="1.0.0"
PACKAGE="loghog"
BUILD_DIR="build-rhel"
RPM_BUILD_DIR="${BUILD_DIR}/rpmbuild"
SOURCES_DIR="${RPM_BUILD_DIR}/SOURCES"
SPECS_DIR="${RPM_BUILD_DIR}/SPECS"

# Prepare build directories
echo "Preparing build directories..."
rm -rf "$BUILD_DIR"
mkdir -p "${RPM_BUILD_DIR}/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}"
mkdir -p "${RPM_BUILD_DIR}/BUILD/${PACKAGE}-${VERSION}"

# Copy the source code to the build directory
echo "Copying source code..."
cp lh.c "${RPM_BUILD_DIR}/BUILD/${PACKAGE}-${VERSION}/lh.c"

# Create a Makefile
echo "Creating Makefile..."
cat > "${RPM_BUILD_DIR}/BUILD/${PACKAGE}-${VERSION}/Makefile" <<EOF
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LIBS = -lreadline -ljson-c
EXEC = lh
SRC = lh.c

all:
	\$(CC) \$(CFLAGS) \$(SRC) -o \$(EXEC) \$(LIBS)

clean:
	rm -f \$(EXEC)
EOF

# Create a tarball of the source code
echo "Creating tarball..."
cd "$RPM_BUILD_DIR/SOURCES"
tar czf "${PACKAGE}-${VERSION}.tar.gz" -C "../BUILD" "${PACKAGE}-${VERSION}"
cd ..

# Create the RPM spec file
echo "Creating RPM spec file..."
cat > "${SPECS_DIR}/${PACKAGE}.spec" <<EOF
Name:           ${PACKAGE}
Version:        ${VERSION}
Release:        1%{?dist}
Summary:        Log monitoring tool

License:        MIT
Source0:        %{name}-%{version}.tar.gz

%description
A tool to monitor and search logs.

%prep
%setup -q

%build
make

%install
rm -rf %{buildroot}
install -d %{buildroot}/usr/bin
install -m 0755 lh %{buildroot}/usr/bin/lh

%files
/usr/bin/lh

%changelog
EOF

# Build the RPM package
echo "Building the RPM package..."
rpmbuild --define "_topdir $(pwd)" -ba "${SPECS_DIR}/${PACKAGE}.spec"

# Install the newly created RPM
cd "${BUILD_DIR}/rpmbuild/RPMS/x86_64"
RPM_PACKAGE="${PACKAGE}-${VERSION}-1.el8.x86_64.rpm"
if [ -f "$RPM_PACKAGE" ]; then
    echo "Installing the new package: $RPM_PACKAGE"
    sudo rpm -Uvh --replacepkgs --force "$RPM_PACKAGE"
else
    echo "Error: RPM package not created."
    exit 1
fi
