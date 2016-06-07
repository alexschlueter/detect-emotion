import QtQuick 2.0

Rectangle {
    id: lview
    //property var model: []
    property QtObject landmarks: none;
    property real scaleFactor: 1.0
    property int rectWidth: 2
    property bool showNumber: false
    property bool normalize: false
    width: childrenRect.width + 10
    height: childrenRect.height + 10
    Repeater{
        id: rep
        model: lview.landmarks ? (normalize ? lview.landmarks.normalized() :lview.landmarks.points): []
        Rectangle{
            color: "black"
            x: modelData.x *  lview.scaleFactor
            y: modelData.y * lview.scaleFactor
            width: rectWidth
            height: rectWidth
            radius: 4
            Text{
                visible: lview.showNumber
                x: 2
                y: 2
                text: index
            }
        }
    }
}
