Name:           lh
Version:        1.0.0
Release:        1%{?dist}
Summary:        Lightweight log monitoring tool
License:        MIT
URL:            https://github.com/unixbox-net/lh
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  gcc, make

%description
lh is a lightweight log monitoring tool designed for easy log file monitoring.

%prep
%setup -q

%build
make

%install
make install DESTDIR=%{buildroot}

%files
/usr/local/bin/lh

%changelog
* Sat May 11 2024 Your Name <you@example.com> - 1.0.0-1
- Initial release
