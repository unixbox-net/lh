#!/bin/bash

# Ensure the required build tools are available
REQUIRED_PACKAGES=(git rpm-build gcc-c++ json-c-devel readline-devel)

echo "Checking for required packages..."
for package in "${REQUIRED_PACKAGES[@]}"; do
    if ! rpm -q $package > /dev/null 2>&1; then
        echo "Installing missing package: $package"
        sudo dnf install -y $package
    fi
done

# Remove old directories
rm -rf build

# Create required build directories
mkdir -p build/rpmbuild/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}

# Create build structure
cp -r * build/rpmbuild/BUILD/lh-1.0.0/

# Add `loghog.conf` file with proper log paths
echo -e "/var/log/messages\n/var/log/secure\n/var/log/audit/audit.log" > build/rpmbuild/BUILD/lh-1.0.0/loghog.conf

# Create the tarball
cd build/rpmbuild/SOURCES
tar czf lh-1.0.0.tar.gz -C ../BUILD lh-1.0.0

# Create the RPM spec file
cat > ../SPECS/lh.spec <<EOF
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
g++ -g -o lh main.cpp logs.cpp json_export.cpp utils.cpp -lreadline -ljson-c

%install
rm -rf %{buildroot}
install -d %{buildroot}/usr/bin
install -m 0755 lh %{buildroot}/usr/bin/lh
install -d %{buildroot}/etc
install -m 0644 loghog.conf %{buildroot}/etc/loghog.conf

%files
/usr/bin/lh
/etc/loghog.conf

%changelog
EOF

# Build the RPM
cd ..
rpmbuild --define "_topdir $(pwd)" -ba SPECS/lh.spec

# Copy the built RPM to the `rpms` directory
cd ../../
mkdir -p rpms
cp rpmbuild/RPMS/x86_64/lh-1.0.0-1.el8.x86_64.rpm rpms/

# Reinstall the RPM
sudo rpm -Uvh --replacepkgs --force rpms/lh-1.0.0-1.el8.x86_64.rpm
