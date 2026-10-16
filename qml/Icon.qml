import QtQuick
import QtQuick.Shapes

// 24x24 filled vector icon from an SVG path (odd-even fill, so holes work).
Shape {
    id: icon
    property string d
    property color color: "white"
    implicitWidth: 24
    implicitHeight: 24
    preferredRendererType: Shape.CurveRenderer
    ShapePath {
        fillColor: icon.color
        strokeWidth: -1
        fillRule: ShapePath.OddEvenFill
        PathSvg { path: icon.d }
    }
}
