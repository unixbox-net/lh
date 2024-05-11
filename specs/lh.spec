Name:           lh
Version:        1.0.0
Release:        1%{?dist}
Summary:        A new Linux command tool

License:        MIT
URL:            https://github.com/unixbox-net/lh
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc, make, readline-devel, json-c-devel
Requires:       readline, json-c

%description
lh (LogHog) is a new Linux command tool for monitoring and managing logs efficiently.

%prep
echo "Starting prep stage..."
%setup -q
echo "Finished prep stage."

%build
echo "Starting build stage..."
make %{?_smp_mflags}
echo "Finished build stage."

%install
echo "Starting install stage..."
make install DESTDIR=%{buildroot}
echo "Finished install stage."

%files
%{_bindir}/lh

%changelog
* Fri May 11 2024 Your Name <you@example.com> - 1.0.0-1
- Initial RPM release
