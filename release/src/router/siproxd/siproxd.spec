%define name		siproxd
%define ver		0.8.1
%define release		1
%define serial		1
%define prefix		%{_prefix}
%define sysconfdir	/etc
%define piddir		/var/run/siproxd/
%define regdir		/var/lib/siproxd/
%define siproxduser	%{name}

Name:		%{name}
Summary:	A SIP masquerading proxy with RTP support
Version: 	%{ver}
Release: 	%{release}
License: 	GPL, (c) 2002-2011 Thomas Ries
Group:		Applications/Communications
Source0: 	%{name}-%{ver}.tar.gz

URL: 		http://siproxd.sourceforge.net/
BuildRoot:	%{_tmppath}/%{name}-%{ver}-root

Requires:	libosip2 >= 3.0.0
BuildRequires:	libosip2 >= 3.0.0
BuildRequires:	libosip2-devel >= 3.0.0

Vendor:		Thomas Ries

Packager:       Thomas Ries <tries@gmx.net>

%description
Siproxd is a proxy/masquerading daemon for SIP (Session Initiation
Protocol), which is used in IP telephony.
It handles registrations of SIP clients on a private IP network
and performs rewriting of the SIP message bodies to make SIP
connections work via a masquerading firewall (NAT).
It allows SIP software clients (like kphone, linphone) or SIP
hardware clients (Voice over IP phones which are SIP-compatible,
such as those from Cisco, Grandstream or Snom) to work behind
an IP masquerading firewall or NAT router.


%prep
%setup -q


%build
CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%{prefix} \
                                    --sysconfdir=%{sysconfdir} \
                                    --disable-doc
make


%install
make prefix=$RPM_BUILD_ROOT%{prefix} \
     sysconfdir=$RPM_BUILD_ROOT%{sysconfdir} \
     install
mv %{buildroot}%{sysconfdir}/siproxd.conf.example \
   %{buildroot}%{sysconfdir}/siproxd.conf

# edit siproxd.conf and put correct username
sed -i "s/user\ =\ nobody/user\ =\ %{siproxduser}/" \
       %{buildroot}%{sysconfdir}/siproxd.conf

# will be added separately below
rm -r %{buildroot}%{_defaultdocdir}/%{name}

install -d %{buildroot}%{_initrddir}/
install contrib/siproxd.init %{buildroot}%{_initrddir}/siproxd

install -d %{buildroot}%{piddir}
install -d %{buildroot}%{regdir}


%clean
rm -rf %{buildroot}


%files
%defattr(-, root, root)
%doc COPYING README AUTHORS INSTALL NEWS ChangeLog
%doc doc/FAQ doc/FLI4L_HOWTO.txt doc/sample_*.txt

%attr(0755,root,root) %{_sbindir}/siproxd
%config %{sysconfdir}/siproxd.conf
%config %{sysconfdir}/siproxd_passwd.cfg
%attr(0755,root,root) %{_initrddir}/siproxd
%attr(0755,root,root) %{_libdir}/%{name}/

%attr(0700,%{siproxduser},root) %{piddir}
%attr(0700,%{siproxduser},root) %{regdir}


%pre
getent group  %{siproxduser} &>/dev/null || groupadd -r %{siproxduser}
getent passwd %{siproxduser} &>/dev/null || \
    /usr/sbin/useradd -r -g %{siproxduser} -s /sbin/nologin -c %{siproxduser} \
    -d %{regdir} %{siproxduser}


%post
echo "Edit the config file %{sysconfigdir}/siproxd.conf!"
/sbin/chkconfig --add %{name}


%preun
if [ $1 = 0 ]; then
   /sbin/service %{name} stop
   /sbin/chkconfig --del %{name}
fi


%changelog
* Fri Oct 09 2004 Thomas Ries <tries@gmx.net>
- startup script in /etc/rc.d/init.d/siproxd
- create directories for PID and registration files

* Fri Oct 31 2003 Thomas Ries <tries@gmx.net>
- siproxd is now installed to sbin directory

* Fri Oct 24 2003 Thomas Ries <tries@gmx.net>
- added config files and some more doc files
- rename sample config file and remind user to edit it.
- minor cleanup

* Sat Aug 30 2003 Thomas Ries <tries@gmx.net>
- always use /etc as sysconfdir

* Sat Sep 21 2002 Thomas Ries <tries@gmx.net>
- first RPM support

