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

Dialog {
    id:root

    signal privateClicked
    onPrivateClicked: {}

    state: main.activeVoiceCall ? main.activeVoiceCall.statusText : 'disconnected'

    states {
        State {name:'active'}
        State {name:'held'}
        State {name:'dialing'}
        State {name:'alerting'}
        State {name:'incoming'}
        State {name:'waiting'}
        State {name:'disconnected'}
    }

    content: Column {
        Column {
            anchors.horizontalCenter: parent.horizontalCenter

            Text {
                id:tLineId
                width:parent.width; height:paintedHeight
                color:'#ffffff'
                horizontalAlignment:Text.Center
                text: main.activeVoiceCall ? main.activeVoiceCall.lineId : '<unknown>'
                onTextChanged: resizeText();

                Component.onCompleted: resizeText();

                function resizeText() {
                    if(paintedWidth < 0 || paintedHeight < 0) return;
                    while(paintedWidth > width)
                        if(--font.pixelSize <= 0) break;

                    while(paintedWidth < width)
                        if(++font.pixelSize >= 38) break;
                }
            }

            // Spacer
            Item {width:parent.width;height:10}

            Image {
                id:iAvatar
                anchors.horizontalCenter:parent.horizontalCenter
                width:196;height:width
                smooth:true
                Rectangle {
                    anchors.fill:parent
                    color:'#6f6f6f'
                    border {color:'#ffffff';width:2}
                    radius:10
                }
            }

            Text {
                id:tVoiceCallDuration
                anchors.horizontalCenter:parent.horizontalCenter
                color:'#ffffff'
                font.pixelSize:18
                text:main.activeVoiceCall ? main.secondsToTimeString(main.activeVoiceCall.duration) : '00:00:00'
            }

            // Spacer
            Item {width:parent.width;height:15}

            Row {
                id:rVoiceCallTools
                anchors.horizontalCenter:parent.horizontalCenter
                width:childrenRect.width
                spacing:5
                visible:root.state == 'active'

                Button {
                    width:72;height:48
                    platformStyle: ButtonStyle {
                        background:'image://theme/color9-meegotouch-button-accent-background'
                        pressedBackground:'image://theme/color9-meegotouch-button-accent-background-pressed'
                    }
                    text:qsTr('LS');
                    onClicked: main.activeVoiceCall.toggleSpeaker();
                }

                Button {
                    width:72;height:48
                    platformStyle: ButtonStyle {
                        background:'image://theme/color9-meegotouch-button-accent-background'
                        pressedBackground:'image://theme/color9-meegotouch-button-accent-background-pressed'
                    }
                    text:qsTr('MU');
                    onClicked: main.activeVoiceCall.mute();
                }

                Button {
                    width:72;height:48
                    platformStyle: ButtonStyle {
                        background:'image://theme/color9-meegotouch-button-accent-background'
                        pressedBackground:'image://theme/color9-meegotouch-button-accent-background-pressed'
                    }
                    text:qsTr('HL');
                    onClicked: main.activeVoiceCall.hold();
                }

                Button {
                    width:72;height:48
                    platformStyle: ButtonStyle {
                        background:'image://theme/color9-meegotouch-button-accent-background'
                        pressedBackground:'image://theme/color9-meegotouch-button-accent-background-pressed'
                    }
                    text:qsTr('NM');
                    onClicked: console.log('SHOW NUMPAD');
                }
            }

            // Spacer
            Item {width:parent.width;height:5}

            Item {
                width:rVoiceCallTools.width;height:childrenRect.height
                Text {
                    id:tVoiceCallStatus
                    anchors.right:parent.right
                    color:'#ffffff'
                    text:qsTr(main.activeVoiceCall ? main.activeVoiceCall.statusText : 'disconnected')
                }
            }
        }

        // Spacer
        Item {width:parent.width;height:150}

        Button {
            platformStyle: ButtonStyle {
                background: 'images/meegotouch-button-positive-background.svg'
                pressedBackground: 'images/meegotouch-button-positive-background-pressed.svg'
            }
            iconSource:'image://theme/icon-m-telephony-call'
            visible:root.state == 'incoming'
            onClicked: if(main.activeVoiceCall) main.activeVoiceCall.answer();
        }

        // Spacer
        Item {width:parent.width;height:5}

        Button {
            platformStyle: ButtonStyle {
                background:'images/meegotouch-button-negative-background.svg'
                pressedBackground:'images/meegotouch-button-negative-background-pressed.svg'
            }
            iconSource:'image://theme/icon-m-telephony-call-end';
            onClicked: {
                if(main.activeVoiceCall) {
                    main.activeVoiceCall.hangup();
                } else {
                    root.close();
                }
            }
        }
    }
}
