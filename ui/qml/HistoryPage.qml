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
import org.nemomobile.contacts 1.0
import org.nemomobile.commhistory 1.0
import com.nokia.meego 1.2
import com.nokia.extras 1.1

Page {
    id:root

    orientationLock:PageOrientation.LockPortrait

    SelectionDialog {
        id:dHistorySelect
        model:ListModel {
            ListElement {tag:'recent'; name:'Recent calls'}
            ListElement {tag:'missed'; name:'Missed calls'}
            ListElement {tag:'incoming'; name:'Received calls'}
            ListElement {tag:'outgoing'; name:'Dialled calls'}
        }
        onSelectedIndexChanged: {
            var filter = model.get(selectedIndex);
            var filterRole = null;
            var filterString = null;

            switch(filter.tag) {
            case 'recent':
                filterRole = -1;
                filterString = "";
                break;

            case 'missed':
                filterRole = CommCallModel.IsMissedCallRole
                filterString = 'true'
                break;

            case 'incoming':
                filterRole = CommCallModel.DirectionRole
                filterString = '' + CommCallModel.Inbound
                break;

            case 'outgoing':
                filterRole = CommCallModel.DirectionRole
                filterString = '' + CommCallModel.Outbound
                break;
            }

            historyList.model.setFilterRole(filterRole)
            historyList.model.setFilterFixedString(filterString)
            bHistorySelect.text = filter.name
        }
    }

    PageHeader {
        id: bHistorySelect
        text: qsTr('Recent Calls')
        color: "#228B22"

        MouseArea {
            anchors.fill: parent
            onClicked: dHistorySelect.open();
        }
    }

    ListView {
        id:historyList
        anchors {top:bHistorySelect.bottom;bottom:parent.bottom;margins:5;horizontalCenter:parent.horizontalCenter}
        width:parent.width - 10
        spacing:4
        clip:true
        model: CommCallModel {}

        delegate: Item {
            width:parent.width;height:72
            property Person contact

            Component.onCompleted: contact = people.personByPhoneNumber(model.remoteUid);

            Row {
                anchors {left:parent.left; leftMargin:10; verticalCenter:parent.verticalCenter}
                spacing:10

                Image {
                    anchors.verticalCenter:parent.verticalCenter
                    smooth:true
                    source: 'images/icon-m-telephony-call-' + (model.isMissedCall ? 'missed' : (model.direction == CommCallModel.Inbound ? 'received' : 'initiated')) + '.svg'
                }

                Column {
                    Text {
                        color:model.isMissedCall ? 'red' : 'white'
                        font.pixelSize:28
                        text:contact ? contact.displayLabel : model.remoteUid
                    }
                    Text {
                        color:'grey'
                        font.pixelSize:18
                        text:Qt.formatDateTime(model.startTime, Qt.DefaultLocaleShortDate)
                    }
                }
            }

            Row {
                anchors {right:parent.right; rightMargin:10; verticalCenter:parent.verticalCenter}
                spacing:10
                ToolButton {
                    width:72;height:60
                    iconSource:'images/icon-m-telephony-accept.svg'
                    onClicked:main.dial(model.remoteUid);
                }

                ToolButton {
                    width:72;height:60
                    iconSource:'images/icon-addressbook.svg'
                    onClicked:console.log('SHOW CONTACT CARD')
                }
            }
        }
    }
}

