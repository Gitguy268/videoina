import QtQuick

// A rounded liquid-glass panel. Put content inside it; `source` is the item
// whose pixels show through (the video scene).
Item {
    id: root
    required property Item source
    property real radius: 22

    ShaderEffectSource {
        id: behind
        visible: false
        sourceItem: root.source
        live: true
        // Re-evaluated when the panel or its parent moves or resizes.
        sourceRect: {
            root.x; root.y; root.width; root.height; root.parent?.x; root.parent?.y; root.visible
            const p = root.mapToItem(root.source, 0, 0)
            return Qt.rect(p.x, p.y, root.width, root.height)
        }
        // Half resolution: cheaper, and bilinear sampling adds to the blur.
        textureSize: Qt.size(Math.max(1, root.width / 2), Math.max(1, root.height / 2))
    }

    ShaderEffect {
        anchors.fill: parent
        property var src: behind
        property size size: Qt.size(width, height)
        property real radius: root.radius
        fragmentShader: "qrc:/shaders/glass.frag.qsb"
    }
}
