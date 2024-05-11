Name:           lh
Version:        1.0.0
Release:        1%{?dist}
Summary:        A simple tool to monitor logs

License:        MIT
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc, make, readline-devel, json-c-devel
Requires:       readline, json-c

%description
lh (LogHog) is a simple command-line utility to monitor and manage logs.

%prep
%setup -q

%build
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/local/bin
install -m 755 lh %{buildroot}/usr/local/bin/

%files
/usr/local/bin/lh

# Correcting the tarball creation process
echo "Creating tarball with correct directory structure..."
mkdir -p lh-1.0.0
cp -a src/* lh-1.0.0/
tar czf SOURCES/lh-1.0.0.tar.gz lh-1.0.0 || { echo "Failed to create source tarball. Exiting."; exit 1; }
rm -rf lh-1.0.0

# Example changelog entry in lh.spec
%changelog
* Fri May 10 2024 Your Name <you@example.com> - 1.0.0-1
- Initial RPM release of lh
