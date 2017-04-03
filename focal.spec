Name: focal
Summary:  A FOCAL language interpreter.
Version: 0.0.0
Release: 0
License: GPL
Group: Development/Languages
URL: http://www.cozx.com/
Source0: %{name}-%{version}.tgz
Buildroot: %{_tmppath}/%{name}-%{version}-root

%description 

This package installs the FOCAL language interpreter.

%pre

%prep 

rm -rf %{buildroot}
if [ ! -d %{buildroot} ]; then
        mkdir -p %{buildroot}/usr/bin
        mkdir -p %{buildroot}/usr/share/man/man1
fi

%setup -q

%build

if [ ! -d %{buildroot}/usr ]; then
	mkdir %{buildroot}/usr
fi

%install 

cp bin/focal %{buildroot}/usr/bin/
cp share/man/man1/focal.1 %{buildroot}/usr/share/man/man1/

%clean 

if [ -d %{buildroot} ]; then
	rm -fr %{buildroot}
fi

%post 

%preun 

%postun 

%files 

%defattr(-,root,root) 
/usr/bin/focal
/usr/share/man/man1/focal.1.gz

%changelog 

* Wed Oct 26 2011  <dpitts@cozx.com>
- First RPM Build
