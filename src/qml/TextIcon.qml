import QtQuick
import QtQuick.Layouts

Item {
  id: textIcon
  property int iconRadius
  property color iconColor
  property string iconTitle

  Rectangle {
    anchors.fill: parent
    radius: iconRadius
    clip: true
    color: iconColor.lighter(1.1)
    border.color: iconColor.darker(1.1)
    border.width: 2
    Text {
      anchors.centerIn: parent
      text: iconTitle.length > 0 ? iconTitle.replace("\"", "")[0] : ""
      color: iconColor.darker(1.1)
      font.capitalization: Font.AllUppercase
      font.pointSize: 40
    }
  }
}
