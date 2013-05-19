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

Item {
    id:root

    property alias text:textedit.text
    property alias color:textedit.color
    property alias alignment:textedit.horizontalAlignment
    property alias inputMethodHints:textedit.inputMethodHints

    property string __previousCharacter

    function insertChar(character) {
        var text = textedit.text
        var cpos = textedit.cursorPosition;

        if(text.length == 0) {
            textedit.text = character
            textedit.cursorPosition = textedit.text.length
        } else {
            textedit.text = text.slice(0, cpos) + character + text.slice(cpos,text. length);
            textedit.cursorPosition = cpos + 1;
        }

        root.__previousCharacter = character;
        interactionTimeout.restart();
    }

    function backspace() {
        var cpos = textedit.cursorPosition == 0 ? 1 : textedit.cursorPosition;
        var text = textedit.text

        if(text.length == 0) return;

        textedit.text = text.slice(0, cpos - 1) + text.slice(cpos, text.length)
        textedit.cursorPosition = cpos - 1;

        root.__previousCharacter = '';
        interactionTimeout.restart();
    }

    function resetCursor() {
        textedit.cursorPosition = textedit.text.length;
        textedit.cursorVisible = false;
    }

    function clear() {

        resetCursor();
        textedit.text = '';
    }

    Timer {
        id:interactionTimeout
        interval:4000
        running:false
        repeat:false
        onTriggered:root.resetCursor();
    }

    Image {
        id:backspace

        anchors {verticalCenter:parent.verticalCenter;right:parent.right; margins:34}
        source: 'image://theme/icon-m-common-backspace'

        MouseArea {
            anchors.fill:parent

            onClicked: root.backspace();
            onPressAndHold: root.clear();
        }
    }

    TextEdit {
        id:textedit

        anchors {
            left:parent.left;right:backspace.left
            leftMargin:30;rightMargin:20
            verticalCenter:parent.verticalCenter
        }

        activeFocusOnPress: false
        cursorVisible:false
        inputMethodHints:Qt.ImhDialableCharactersOnly
        font.pixelSize:64//TODO:Theme
        horizontalAlignment:TextEdit.AlignRight

        onTextChanged:__resizeText();

        function __resizeText() {
            if(paintedWidth < 0 || paintedHeight < 0) return;

            while(paintedWidth > width) {
                if(font.pixelSize <= 0) break;
                font.pixelSize--;
            }

            while(paintedWidth < width) {
                if(font.pixelSize >= 72) break;
                font.pixelSize++;
            }
        }
    }

    MouseArea {
        anchors.fill:textedit

        onPressed: {
            textedit.cursorVisible = true;
            interactionTimeout.restart();
            mouse.accepted = false;
        }
    }
}
