Name:           lh
Version:        1.0.0
Release:        1%{?dist}
Summary:        Lightweight log monitoring tool
License:        MIT
URL:            https://github.com/unixbox-net/lh
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  gcc, json-c-devel, readline-devel

%description
lh is a lightweight log monitoring tool designed for easy log file monitoring.

%prep
%setup -q

%build
gcc -o lh lh.c -Wall -lreadline -ljson-c

%install
mkdir -p %{buildroot}/usr/bin
mkdir -p %{buildroot}/usr/share/doc/%{name}-%{version}
install -m 0755 lh %{buildroot}/usr/bin/lh
install -m 0644 LICENSE %{buildroot}/usr/share/doc/%{name}-%{version}
install -m 0644 README.md %{buildroot}/usr/share/doc/%{name}-%{version}

%files
/usr/bin/lh
%doc /usr/share/doc/%{name}-%{version}/LICENSE
%doc /usr/share/doc/%{name}-%{version}/README.md

%changelog
* Sat May 11 2024 Your Name <you@example.com> - 1.0.0-1
- Initial release