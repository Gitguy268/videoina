import QtQuick
import QtQuick.Controls.Basic

// Glass sidebar listing mpv's playlist; click an entry to play it.
Glass {
    id: side
    required property var mpv
    radius: 20

    Label {
        id: heading
        text: qsTr("Playlist")
        color: "white"
        font.pixelSize: 17
        font.bold: true
        x: 18; y: 16
    }

    ListView {
        anchors.fill: parent
        anchors.topMargin: heading.y + heading.height + 10
        anchors.margins: 8
        clip: true
        model: side.mpv.state["playlist"] ?? []
        delegate: ItemDelegate {
            required property var modelData
            required property int index
            width: ListView.view.width
            highlighted: !!modelData.current
            contentItem: Label {
                text: modelData.title || String(modelData.filename).split(/[\\/]/).pop()
                color: "white"
                font.bold: !!modelData.current
                elide: Text.ElideMiddle
            }
            background: Rectangle {
                radius: 10
                color: parent.highlighted ? "#33ffffff" : parent.hovered ? "#1affffff" : "transparent"
            }
            onClicked: side.mpv.setProperty("playlist-pos", index)
        }
    }
}
