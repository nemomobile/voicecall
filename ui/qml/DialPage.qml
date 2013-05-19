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
import QtQuick 1.1
import com.nokia.meego 1.2

Page {
    id:root

    orientationLock:PageOrientation.LockPortrait

    property alias numberEntryText: numentry.text

    SelectionDialog {
        id:dProviderSelect
        model:main.manager.providers

        onSelectedIndexChanged: {
            main.providerId = model.id(selectedIndex);
            main.providerType = model.type(selectedIndex);
            main.providerLabel = model.label(selectedIndex);
        }

        Component.onCompleted: selectedIndex = 0;
    }

    PageHeader {
        id:bProviderSelect
        text: main.providerLabel
        color: "#228B22"

        MouseArea {
            anchors.fill: parent
            onClicked: dProviderSelect.open();
        }
    }

    NumberEntry {
        id:numentry
        anchors {
            top:bProviderSelect.bottom;bottom:numpad.top
            left:parent.left;right:parent.right
        }
        color:'#ffffff'
    }

    NumPad {
        id:numpad
        width:root.width;height:childrenRect.height
        anchors {bottom:rCallActions.top;margins:20}
        entryTarget:numentry
    }

    Row {
        id:rCallActions
        width:childrenRect.width;height:childrenRect.height
        anchors {bottom:parent.bottom;horizontalCenter:parent.horizontalCenter;margins:30}

        AcceptButton {
            id:bCallNumber
            width:numpad.width * 0.66
            onClicked: {
                if(numentry.text.length > 0) {
                    main.dial(numentry.text);
                } else {
                    console.log('*** QML *** VCI WARNING: Number entry is blank.');
                }
            }
        }
    }
}
