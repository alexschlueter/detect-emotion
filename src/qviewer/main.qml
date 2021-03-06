import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3
import PCAAnalyse 1.0

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("QViewer")
    id: win
    property var landmodel: []
    property var actionunits: []
    property PCAAnalyse pca: PCAAnalyse{}

    OpenDialog{
        id: dlg
        onAccepted: function(){
            landmodel = loader.loadLandmarksIterateDir(dlg.landmarkdir)
            win.pca.analyseData(landmodel)
            //win.pca.analyseData(landmodel[0])
            actionunits = loader.loadActionUnitIterateDir(dlg.actiondir)
        }
    }

    toolBar: ToolBar{
        RowLayout{
            anchors.fill: parent
            ToolButton{
                text: "Load"
                onClicked: dlg.show()
            }
            Text{text: "Scale"}
            Slider{
                maximumValue: 1
                id: scaleSlider
                value: 1
            }

            Text{text: "Person"}
            ComboBox{
                width: 100
                id: personBox
                model: landmodel.length
            }

            Text{text: "FPS"}
            SpinBox{
                stepSize: 1
                maximumValue: 200
                value: 25
                id: fpsbox
            }

            Text{text: "radius"}
            SpinBox{
                stepSize: 1
                maximumValue: 200
                value: 5
                id: radiusbox
            }

            Text{text: "Norm."}
            CheckBox{
                id: normbox
                checked: false
            }
            Text{text: "Show Nr."}
            CheckBox{
                id: nrbox
                checked: false
            }
            Text{text: "PCA Dimension"}
            SpinBox{
                stepSize: 1
                minimumValue: 1
                maximumValue: 66
                value: 66
                id: pcaDimBox
            }
            Button{
              text:"Print LM coords"
              id: saveBtn
              onClicked: player.printCoords()
            }
        }
    }

    property int personidx: personBox.currentIndex

    LandmarkPlayer{
        id: player
        anchors.fill: parent
        fps: fpsbox.value
        normalize: normbox.checked
        showNumber: nrbox.checked
        landmarkModel: personidx < win.landmodel.length ? win.landmodel[personidx]: []
        actionUnit: personidx < win.actionunits.length ? win.actionunits[personidx]: null
        pca: win.pca
        //actionUnit: none // win.actionunit
        scaleFactor: scaleSlider.value
        rectWidth: radiusbox.value
        pcaDimCount: pcaDimBox.value
    }
}
