#!/bin/bash

# Define the base directory as 'lh' within the current working directory
BASE_DIR="$PWD/lh"

# Create the base directory and enter it
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

# Set up build environment
BUILD_DIR="${BASE_DIR}/build-rhel"
RPM_BUILD_DIR="${BUILD_DIR}/rpmbuild"
SOURCES_DIR="${RPM_BUILD_DIR}/SOURCES"
SPECS_DIR="${RPM_BUILD_DIR}/SPECS"

mkdir -p "${RPM_BUILD_DIR}/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}"
mkdir -p "${RPM_BUILD_DIR}/BUILD/lh-1.0.0"

# Fetch or confirm the source code
echo "Preparing source code..."
# Assuming lh.c needs to be fetched or verified:
curl -o "${RPM_BUILD_DIR}/BUILD/lh-1.0.0/lh.c" -L "https://github.com/unixbox-net/loghog/raw/main/lh.c" || { echo "Failed to retrieve lh.c"; exit 1; }

# Create a Makefile
echo "Creating Makefile..."
cat > "${RPM_BUILD_DIR}/BUILD/lh-1.0.0/Makefile" <<EOF
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
tar czf "${SOURCES_DIR}/lh-1.0.0.tar.gz" -C "${RPM_BUILD_DIR}/BUILD" "lh-1.0.0"

# Create the RPM spec file
echo "Creating RPM spec file..."
cat > "${SPECS_DIR}/lh.spec" <<EOF
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
rpmbuild --define "_topdir ${RPM_BUILD_DIR}" -ba "${SPECS_DIR}/lh.spec"

# Install the RPM package
echo "Installing the RPM package..."
RPM_PACKAGE="${RPM_BUILD_DIR}/RPMS/x86_64/lh-1.0.0-1.el8.x86_64.rpm"
if [ -f "$RPM_PACKAGE" ]; then
    sudo dnf install "$RPM_PACKAGE" -y
else
    echo "Error: RPM package not created."
    exit 1
fi

# Cleanup the lh directory
echo "Cleaning up..."
cd ..
rm -rf "$BASE_DIR"

echo "Installation and cleanup complete."
