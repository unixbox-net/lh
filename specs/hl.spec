Name:           lh
Version:        1.0.0
Release:        1%{?dist}
Summary:        Log monitoring tool
License:        MIT
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
A tool to monitor and search logs.

%prep
%setup -q

%build
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/local/bin
install -m 755 lh %{buildroot}/usr/local/bin

%files
/usr/local/bin/lh

%clean
rm -rf %{buildroot}

%changelog
* Wed Oct 04 2023 Your Name <you@example.com> - 1.0.0-1
- First build of lh
