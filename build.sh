
### `build.sh`
```bash
#!/bin/bash

# Remove old directories
rm -rf build

# Create required build directories
mkdir -p build/rpmbuild/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}

# Download latest source code from GitHub
git clone https://github.com/unixbox-net/loghog.git
cd loghog

# Create build structure
mkdir -p ../build/rpmbuild/BUILD/lh-1.0.0
cp -r * ../build/rpmbuild/BUILD/lh-1.0.0/

# Add loghog.conf file if it doesn't exist
echo -e "/var/log/messages\n/var/log/secure\n/var/log/audit/audit.log" > ../build/rpmbuild/BUILD/lh-1.0.0/loghog.conf

# Create the tarball
cd ../build/rpmbuild/SOURCES
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
g++ -o lh main.cpp logs.cpp json_export.cpp utils.cpp -lreadline -ljson-c

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

# Reinstall the RPM
cd ../../rpms
mkdir -p ../build/rpms
cp ../rpmbuild/RPMS/x86_64/lh-1.0.0-1.el8.x86_64.rpm ../build/rpms/
rpm -Uvh --replacepkgs --force ../build/rpms/lh-1.0.0-1.el8.x86_64.rpm
