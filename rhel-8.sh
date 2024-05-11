#!/bin/bash

# Constants
PACKAGE_NAME="lh"
VERSION="1.0.0"
RELEASE="1"
MAINTAINER="Your Name <you@example.com>"
SOURCE_FILE="lh.c"
BIN_FILE="$PACKAGE_NAME"
WORKDIR="$HOME/${PACKAGE_NAME}_build"
RPMBUILD_DIR="$WORKDIR/rpmbuild"

# Ensure necessary packages are installed
sudo dnf install -y rpm-build gcc make

# Prepare environment
mkdir -p $RPMBUILD_DIR/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
cd $WORKDIR

# Create necessary files
echo "#include <stdio.h>
int main() {
    printf(\"Hello from $PACKAGE_NAME\\n\");
    return 0;
}" > $SOURCE_FILE

# Create the correct directory structure within the tarball
mkdir $PACKAGE_NAME-$VERSION
mv $SOURCE_FILE $PACKAGE_NAME-$VERSION/

# Spec file creation
cat <<EOF > $RPMBUILD_DIR/SPECS/$PACKAGE_NAME.spec
Name:           $PACKAGE_NAME
Version:        $VERSION
Release:        $RELEASE
Summary:        Simple log handler tool
License:        MIT
URL:            https://github.com/unixbox-net/$PACKAGE_NAME
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  gcc, make

%description
%{summary}.

%prep
%setup -q

%build
gcc -o $BIN_FILE $SOURCE_FILE

%install
mkdir -p %{buildroot}/usr/bin
install -m 0755 $BIN_FILE %{buildroot}/usr/bin/$BIN_FILE

%files
/usr/bin/$BIN_FILE

%changelog
* $(date "+%a %b %d %Y") $MAINTAINER - $VERSION-$RELEASE
- Initial RPM release
EOF

# Create source tarball with correct directory structure
tar czf $RPMBUILD_DIR/SOURCES/$PACKAGE_NAME-$VERSION.tar.gz $PACKAGE_NAME-$VERSION

# Build the RPM
rpmbuild --define "_topdir $RPMBUILD_DIR" -ba $RPMBUILD_DIR/SPECS/$PACKAGE_NAME.spec

# Clean up
cd $HOME
rm -rf $WORKDIR

echo "Build complete, package located in $RPMBUILD_DIR/RPMS/"
