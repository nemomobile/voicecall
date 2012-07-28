/****************************************************************************
**
** Copyright (C) 2011-2012 Tom Swindell <t.swindell@rubyx.co.uk>
** All rights reserved.
**
** This file is part of the Voice Call UI project.
**
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * The names of its contributors may NOT be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
****************************************************************************/
#include <qtsingleapplication.h>
#include "declarativeview.h"

#include <QtDeclarative>

int main(int argc, char **argv)
{
    QtSingleApplication app(argc, argv);

    if(!app.arguments().contains("-devel"))
    {
        app.setQuitOnLastWindowClosed(false);
    }

    if(app.isRunning())
    {
        app.sendMessage("invoke");
        return EXIT_SUCCESS;
    }

    DeclarativeView view;
    for(int i = app.arguments().indexOf("-I"); i < app.arguments().count();)
    {
        if(app.arguments().value(i) == "-I" && app.arguments().count() > i + 1)
        {
            QDir importPath(app.arguments().value(i + 1));

            if(importPath.exists())
            {
                qDebug() << "Adding import path;" << importPath.absolutePath();
                view.engine()->addImportPath(importPath.absolutePath());
                i += 2;
            }
        }

        i++;
    }

    if(!app.arguments().contains("-prestart"))
    {
        view.rootContext()->setContextProperty("activationReason", "invoked");
        view.show();
    }

    int paramIndex = app.arguments().indexOf("-qml");
    if(paramIndex != -1 && app.arguments().count() > paramIndex + 1)
    {
        view.setSource(QUrl::fromLocalFile(app.arguments().value(paramIndex + 1)));
    }
    else
    {
        view.setSource(QUrl::fromLocalFile("/usr/share/voicecall-ui/qml/main.qml"));
    }

    app.setActivationWindow(&view);
    return app.exec();
}
