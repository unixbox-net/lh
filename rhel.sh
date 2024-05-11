#!/bin/bash

# Base directory
BASE_DIR="$PWD/lh"

# Create base directory
mkdir -p "$BASE_DIR"
cd "$BASE_DIR"

# Ensure required packages are installed
REQUIRED_PACKAGES=(git rpm-build readline-devel json-c-devel gcc-c++)
echo "Checking for required packages..."
for package in "${REQUIRED_PACKAGES[@]}"; do
    if ! rpm -q "$package" >/dev/null 2>&1; then
        echo "Installing missing package: $package"
        sudo dnf install -y "$package"
    fi
done

# Prepare directories for building RPM
BUILD_DIR="$BASE_DIR/build-rhel"
RPM_BUILD_DIR="$BUILD_DIR/rpmbuild"
mkdir -p "$RPM_BUILD_DIR/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}"
mkdir -p "$RPM_BUILD_DIR/BUILD/lh-1.0.0"

# Fetch source file
echo "Fetching source code..."
curl -o "$RPM_BUILD_DIR/BUILD/lh-1.0.0/lh.c" -L "URL_TO_LH.C" || { echo "Failed to retrieve lh.c"; exit 1; }

# Check if lh.c is really there
if [ ! -f "$RPM_BUILD_DIR/BUILD/lh-1.0.0/lh.c" ]; then
    echo "Source file lh.c not found after download."
    exit 1
fi

# Create the Makefile
echo "Creating Makefile..."
cat > "$RPM_BUILD_DIR/BUILD/lh-1.0.0/Makefile" <<EOF
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
tar czf "$RPM_BUILD_DIR/SOURCES/lh-1.0.0.tar.gz" -C "$RPM_BUILD_DIR/BUILD" "lh-1.0.0"

# Create RPM spec file
echo "Creating RPM spec file..."
cat > "$RPM_BUILD_DIR/SPECS/lh.spec" <<EOF
Name:           lh
Version:        1.0.0
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
rpmbuild --define "_topdir $RPM_BUILD_DIR" -ba "$RPM_BUILD_DIR/SPECS/lh.spec"

# Install the RPM package
echo "Installing the RPM package..."
RPM_PACKAGE="$RPM_BUILD_DIR/RPMS/x86_64/lh-1.0.0-1.el8.x86_64.rpm"
if [ -f "$RPM_PACKAGE" ]; then
    sudo dnf install "$RPM_PACKAGE" -y
fi

# Cleanup
echo "Cleaning up..."
cd ..
rm -rf "$BASE_DIR"

echo "Installation and cleanup complete."
