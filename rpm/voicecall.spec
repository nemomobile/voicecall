Name:       voicecall
Summary:    Voice Call Suite
Version:    0.4.0
Release:    1
Group:      Communications/Telephony and IM
License:    Apache License, Version 2.0
URL:        http://github.com/nemomobile/voicecall
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  pkgconfig(QtOpenGL)
BuildRequires:  pkgconfig(QtDeclarative)
BuildRequires:  pkgconfig(QtMultimediaKit)
BuildRequires:  pkgconfig(libresourceqt1)
BuildRequires:  pkgconfig(libpulse-mainloop-glib)
BuildRequires:  pkgconfig(ofono-qt)
# May work with earlier version, but this is tested against in nemo.
# Requires header includes to be under TelepathyQt/ rather than TelepathyQt4/
BuildRequires:  pkgconfig(TelepathyQt4) >= 0.9.3
BuildRequires:  pkgconfig(TelepathyQt4Farstream)
BuildRequires:  pkgconfig(ngf-qt)
BuildRequires:  pkgconfig(qdeclarative-boostable)
Requires:  mapplauncherd-booster-qtcomponents

%description
Next Generation Dialer Application for Nemo Mobile

%package libs
Summary: Core voicecall libraries
Group: Communications/Telephony and IM

%description libs
Core voicecall libraries

%package devel
Requires: voicecall-libs == %{version}-%{release}
Summary: Voicecall development package
Group: Communications/Telephony and IM

%description devel
Voicecall development libraries and headers

%package core
Requires: voicecall-libs == %{version}-%{release}
Requires: telepathy-ring
Summary: Core voicecall service and libraries
Group: Communications/Telephony and IM

%description core
Core voicecall service executable, core plugins and library

%package plugin-ofono
Requires: voicecall-core == %{version}-%{release}
Summary: Voicecall ofono plugin provider
Group: Communications/Telephony and IM

%description plugin-ofono
Voicecall manager plugin for using ofono to make and receive calls.

%package plugin-pulseaudio
Requires: voicecall-core == %{version}-%{release}
Summary: Voicecall plugin for pulseaudio interaction
Group: Communications/Telephony and IM

%description plugin-pulseaudio
Voicecall manager plugin for direct pulseaudio audio routing and stream control.

%package plugin-resource-policy
Requires: voicecall-core == %{version}-%{release}
Summary: Voicecall plugin for resource policy audio routing
Group: Communications/Telephony and IM

%description plugin-resource-policy
Voicecall manager plugin for direct resource policy based audio routing and stream control.

%package ui-reference
Provides: meego-handset-dialer >= 0.2.4
Obsoletes: meego-handset-dialer < 0.2.4
Provides: voicecall == %{version}-%{release}
Obsoletes: voicecall <= 0.1.5
Requires: voicecall-core == %{version}-%{release}
Requires: tone-generator
Requires: commhistory-daemon
Requires: libcommhistory-declarative
Summary: Voicecall reference QML user interface
Group: Communications/Telephony and IM

%description ui-reference
Voicecall reference QML phone application user interface

%prep
%setup -q -n %{name}-%{version}

%build
unset LD_AS_NEEDED
%qmake 
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%qmake_install

%post libs
/sbin/ldconfig

%postun libs
/sbin/ldconfig

%files libs
%defattr(-,root,root,-)
%{_libdir}/libvoicecall.so.1
%{_libdir}/libvoicecall.so.1.0
%{_libdir}/libvoicecall.so.1.0.0
%{_libdir}/qt4/imports/stage/rubyx/voicecall/libvoicecall.so
%{_libdir}/qt4/imports/stage/rubyx/voicecall/qmldir

%files core
%defattr(-,root,root,-)
%{_bindir}/voicecall-manager
%{_libdir}/voicecall/plugins/libvoicecall-playback-manager-plugin.so
%{_libdir}/voicecall/plugins/libvoicecall-telepathy-plugin.so
%{_libdir}/voicecall/plugins/libvoicecall-ngf-plugin.so
%config %{_sysconfdir}/xdg/autostart/voicecall-manager.desktop
%{_libdir}/systemd/user/voicecall-manager.service

%files devel
%defattr(-,root,root,-)
%{_libdir}/libvoicecall.so

%files plugin-ofono
%defattr(-,root,root,-)
%{_libdir}/voicecall/plugins/libvoicecall-ofono-plugin.so
#TODO: Add voicecall-manager -reconfigure to post/un when implemented in core.

%files plugin-pulseaudio
%defattr(-,root,root,-)
%{_libdir}/voicecall/plugins/libvoicecall-pulseaudio-plugin.so
%config %{_sysconfdir}/voicecall/modes.ini

%files plugin-resource-policy
%defattr(-,root,root,-)
%{_libdir}/voicecall/plugins/libvoicecall-resource-policy-routing-plugin.so

%files ui-reference
%defattr(-,root,root,-)
%{_bindir}/voicecall-ui
%{_datadir}/voicecall-ui/qml
%{_datadir}/applications/voicecall-ui.desktop
%config %{_sysconfdir}/xdg/autostart/voicecall-ui-prestart.desktop
%{_libdir}/systemd/user/voicecall-ui-prestart.service
