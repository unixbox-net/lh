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
PACKAGE="lh"
BUILD_DIR="$PWD/build-rhel"
RPM_BUILD_DIR="${BUILD_DIR}/rpmbuild"
SOURCES_DIR="${RPM_BUILD_DIR}/SOURCES"
SPECS_DIR="${RPM_BUILD_DIR}/SPECS"
LOGHOG_DIR="$PWD"

# Prepare build directories
echo "Preparing build directories..."
rm -rf "$BUILD_DIR"
mkdir -p "${RPM_BUILD_DIR}/BUILD"
mkdir -p "${RPM_BUILD_DIR}/BUILDROOT"
mkdir -p "${RPM_BUILD_DIR}/RPMS"
mkdir -p "${RPM_BUILD_DIR}/SOURCES"
mkdir -p "${RPM_BUILD_DIR}/SPECS"
mkdir -p "${RPM_BUILD_DIR}/SRPMS"
mkdir -p "${RPM_BUILD_DIR}/BUILD/${PACKAGE}-${VERSION}"

# Copy the source code to the build directory
echo "Copying source code..."
cp lh.c "${RPM_BUILD_DIR}/BUILD/${PACKAGE}-${VERSION}/lh.c"

# Create a Makefile using printf to ensure tabs are preserved
echo "Creating Makefile..."
printf "CC = gcc\nCFLAGS = -Wall -Wextra -std=c99 -g\nLIBS = -lreadline -ljson-c\nEXEC = lh\nSRC = lh.c\n\nall:\n\t\$(CC) \$(CFLAGS) \$(SRC) -o \$(EXEC) \$(LIBS)\n\nclean:\n\trm -f \$(EXEC)\n" > "${RPM_BUILD_DIR}/BUILD/${PACKAGE}-${VERSION}/Makefile"

# Create a tarball of the source code
echo "Creating tarball..."
tar czf "${SOURCES_DIR}/${PACKAGE}-${VERSION}.tar.gz" -C "${RPM_BUILD_DIR}/BUILD" "${PACKAGE}-${VERSION}"

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
install -d %{buildroot}/usr/local/bin
install -m 0755 lh %{buildroot}/usr/local/bin/lh

%files
/usr/local/bin/lh

%changelog
EOF

# Build the RPM package
echo "Building the RPM package..."
rpmbuild --define "_topdir ${RPM_BUILD_DIR}" -ba "${SPECS_DIR}/${PACKAGE}.spec"

# Move the RPM package to the loghog directory and install it using dnf
echo "Managing the RPM package..."
RPM_PACKAGE="${RPM_BUILD_DIR}/RPMS/x86_64/${PACKAGE}-${VERSION}-1.el8.x86_64.rpm"
if [ -f "$RPM_PACKAGE" ]; then
    mv "$RPM_PACKAGE" "$LOGHOG_DIR/"
    echo "Package moved to $LOGHOG_DIR"
    echo "Installing the package using DNF..."
    if sudo dnf list installed | grep -q "^${PACKAGE}"; then
        sudo dnf upgrade "$LOGHOG_DIR/$(basename "$RPM_PACKAGE")" -y
    else
        sudo dnf install "$LOGHOG_DIR/$(basename "$RPM_PACKAGE")" -y
    fi
    echo "Cleaning up..."
    rm -f "$LOGHOG_DIR/$(basename "$RPM_PACKAGE")"
    rm -rf "$BUILD_DIR"
else
    echo "Error: RPM package not created."
    exit 1
fi
