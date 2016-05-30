import QtQuick 2.0

Rectangle {
    id: lview
    property var model: []
    property real scaleFactor: 1.0
    property int rectwidth: 2
    property bool showNumber: false
    width: childrenRect.width + 10
    height: childrenRect.height + 10
    Repeater{
        id: rep
        model: lview.model
        Rectangle{
            color: "black"
            x: modelData.x *  lview.scaleFactor
            y: modelData.y * lview.scaleFactor
            width: rectwidth
            height: rectwidth
            Text{
                visible: lview.showNumber
                x: 2
                y: 2
                text: index
            }
        }
    }
}
