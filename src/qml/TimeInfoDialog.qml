import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
  id: timeInfoDialog
  property date creationTime
  property date modificationTime

  ColumnLayout {
    anchors.fill: parent
    anchors.margins: -5
    spacing: 15
    InfoLine {
      Layout.topMargin: 20
      infoTitle: qsTr("Creation:")
      infoText: Qt.formatDateTime(creationTime)
    }
    InfoLine {
      infoTitle: qsTr("Last edit:")
      infoText: Qt.formatDateTime(modificationTime)
    }
    PrettyButton {
      text: qsTr("Ok")
      Layout.alignment: Qt.AlignHCenter
      onClicked: {
        timeInfoDialog.accept()
      }
    }

  }

}
