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
    RowLayout {
      Layout.fillWidth: true
      Layout.topMargin: 15
      Layout.bottomMargin: 10
      PrettyLabel {
        title: qsTr("Creation:")
      }
      Item {Layout.fillWidth: true}
      Label {
        text: Qt.formatDateTime(creationTime)
      }
    }
    RowLayout {
      Layout.fillWidth: true
      Layout.bottomMargin: 15
      PrettyLabel {
        title: qsTr("Last edit:")
      }
      Item {Layout.fillWidth: true}
      Label {
        text: Qt.formatDateTime(modificationTime)
      }
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
