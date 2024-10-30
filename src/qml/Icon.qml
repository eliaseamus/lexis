import QtQuick
import QtQuick.Layouts

Item {
  id: textIcon
  property string image
  property int iconRadius
  property color iconColor
  property string iconTitle

  RoundImage {
    id: img
    width: iconRadius
    height: iconRadius
    anchors.centerIn: parent
    imageRadius: iconRadius
    source: image
    visible: image.length > 0
  }

  Rectangle {
    anchors.centerIn: parent
    width: iconRadius
    height: iconRadius
    radius: iconRadius
    clip: true
    visible: !img.visible
    color: iconColor.lighter(1.1)
    border.color: iconColor.darker(1.1)
    border.width: 2
    Text {
      anchors.centerIn: parent
      text: iconTitle.length > 0 ? iconTitle[0] : ""
      color: iconColor.darker(1.1)
      font.capitalization: Font.AllUppercase
      font.pointSize: 40
    }
  }
}
