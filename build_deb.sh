#!/bin/bash

# Ensure required packages are installed
REQUIRED_PACKAGES=(build-essential devscripts debhelper dh-make libreadline-dev libjson-c-dev)

echo "Checking for required packages..."
for package in "${REQUIRED_PACKAGES[@]}"; do
    if ! dpkg -s "$package" >/dev/null 2>&1; then
        echo "Installing missing package: $package"
        sudo apt-get install -y "$package"
    fi
done

# Define variables
VERSION="1.0.0"
PACKAGE="loghog"
BUILD_DIR="build-ubuntu"
DEB_DIR="${BUILD_DIR}/${PACKAGE}-${VERSION}"
INSTALL_DIR="${DEB_DIR}/usr/bin"
DEBIAN_DIR="${DEB_DIR}/debian"

# Prepare build directories
echo "Preparing build directories..."
rm -rf "$BUILD_DIR"
mkdir -p "${INSTALL_DIR}"
mkdir -p "${DEBIAN_DIR}"

# Copy the source code to the build directory
echo "Copying source code..."
cp lh.c "${DEB_DIR}/lh.c"

# Create a Makefile
echo "Creating Makefile..."
cat > "${DEB_DIR}/Makefile" <<EOF
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

# Create the Debian control file
echo "Creating Debian control file..."
cat > "${DEBIAN_DIR}/control" <<EOF
Source: ${PACKAGE}
Section: utils
Priority: optional
Maintainer: Your Name <your.email@example.com>
Build-Depends: debhelper (>= 9), libreadline-dev, libjson-c-dev
Standards-Version: 3.9.6

Package: ${PACKAGE}
Architecture: any
Depends: \${shlibs:Depends}, \${misc:Depends}, libreadline-dev, libjson-c-dev
Description: LogHOG - No-Nonsense Digital Forensics
 Offering FAST, comprehensive log analysis, searching, bug hunting, system diagnostics, and digital forensics tools.
EOF

# Create the Debian rules file
echo "Creating Debian rules file..."
cat > "${DEBIAN_DIR}/rules" <<EOF
#!/usr/bin/make -f
# -*- makefile -*-

%:
	dh \$@

override_dh_auto_build:
	\$(MAKE) all

override_dh_auto_clean:
	\$(MAKE) clean
EOF
chmod +x "${DEBIAN_DIR}/rules"

# Create the Debian install file
echo "Creating Debian install file..."
cat > "${DEBIAN_DIR}/install" <<EOF
lh usr/bin
EOF

# Create the Debian changelog file
echo "Creating Debian changelog file..."
cat > "${DEBIAN_DIR}/changelog" <<EOF
${PACKAGE} (${VERSION}-1) unstable; urgency=medium

  * Initial release

 -- Your Name <your.email@example.com>  $(date -R)
EOF

# Create the Debian compat file
echo "Creating Debian compat file..."
echo "9" > "${DEBIAN_DIR}/compat"

# Build the Debian package
echo "Building the Debian package..."
cd "$DEB_DIR"
debuild -us -uc

# Reinstall the newly created Debian package
cd ..
DEB_PACKAGE="${PACKAGE}_${VERSION}-1_amd64.deb"
if [ -f "$DEB_PACKAGE" ]; then
    echo "Installing the new package: $DEB_PACKAGE"
    sudo dpkg -i "$DEB_PACKAGE"
else
    echo "Error: Debian package not created."
    exit 1
fi
