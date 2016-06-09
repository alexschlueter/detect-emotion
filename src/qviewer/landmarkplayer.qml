import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.3

Item{
    id: player
    property var landmarkModel: []
    property var actionUnit: null
    property int frame: 0
    property bool playing: false
    property int fps: 25
    property alias scaleFactor: landmarks.scaleFactor
    property alias showNumber: landmarks.showNumber
    property alias normalize: landmarks.normalize
    property alias rectWidth: landmarks.rectWidth

    onActionUnitChanged: refreshActionUnit()

    Timer{
        onTriggered: function(){
            if (frame >= landmarkModel.length){
                frame = landmarkModel.length-1;
                playing = false;
            }else{
                frame = frame+1;
            }
        }
        interval: 1000 / fps
        repeat: true
        running: player.playing
    }

    RowLayout{
        id: controls
        height: childrenRect.height
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        Button{
            id: playButtom
            text: playing? "| |": ">"
            height: 30
            width: 20
            onClicked: player.playing = !player.playing
        }
        Slider{
            id: slider
            height: 30
            stepSize: 1
            Layout.fillWidth: true
            maximumValue: landmarkModel.length
            value: player.frame
            onValueChanged: player.frame = value
        }
        Text{
            text: player.frame+"/"+ player.landmarkModel.length
        }
    }

    ScrollView{
        id: scrollview
        anchors.top: controls.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        contentItem:Item{
            width: Math.max(actionunits.x+actionunits.width,landmarks.width)
            height: Math.max(actionunits.y+actionunits.height,landmarks.height)
            LandmarkView{
                id: landmarks
                scaleFactor: 0.3
                landmarks: player.landmarkModel[frame]
            }
            Rectangle{
                id: actionunits
                visible: actionUnitView && actionUnitView.model && actionUnitView.model.length > 0
                color: "black"
                opacity: 0.6
                x:0
                y:0
                height: cl.height+20
                width: cl.width+20
                ColumnLayout{
                    id: cl
                    x: 10
                    y: 10
                    Rectangle{
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 10
                        color: "white"
                    }

                    Repeater{
                        id: actionUnitView
                        model: []
                        delegate:
                            RowLayout{
                            Text{
                                text: modelData[0]
                                //color:  modelData[1] > 0 ? Qt.rgba(modelData[1]*0.2,0.8,1-modelData[1]*0.2,1) : "white"
                                color:  modelData[1] > 0 ? Qt.tint(Qt.rgba(0,0.8,1,1),Qt.rgba(0.8,1,1,modelData[1]*0.2)) : "white"
                                font.pixelSize: 14
                                id: actionname
                            }
                            spacing: 1
                            Item{ width: 9}
                            Repeater{
                                model: modelData[1]
                                delegate:
                                    Rectangle{
                                        color:  Qt.tint(Qt.rgba(0,0.8,1,1),Qt.rgba(0.8,1,1,modelData*0.2))
                                        height:  actionname.height
                                        width: 4
                                }
                            }
                        }
                    }
                }
                Drag.active: mouseArea.drag.active
                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    drag.target: parent

                }
            }
        }
    }


    function refreshActionUnit(){
        if (player.actionUnit && player.actionUnit.length > frame){
            var m = player.actionUnit.actionIntensity(player.frame+1)
            var res = []
            for (var prop in m){
                //res.push(prop+ " - "+m[prop])
                res.push([prop,m[prop]])
            }
            actionUnitView.model = res
        }
    }

    onFrameChanged:refreshActionUnit()
}
