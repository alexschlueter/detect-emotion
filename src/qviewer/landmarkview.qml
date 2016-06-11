import QtQuick 2.0

Rectangle {
    id: lview
    //property var model: []
    property QtObject landmarks: null;
    onLandmarksChanged: rebuildModel()
    property real scaleFactor: 1.0
    property int rectWidth: 2
    property bool showNumber: false
    property bool normalize: false
    onNormalizeChanged: rebuildModel()
    property QtObject pca: null;
    property int pcaDimCount: 66
    onPcaDimCountChanged: rebuildModel()
    onPcaChanged: rebuildModel()
    width: childrenRect.width + 10
    height: childrenRect.height + 10
    function rebuildModel(){
        if (!lview.landmarks) {
            rep.model =  []
            return
        }
        if (pca && !pca.isEmpty() && pcaDimCount < 66){
            var cloud = lview.landmarks
            rep.model =  pca.rebuildPoints(cloud,pcaDimCount);
        }
        else{
            var cloud  = normalize ? lview.landmarks.normalized() :lview.landmarks
            rep.model = cloud.points
        }
        if (normalize && rep.model.length > 0){
          var minX = rep.model[0].x
          var minY = rep.model[0].y
          for(var i=1; i< rep.model.length; i++){
             var p = rep.model[i];
              if (minX > p.x) { minX=p.x;}
              if (minY > p.y) { minY=p.y;}
          }
          rep.xOffset = -minX;
          rep.yOffset = -minY;
        }
    }

    Repeater{
        id: rep
        property real xOffset: 0
        property real yOffset: 0
        Rectangle{
            color: "black"
            x: (rep.xOffset + modelData.x) *  lview.scaleFactor
            y: (rep.yOffset + modelData.y) * lview.scaleFactor
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
