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
import com.nokia.meego 1.1

Page {
    id:root

    orientationLock:PageOrientation.LockPortrait

    SelectionDialog {
        id:dProviderSelect
        model:providers

        onSelectedIndexChanged: {
            main.providerId = model.id(selectedIndex);
            main.providerType = model.type(selectedIndex);
            main.providerLabel = model.label(selectedIndex);
        }

        Component.onCompleted: selectedIndex = 0;
    }

    Button {
        id:bProviderSelect
        width:parent.width;height:64
        text:main.providerLabel
        onClicked:dProviderSelect.open();
    }

    TextEdit {
        id:iNumberEntry
        anchors {top:bProviderSelect.bottom;left:parent.left;right:bBackspace.left;bottom:numpad.top;margins:10}
        readOnly:true
        inputMethodHints:Qt.ImhDialableCharactersOnly
        color:'#ffffff'
        font.pixelSize:64
        horizontalAlignment:TextEdit.AlignRight

        /*
        placeholderText:qsTr('Enter Number')
        platformStyle: TextFieldStyle {
            background:null
            backgroundSelected:null
            backgroundDisabled:null
            backgroundError:null
        }
        */

        onTextChanged: {
            resizeText();
        }

        function resizeText() {
            if(paintedWidth < 0 || paintedHeight < 0) return;
            while(paintedWidth > width)
            {
                if(font.pixelSize <= 0) break;
                font.pixelSize--;
            }

            while(paintedWidth < width)
            {
                if(font.pixelSize >= 64) break;
                font.pixelSize++;
            }
        }


        function appendChar(character)
        {
            if(iNumberEntry.text.length == 0) {
                iNumberEntry.text = character
            } else {
                iNumberEntry.text += character
            }
        }
    }

    Image {
        id:bBackspace
        anchors {top:bProviderSelect.bottom;right:parent.right; margins:34}
        source:'images/icon-m-common-backspace.svg'
        MouseArea {
            anchors.fill:parent

            onClicked: {
                if(iNumberEntry.text.length > 0) {
                    iNumberEntry.text = iNumberEntry.text.substring(0, iNumberEntry.text.length - 1);
                }
            }
            onPressAndHold: {
                if(iNumberEntry.text.length > 0) {
                    iNumberEntry.clear();
                }
            }
        }
    }

    GridView {
        id:numpad
        width:parent.width - 20; height:cellHeight * 4
        anchors {bottom:rCallActions.top;horizontalCenter:parent.horizontalCenter;margins:10}
        interactive:false

        cellWidth:(parent.width - 20) / 3
        cellHeight:cellWidth * 0.6

        model: ListModel {
            ListElement {key:'1';sub:'voicemail'}
            ListElement {key:'2';sub:'abc'}
            ListElement {key:'3';sub:'def'}
            ListElement {key:'4';sub:'ghi'}
            ListElement {key:'5';sub:'jkl'}
            ListElement {key:'6';sub:'mno'}
            ListElement {key:'7';sub:'pqrs'}
            ListElement {key:'8';sub:'tuv'}
            ListElement {key:'9';sub:'wxyz'}
            ListElement {key:'*';sub:'+';alt:'+'}
            ListElement {key:'0'}
            ListElement {key:'#';alt:'p'}
        }

        delegate: Item {
            width:numpad.cellWidth;height:numpad.cellHeight

            Text {
                id:tKeyText
                anchors.centerIn: parent
                color:'#ffffff'
                font.pixelSize:28
                text:model.key
            }
            Text {
                id:tSubText
                anchors {horizontalCenter:parent.horizontalCenter;top:tKeyText.bottom}
                color:'#6f6f6f'
                text:model.sub
            }
            MouseArea {
                anchors.fill:parent
                onClicked:
                {
                    iNumberEntry.appendChar(model.key);
                }
                onPressAndHold:
                {
                    iNumberEntry.appendChar(model.alt || model.key);
                }
                onPressed:
                {
                    VoiceCallManager.startDtmfTone(model.key, 100);
                }
                onReleased:
                {
                    VoiceCallManager.stopDtmfTone();
                }
            }
        }
    }

    Row {
        id:rCallActions
        width:childrenRect.width;height:childrenRect.height
        anchors {bottom:parent.bottom;horizontalCenter:parent.horizontalCenter;margins:10}

        Button {
            id:bCallNumber
            width:root.width / 2; height:72
            iconSource:'image://theme/icon-m-telephony-call';
            platformStyle: ButtonStyle {
                background: 'images/meegotouch-button-positive-background.svg'
                pressedBackground: 'images/meegotouch-button-positive-background-pressed.svg'
            }
            onClicked: {
                if(iNumberEntry.text.length > 0) {
                    main.dial(iNumberEntry.text);
                } else {
                    //TODO: Popup number error message.
                }
            }
        }
    }
}
