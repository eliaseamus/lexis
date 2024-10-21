import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Label {
  property string title
  text: qsTr(title)
  horizontalAlignment: Qt.AlignHCenter
  color: settings.fgColor
  Layout.leftMargin: 5
  background: Rectangle {
    anchors.fill: parent
    anchors.margins: -5
    color: settings.accentColor
    radius: 10
  }
}
