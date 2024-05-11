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

%changelog
* Fri May 11 2024 Your Name <you@example.com> 1.0.0-1
- Initial RPM release
