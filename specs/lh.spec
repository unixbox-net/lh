Name: lh
Version: 1.0.0
Release: 1%{?dist}
Summary: Log monitoring tool

License: GPL
Source0: %{name}-%{version}.tar.gz

BuildRequires: gcc, make, readline-devel, json-c-devel
Requires: readline, json-c

%description
A simple tool to monitor and manage logs.

%prep
%setup -q

%build
make

%install
make install DESTDIR=%{buildroot}

%files
/usr/local/bin/lh

%changelog
* Date Your Name <email@example.com> - 1.0.0-1
- Initial RPM release
