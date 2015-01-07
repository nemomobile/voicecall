Name:       voicecall-qt5
Summary:    Dialer engine for Nemo Mobile
Version:    0.0.0
Release:    1
Group:      Communications/Telephony
License:    Apache License, Version 2.0
URL:        http://github.com/nemomobile/voicecall
Source0:    %{name}-%{version}.tar.bz2
Source100:  voicecall-qt5.yaml
Requires:   systemd
Requires:   systemd-user-session-targets
Requires:   voicecall-qt5-plugin-telepathy = %{version}
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Multimedia)
BuildRequires:  pkgconfig(libresourceqt5)
BuildRequires:  pkgconfig(libpulse-mainloop-glib)
BuildRequires:  pkgconfig(ngf-qt5)
BuildRequires:  pkgconfig(qt5-boostable)

Provides:   voicecall-core >= 0.4.9
Provides:   voicecall-libs >= 0.4.9
Provides:   voicecall-plugin-pulseaudio >= 0.4.9
Provides:   voicecall-qt5-plugin-pulseaudio >= 0.5.1
Provides:   voicecall-plugin-resource-policy >= 0.4.9
Provides:   voicecall-qt5-plugin-resource-policy >= 0.5.1
Obsoletes:   voicecall-core < 0.4.9
Obsoletes:   voicecall-libs < 0.4.9
Obsoletes:   voicecall-plugin-pulseaudio < 0.4.9
Obsoletes:   voicecall-qt5-plugin-pulseaudio < 0.5.1
Obsoletes:   voicecall-plugin-resource-policy < 0.4.9
Obsoletes:   voicecall-qt5-plugin-resource-policy < 0.5.1

%description
%{summary}.

%package devel
Summary:    Voicecall development package
Group:      Communications/Telephony
Requires:   %{name} = %{version}-%{release}
Provides:   voicecall-devel >= 0.4.9
Obsoletes:  voicecall-devel < 0.4.9

%description devel
%{summary}.

%package plugin-telepathy
Summary:    Voicecall plugin for calls using telepathy
Group:      Communications/Telephony
Requires:   %{name} = %{version}-%{release}
Requires:   telepathy-ring
Conflicts:  voicecall-qt5-plugin-ofono
BuildRequires:  pkgconfig(TelepathyQt5)
BuildRequires:  pkgconfig(TelepathyQt5Farstream)

%description plugin-telepathy
%{summary}.

%package plugin-ofono
Summary:    Voicecall plugin for calls using ofono
Group:      Communications/Telephony
Requires:   %{name} = %{version}-%{release}
Provides:   voicecall-plugin-ofono >= 0.4.9
Conflicts:  voicecall-qt5-plugin-telepathy
Obsoletes:  voicecall-plugin-ofono < 0.4.9
BuildRequires:  pkgconfig(qofono-qt5)

%description plugin-ofono
%{summary}.

%prep
%setup -q -n %{name}-%{version}

%build

%qmake5 

make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install

mkdir -p %{buildroot}%{_libdir}/systemd/user/user-session.target.wants
ln -s ../voicecall-manager.service %{buildroot}%{_libdir}/systemd/user/user-session.target.wants/

%post
/sbin/ldconfig
if [ "$1" -ge 1 ]; then
systemctl-user daemon-reload || :
systemctl-user restart voicecall-manager.service || :
fi

%postun
/sbin/ldconfig
if [ "$1" -eq 0 ]; then
systemctl-user stop voicecall-manager.service || :
systemctl-user daemon-reload || :
fi

%files
%defattr(-,root,root,-)
%{_libdir}/libvoicecall.so.1
%{_libdir}/libvoicecall.so.1.0
%{_libdir}/libvoicecall.so.1.0.0
%{_libdir}/qt5/qml/org/nemomobile/voicecall/libvoicecall.so
%{_libdir}/qt5/qml/org/nemomobile/voicecall/qmldir
%{_bindir}/voicecall-manager
%{_libdir}/voicecall/plugins/libvoicecall-playback-manager-plugin.so
%{_libdir}/voicecall/plugins/libvoicecall-ngf-plugin.so
%{_libdir}/systemd/user/voicecall-manager.service
%{_libdir}/systemd/user/user-session.target.wants/voicecall-manager.service

%files devel
%defattr(-,root,root,-)
%{_libdir}/libvoicecall.so

%files plugin-telepathy
%defattr(-,root,root,-)
%{_libdir}/voicecall/plugins/libvoicecall-telepathy-plugin.so
# >> files plugin-telepathy
# << files plugin-telepathy

%files plugin-ofono
%defattr(-,root,root,-)
%{_libdir}/voicecall/plugins/libvoicecall-ofono-plugin.so

