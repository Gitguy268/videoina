import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// IINA-style floating on-screen controller.
Glass {
    id: bar
    required property var mpv
    signal togglePlaylist()
    readonly property var s: mpv.state
    readonly property real duration: s["duration"] ?? 0

    radius: 24
    implicitHeight: 92

    function clock(t) {
        t = Math.max(0, Math.floor(t || 0))
        const h = Math.floor(t / 3600), m = Math.floor(t / 60) % 60, sec = t % 60
        const pad = n => String(n).padStart(2, "0")
        return (h ? h + ":" + pad(m) : m) + ":" + pad(sec)
    }

    component GlassButton: ToolButton {
        id: btn
        property string glyph: ""
        property real size: 22
        focusPolicy: Qt.NoFocus // keep keyboard focus on the window, where keys reach mpv
        implicitWidth: size + 16
        implicitHeight: size + 16
        contentItem: Item {
            Icon { anchors.centerIn: parent; d: btn.glyph; scale: btn.size / 24 }
        }
        background: Rectangle {
            radius: height / 2
            color: btn.down ? "#40ffffff" : btn.hovered ? "#22ffffff" : "transparent"
        }
    }

    component ThinSlider: Slider {
        id: sl
        focusPolicy: Qt.NoFocus
        background: Rectangle {
            x: sl.leftPadding
            y: sl.topPadding + sl.availableHeight / 2 - height / 2
            width: sl.availableWidth
            height: 4
            radius: 2
            color: "#40ffffff"
            Rectangle { width: sl.visualPosition * parent.width; height: parent.height; radius: 2; color: "white" }
        }
        handle: Rectangle {
            x: sl.leftPadding + sl.visualPosition * (sl.availableWidth - width)
            y: sl.topPadding + sl.availableHeight / 2 - height / 2
            width: 14; height: 14; radius: 7
            color: "white"
            opacity: sl.hovered || sl.pressed ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: 120 } }
        }
    }

    // Menu listing mpv tracks of one type ("sub" / "audio"), selecting via `prop` ("sid" / "aid").
    component TrackMenu: Menu {
        id: menu
        property string type
        property string prop
        background: Glass { source: bar.source; radius: 14; implicitWidth: 260 }
        MenuItem {
            text: qsTr("Off")
            checkable: true
            checked: !(bar.s["track-list"] ?? []).some(t => t.type === menu.type && t.selected)
            onTriggered: bar.mpv.setProperty(menu.prop, "no")
        }
        Instantiator {
            model: (bar.s["track-list"] ?? []).filter(t => t.type === menu.type)
            delegate: MenuItem {
                required property var modelData
                text: [modelData.title, modelData.lang, modelData.codec].filter(x => x).join(" · ") || ("#" + modelData.id)
                checkable: true
                checked: modelData.selected
                onTriggered: bar.mpv.setProperty(menu.prop, modelData.id)
            }
            onObjectAdded: (i, o) => menu.insertItem(i + 1, o)
            onObjectRemoved: (i, o) => menu.removeItem(o)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        anchors.leftMargin: 18
        anchors.rightMargin: 18
        spacing: 2

        RowLayout {
            Layout.fillWidth: true
            GlassButton {
                glyph: bar.s["mute"] ? "M3 9h4l5-4v14l-5-4H3z" : "M3 9h4l5-4v14l-5-4H3zM15 9h2v6h-2zM19 7h2v10h-2z"
                size: 20
                onClicked: bar.mpv.command(["cycle", "mute"])
            }
            ThinSlider {
                Layout.preferredWidth: 110
                from: 0; to: 100
                value: bar.s["volume"] ?? 100
                onMoved: bar.mpv.setProperty("volume", value)
            }
            Item { Layout.fillWidth: true }
            GlassButton { glyph: "M6 5h2v14H6zM9.5 12L19 5v14z"; onClicked: bar.mpv.command(["playlist-prev"]) }
            GlassButton {
                glyph: bar.s["pause"] === false ? "M6 5h4v14H6zM14 5h4v14h-4z" : "M8 5v14l11-7z"
                size: 30
                onClicked: bar.mpv.command(["cycle", "pause"])
            }
            GlassButton { glyph: "M16 5h2v14h-2zM5 5l9.5 7L5 19z"; onClicked: bar.mpv.command(["playlist-next"]) }
            Item { Layout.fillWidth: true }
            GlassButton {
                glyph: "M3 5h18v14H3zM5 7v10h14V7zM7 12h4v2H7zM13 12h4v2h-4z"
                size: 20
                onClicked: subMenu.popup()
                TrackMenu { id: subMenu; type: "sub"; prop: "sid" }
            }
            GlassButton {
                glyph: "M10 4h9v3h-7v9.5a3 3 0 1 1-2-2.8z"
                size: 20
                onClicked: audioMenu.popup()
                TrackMenu { id: audioMenu; type: "audio"; prop: "aid" }
            }
            GlassButton { glyph: "M3 6h12v2H3zM3 11h12v2H3zM3 16h8v2H3zM17 13l5 3-5 3z"; size: 20; onClicked: bar.togglePlaylist() }
            GlassButton {
                glyph: "M3 3h6v2H5v4H3zM15 3h6v6h-2V5h-4zM3 15h2v4h4v2H3zM19 15h2v6h-6v-2h4z"
                size: 20
                onClicked: bar.mpv.command(["cycle", "fullscreen"])
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            Label { text: bar.clock(bar.s["time-pos"]); color: "white"; font.features: { "tnum": 1 } }
            ThinSlider {
                id: seek
                Layout.fillWidth: true
                from: 0; to: Math.max(bar.duration, 0.001)
                enabled: bar.duration > 0
                // Keep the handle where the user drags it until mpv catches up.
                Binding on value { when: !seek.pressed; value: bar.s["time-pos"] ?? 0 }
                onMoved: bar.mpv.command(["seek", String(value), "absolute"])
            }
            Label { text: bar.clock(bar.duration); color: "white"; font.features: { "tnum": 1 } }
        }
    }
}
