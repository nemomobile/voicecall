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

import stage.rubyx.voicecall 1.0

PageStackWindow {
    id:main

    showToolBar:true
    showStatusBar:true

    property VoiceCallUiTheme appTheme: VoiceCallUiTheme {}

    property string activationReason: 'invoked'

    property string providerId
    property string providerType
    property string providerLabel

    property VoiceCallManager manager: VoiceCallManager {
        id:manager

        onActiveVoiceCallChanged: {
            if(activeVoiceCall) {
                dActiveCall.open();

                if(!__window.visible)
                {
                    main.activationReason = 'activeVoiceCallChanged';
                    __window.show();
                }
            }
            else
            {
                pDialPage.numberEntryText = '';
                dActiveCall.close();

                if(main.activationReason != "invoked")
                {
                    main.activationReason = 'invoked'; // reset for next time
                    __window.close();
                }
            }
        }

        onError: {
            console.log('*** QML *** VCM ERROR: ' + message);
            dErrorDialog.message = message;
            dErrorDialog.open();
        }
    }

    function dial(msisdn) {
        manager.dial(providerId, msisdn);
    }

    function secondsToTimeString(seconds) {
        var h = Math.floor(seconds / 3600);
        var m = Math.floor((seconds - (h * 3600)) / 60);
        var s = seconds - h * 3600 - m * 60;
        if(h < 10) h = '0' + h;
        if(m < 10) m = '0' + m;
        if(s < 10) s = '0' + s;
        return '' + h + ':' + m + ':' + s;
    }

    initialPage: pDialPage

    Component.onCompleted: {
        theme.inverted = true;
    }


    Keys.onPressed: {
        if(event.key == Qt.Key_Phone) {
            console.log("*** QML *** Detected phone key press, processing.");

            if(manager.activeVoiceCall) {
                var call = manager.activeVoiceCall;

                if(call.status == VoiceCall.STATUS_INCOMING) {
                    call.answer();
                } else {
                    call.hangup();
                }
            } else if(pDialPage.numberEntryText.length > 0) {
                main.dial(pDialPage.numberEntryText);
            }
        }
    }

    QueryDialog {
        id:dErrorDialog
        titleText:qsTr('Error')
        acceptButtonText:qsTr('OK')
    }

    ActiveCallDialog {id:dActiveCall}

    DialPage {id:pDialPage;tools:toolbar}
    HistoryPage {id:pHistoryPage;tools:toolbar}

    ToolBarLayout {
        id:toolbar

        ButtonRow {
            TabButton {
                iconSource:'images/icon-m-telephony-numpad.svg'
                onClicked:
                {
                    main.pageStack.replace(pDialPage);
                }
            }
            TabButton {
                iconSource:'image://theme/icon-m-toolbar-callhistory-white'
                onClicked:
                {
                    main.pageStack.replace(pHistoryPage);
                }
            }
        }
    }
}
