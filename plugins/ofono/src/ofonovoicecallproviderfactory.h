/*
 * This file is a part of the Voice Call Manager Ofono Plugin project.
 *
 * Copyright (C) 2011-2012  Tom Swindell <t.swindell@rubyx.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#ifndef OFONOVOICECALLPROVIDERFACTORY_H
#define OFONOVOICECALLPROVIDERFACTORY_H

#include <abstractvoicecallmanagerplugin.h>

class OfonoVoiceCallProviderFactory : public AbstractVoiceCallManagerPlugin
{
    Q_OBJECT
    Q_INTERFACES(AbstractVoiceCallManagerPlugin)
    Q_PLUGIN_METADATA(IID "org.nemomobile.voicecall.ofono")
public:
    explicit OfonoVoiceCallProviderFactory(QObject *parent = 0);
            ~OfonoVoiceCallProviderFactory();

    QString pluginId() const;
    QString pluginVersion() const;

public Q_SLOTS:
    bool initialize();
    bool configure(VoiceCallManagerInterface *manager);
    bool start();
    bool suspend();
    bool resume();
    void finalize();

protected Q_SLOTS:
    void onModemAdded(const QString &modemPath);
    void onModemRemoved(const QString &modemPath);

private:
    class OfonoVoiceCallProviderFactoryPrivate *d_ptr;

    Q_DECLARE_PRIVATE(OfonoVoiceCallProviderFactory)
};

#endif // OFONOVOICECALLPROVIDERFACTORY_H
