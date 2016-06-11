import QtQuick 2.0
import QtQuick.Controls 1.3
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3

Dialog{
   id: opendlg
   width: 400
   height: 200

   property alias landmarkdir: landmarktext.text
   property alias actiondir: actiontext.text

   function show(){
      opendlg.visible = true
   }

   function clearUrl(url){
       var path = url.toString();
       path = path.replace(/^(file:\/{2})/,"");
       return decodeURIComponent(path);
   }

   FileDialog{
       id: landdlg
       selectFolder: true
       onAccepted: landmarktext.text = clearUrl(landdlg.fileUrl)
   }
   FileDialog{
       id: actiondlg
       selectFolder: true
       onAccepted: actiontext.text = clearUrl(actiondlg.fileUrl)
   }

   ColumnLayout{
       anchors.fill: parent
       RowLayout{
           Text{
               text: "Landmark Dir"
           }

           TextField{
               id: landmarktext
               Layout.fillWidth: true
           }
           Button{
               text: "*"
               onClicked: landdlg.open()
           }
       }
       RowLayout{
           Text{
               text: "Action Unit Dir"
           }

           TextField{
               id: actiontext
               Layout.fillWidth: true
           }
           Button{
               text: "*"
               onClicked: actiondlg.open()
           }
       }
   }
}
