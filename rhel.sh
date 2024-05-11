#!/bin/bash

# Define the base directory as 'lh' within the current working directory
BASE_DIR="$PWD/lh"

# Create the base directory
mkdir -p "$BASE_DIR"
cd "$BASE_DIR"

# Optional: Clone/copy necessary files into BASE_DIR
# For example, if you need to pull files from a repository:
# git clone https://github.com/unixbox-net/loghog.git "$BASE_DIR"
# For this example, let's assume the necessary files are in the script itself or are fetched separately.

# Ensure required packages are installed
REQUIRED_PACKAGES=(git rpm-build readline-devel json-c-devel gcc-c++)

echo "Checking for required packages..."
for package in "${REQUIRED_PACKAGES[@]}"; do
    if ! rpm -q "$package" >/dev/null 2>&1; then
        echo "Installing missing package: $package"
        sudo dnf install -y "$package"
    fi
done

# Set up the build environment
BUILD_DIR="${BASE_DIR}/build-rhel"
RPM_BUILD_DIR="${BUILD_DIR}/rpmbuild"
SOURCES_DIR="${RPM_BUILD_DIR}/SOURCES"
SPECS_DIR="${RPM_BUILD_DIR}/SPECS"

mkdir -p "${RPM_BUILD_DIR}/BUILD"
mkdir -p "${RPM_BUILD_DIR}/BUILDROOT"
mkdir -p "${RPM_BUILD_DIR}/RPMS"
mkdir -p "${RPM_BUILD_DIR}/SOURCES"
mkdir -p "${RPM_BUILD_DIR}/SPECS"
mkdir -p "${RPM_BUILD_DIR}/SRPMS"
mkdir -p "${RPM_BUILD_DIR}/BUILD/lh-1.0.0"

# Continue with script operations (build, makefile creation, etc.)

# Example Makefile creation
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
fi

# Cleanup the lh directory
cd ..
rm -rf "$BASE_DIR"

echo "Installation and cleanup complete."
