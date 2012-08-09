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

import org.nemomobile.thumbnailer 1.0

Dialog {
    id:root

    property Dialog dtmfKeypadDialog

    signal privateClicked
    onPrivateClicked: {}

    state: manager.activeVoiceCall ? manager.activeVoiceCall.statusText : 'disconnected'

    states {
        State {name:'active'}
        State {name:'held'}
        State {name:'dialing'}
        State {name:'alerting'}
        State {name:'incoming'}
        State {name:'waiting'}
        State {name:'disconnected'}
    }

    Component {
        id:numPadDialog

        Dialog {
            width:parent.width;height:parent.height

            NumPad {
                id:dtmfpad
                anchors {
                    horizontalCenter:parent.horizontalCenter
                    bottom:parent.bottom
                    bottomMargin:100
                }
                mode:'dtmf'
                width:parent.width - 50;height:childrenRect.height
            }
        }
    }

    content: Column {
        Column {
            width:root.width
            anchors.horizontalCenter: parent.horizontalCenter

            Text {
                id:tLineId
                width:parent.width; height:paintedHeight
                color:main.appTheme.foregroundColor
                horizontalAlignment:Text.Center

                text:main.activeVoiceCallPerson
                     ? main.activeVoiceCallPerson.displayLabel
                     : (manager.activeVoiceCall ? manager.activeVoiceCall.lineId : '');

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
                asynchronous:true
                fillMode:Image.PreserveAspectFit
                smooth:true

                Rectangle {
                    anchors.fill:parent
                    border {color:main.appTheme.foregroundColor;width:2}
                    radius:10
                    color:'#00000000'
                }

                source: main.activeVoiceCallPerson
                        ? main.activeVoiceCallPerson.avatarPath
                        : 'image://theme/icon-m-telephony-contact-avatar';
            }

            Text {
                id:tVoiceCallDuration
                anchors.horizontalCenter:parent.horizontalCenter
                color:main.appTheme.foregroundColor
                font.pixelSize:18
                text:manager.activeVoiceCall ? main.secondsToTimeString(manager.activeVoiceCall.duration) : '00:00:00'
            }

            // Spacer
            Item {width:parent.width;height:15}

            Row {
                id:rVoiceCallTools
                anchors.horizontalCenter:parent.horizontalCenter
                spacing:5

                CallDialogToolButton {
                    visible:root.state == 'active'
                    iconSource:'images/icon-m-telephony-volume.svg'
                    onClicked: {
                        manager.audioMode = manager.audioMode == 'ihf' ? 'earpiece' : 'ihf';
                    }
                }

                CallDialogToolButton {
                    visible:root.state == 'incoming' || root.state == 'active'
                    iconSource:'images/icon-m-telephony-volume-off.svg'
                    onClicked: {
                        if(root.state == 'incoming') { // TODO: Take in to account unmuting audio when call is answered.
                            //manager.setMuteSpeaker(true);
                        } else {
                            manager.setMuteMicrophone(manager.muteMicrophone ? false : true);
                        }
                    }
                }

                CallDialogToolButton {
                    visible:root.state == 'active'
                    iconSource:'images/icon-m-telephony-pause.svg'
                    onClicked: manager.activeVoiceCall.hold();
                }

                CallDialogToolButton {
                    visible:root.state == 'active'
                    iconSource:'images/icon-m-telephony-numpad.svg'
                    onClicked: {
                        if(!root.dtmfKeypadDialog) {
                            root.dtmfKeypadDialog = numPadDialog.createObject(root);
                        }

                        root.dtmfKeypadDialog.open();
                    }
                }
            }

            // Spacer
            Item {width:parent.width;height:5}

            Item {
                width:parent.width;height:childrenRect.height
                Text {
                    id:tVoiceCallStatus
                    anchors.right:parent.right
                    color:main.appTheme.foregroundColor
                    font.pixelSize:18
                    text:qsTr(manager.activeVoiceCall ? manager.activeVoiceCall.statusText : 'disconnected')
                }
            }
        }

        // Spacer
        Item {width:parent.width;height:150}

        AcceptButton {
            visible:root.state == 'incoming'
            onClicked: if(manager.activeVoiceCall) manager.activeVoiceCall.answer();
        }

        // Spacer
        Item {width:parent.width;height:5}

        RejectButton {
            onClicked: {
                if(manager.activeVoiceCall) {
                    manager.activeVoiceCall.hangup();
                } else {
                    root.close();
                }
            }
        }
    }
}
