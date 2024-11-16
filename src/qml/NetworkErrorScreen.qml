import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
  id: screen
  property string errorString

  ColumnLayout {
    anchors.centerIn: parent
    Image {
      Layout.preferredWidth: 200
      Layout.preferredHeight: 200
      Layout.alignment: Qt.AlignHCenter
      source: "icons/internet-error.png"
    }
    PrettyLabel {
      id: message
      Layout.alignment: Qt.AlignHCenter
      Layout.topMargin: 20
      title: qsTr("Network connection problem:\n") + errorString
    }
  }

}
