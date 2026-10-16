import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import Videoina

ApplicationWindow {
    id: win
    width: 1280
    height: 720
    minimumWidth: 480
    minimumHeight: 300
    visible: true
    color: "black"
    title: s["media-title"] ? s["media-title"] + " — Videoina" : "Videoina"
    palette {
        windowText: "white"; text: "white"; buttonText: "white"; highlightedText: "white"
        light: "#2affffff"; midlight: "#40ffffff"; mid: "#60ffffff"; dark: "#80ffffff"
    }

    readonly property var s: mpv.state
    readonly property bool idle: s["idle-active"] !== false
    property bool showControls: true

    // mpv owns the fullscreen flag (IINA's bindings toggle it); mirror it onto the window.
    readonly property bool mpvFullscreen: !!s["fullscreen"]
    onMpvFullscreenChanged: visibility = mpvFullscreen ? Window.FullScreen : Window.Windowed

    function open(urls) {
        urls.forEach((u, i) => mpv.command(["loadfile", String(u), i ? "append" : "replace"]))
    }

    // Everything the glass panels refract: the backdrop plus the video.
    Item {
        id: scene
        anchors.fill: parent
        // Must stay visible even when idle: mpv needs its render context before a file opens.
        MpvItem {
            id: mpv
            anchors.fill: parent
        }
        Rectangle {
            anchors.fill: parent
            visible: win.idle
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop { position: 0; color: "#1d2b64" }
                GradientStop { position: 0.55; color: "#5b3a8c" }
                GradientStop { position: 1; color: "#f8a978" }
            }
        }
    }

    MouseArea {
        id: pointer
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        cursorShape: win.showControls ? Qt.ArrowCursor : Qt.BlankCursor
        onPositionChanged: { win.showControls = true; hideTimer.restart() }
        onDoubleClicked: mpv.command(["cycle", "fullscreen"])
        onWheel: wheel => mpv.command(["add", "volume", String(wheel.angleDelta.y > 0 ? 2 : -2)])
    }

    Timer {
        id: hideTimer
        interval: 2500
        onTriggered: win.showControls = win.idle || s["pause"] === true || controls.hovered
    }

    Glass {
        source: scene
        anchors.centerIn: parent
        width: 420
        height: 150
        visible: win.idle
        Column {
            anchors.centerIn: parent
            spacing: 8
            Label { anchors.horizontalCenter: parent.horizontalCenter; text: "Videoina"; color: "white"; font.pixelSize: 30; font.bold: true }
            Label { anchors.horizontalCenter: parent.horizontalCenter; text: qsTr("Drop a file here or press Ctrl+O"); color: "#e0ffffff" }
        }
    }

    ControlBar {
        id: controls
        source: scene
        mpv: mpv
        property bool hovered: hover.hovered
        HoverHandler { id: hover }
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 24
        width: Math.min(parent.width - 48, 680)
        opacity: win.showControls ? 1 : 0
        visible: opacity > 0
        Behavior on opacity { NumberAnimation { duration: 200 } }
        onTogglePlaylist: playlist.shown = !playlist.shown
    }

    Playlist {
        id: playlist
        property bool shown: false
        source: scene
        mpv: mpv
        width: 300
        anchors.top: parent.top
        anchors.bottom: controls.top
        anchors.margins: 16
        x: shown ? parent.width - width - 16 : parent.width + 8
        Behavior on x { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }
    }

    // Keys go to mpv as `keypress`, so IINA's input.conf drives them.
    Item {
        focus: true
        Keys.onPressed: event => { event.accepted = mpv.keypress(event.key, event.modifiers, event.text) }
    }

    Shortcut { sequences: [StandardKey.Open]; onActivated: dialog.open() }

    FileDialog {
        id: dialog
        fileMode: FileDialog.OpenFiles
        onAccepted: win.open(selectedFiles)
    }

    DropArea {
        anchors.fill: parent
        onDropped: drop => win.open(drop.urls)
    }
}
