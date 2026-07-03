import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Label {
  property string title
  property int format: Text.PlainText
  property bool dimmed: false

  text: qsTr(title)
  textFormat: format
  horizontalAlignment: Qt.AlignHCenter
  color: settings.fgColor
  Layout.margins: 5

  background: Rectangle {
    anchors.fill: parent
    anchors.margins: -5
    radius: 10
    color: dimmed ? Qt.rgba(settings.accentColor.r, settings.accentColor.g,
                            settings.accentColor.b, 0.75) : settings.accentColor
  }
}
