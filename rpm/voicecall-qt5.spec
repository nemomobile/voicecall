# 
# Do NOT Edit the Auto-generated Part!
# Generated by: spectacle version 0.27
# 

Name:       voicecall-qt5

# >> macros
# << macros

Summary:    Dialer engine for Nemo Mobile
Version:    0.0.0
Release:    1
Group:      Communications/Telephony
License:    Apache License, Version 2.0
URL:        http://github.com/nemomobile/voicecall
Source0:    %{name}-%{version}.tar.bz2
Source100:  voicecall-qt5.yaml
Requires:   telepathy-ring
Requires:   systemd
Requires:   systemd-user-session-targets
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Multimedia)
BuildRequires:  pkgconfig(libresourceqt5)
BuildRequires:  pkgconfig(libpulse-mainloop-glib)
BuildRequires:  pkgconfig(qofono-qt5)
BuildRequires:  pkgconfig(commhistory-qt5)
BuildRequires:  pkgconfig(TelepathyQt5)
BuildRequires:  pkgconfig(TelepathyQt5Farstream)
BuildRequires:  pkgconfig(ngf-qt5)
Provides:   voicecall-core >= 0.4.9
Provides:   voicecall-libs >= 0.4.9
Obsoletes:   voicecall-core < 0.4.9
Obsoletes:   voicecall-libs < 0.4.9

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

%package plugin-ofono
Summary:    Voicecall plugin for calls using ofono
Group:      Communications/Telephony
Requires:   %{name} = %{version}-%{release}
Provides:   voicecall-plugin-ofono >= 0.4.9
Obsoletes:  voicecall-plugin-ofono < 0.4.9

%description plugin-ofono
%{summary}.

#%package plugin-pulseaudio
#Summary:    Voicecall plugin for direct pulseaudio audio routing and stream control.
#Group:      Communications/Telephony
#Requires:   %{name} = %{version}-%{release}
#Provides:   voicecall-plugin-pulseaudio >= 0.4.9
#Obsoletes:  voicecall-plugin-pulseaudio < 0.4.9

#%description plugin-pulseaudio
#%{summary}.

%package plugin-resource-policy
Summary:    Voicecall plugin for resource policy audio routing and stream control.
Group:      Communications/Telephony
Requires:   %{name} = %{version}-%{release}
Provides:   voicecall-plugin-resource-policy >= 0.4.9
Obsoletes:  voicecall-plugin-resource-policy < 0.4.9

%description plugin-resource-policy
%{summary}.

%prep
%setup -q -n %{name}-%{version}

# >> setup
# << setup

%build
# >> build pre
# << build pre

%qmake5 

make %{?jobs:-j%jobs}

# >> build post
# << build post

%install
rm -rf %{buildroot}
# >> install pre
# << install pre
%qmake5_install

# >> install post
mkdir -p %{buildroot}%{_libdir}/systemd/user/user-session.target.wants
ln -s ../voicecall-manager.service %{buildroot}%{_libdir}/systemd/user/user-session.target.wants/
# << install post

%post
/sbin/ldconfig
# >> post
if [ "$1" -ge 1 ]; then
systemctl-user daemon-reload || :
systemctl-user restart voicecall-manager.service || :
fi
# << post

%postun
/sbin/ldconfig
# >> postun
if [ "$1" -eq 0 ]; then
systemctl-user stop voicecall-manager.service || :
systemctl-user daemon-reload || :
fi
# << postun

%files
%defattr(-,root,root,-)
%{_libdir}/libvoicecall.so.1
%{_libdir}/libvoicecall.so.1.0
%{_libdir}/libvoicecall.so.1.0.0
%{_libdir}/qt5/qml/org/nemomobile/voicecall/libvoicecall.so
%{_libdir}/qt5/qml/org/nemomobile/voicecall/qmldir
%{_bindir}/voicecall-manager
%{_libdir}/voicecall/plugins/libvoicecall-playback-manager-plugin.so
%{_libdir}/voicecall/plugins/libvoicecall-telepathy-plugin.so
%{_libdir}/voicecall/plugins/libvoicecall-ngf-plugin.so
%{_libdir}/voicecall/plugins/libvoicecall-commhistory-plugin.so
%config %{_sysconfdir}/xdg/autostart/voicecall-manager.desktop
%{_libdir}/systemd/user/voicecall-manager.service
# >> files
%{_libdir}/systemd/user/user-session.target.wants/voicecall-manager.service
# << files

%files devel
%defattr(-,root,root,-)
%{_libdir}/libvoicecall.so
# >> files devel
# << files devel

%files plugin-ofono
%defattr(-,root,root,-)
%{_libdir}/voicecall/plugins/libvoicecall-ofono-plugin.so
# >> files plugin-ofono
# << files plugin-ofono

%files plugin-pulseaudio
%defattr(-,root,root,-)
%{_libdir}/voicecall/plugins/libvoicecall-pulseaudio-plugin.so
%config %{_sysconfdir}/voicecall/modes.ini
# >> files plugin-pulseaudio
# << files plugin-pulseaudio

%files plugin-resource-policy
%defattr(-,root,root,-)
%{_libdir}/voicecall/plugins/libvoicecall-resource-policy-routing-plugin.so
# >> files plugin-resource-policy
# << files plugin-resource-policy
