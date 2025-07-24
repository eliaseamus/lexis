import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "utils.js" as Utils

Rectangle {
  id: sectionItem
  property bool isSelected
  property color backgroundColor
  property color borderColor
  property string itemTitle
  property string imageSource
  width: 200
  height: 200
  color: backgroundColor
  border.color: borderColor
  border.width: 2
  radius: 10
  RoundButton {
    flat: true
    visible: isSelected
    icon.source: "qrc:/qt/qml/QLexis/icons/check.png"
    icon.color: settings.fgColor
    Material.background: settings.accentColor
  }
  ColumnLayout {
    anchors.fill: parent
    anchors.margins: 10
    Icon {
      Layout.preferredHeight: 150
      Layout.preferredWidth: 150
      Layout.alignment: Qt.AlignCenter
      image: imageSource
      iconRadius: 150
      iconColor: backgroundColor
      iconTitle: itemTitle
    }
    Text {
      Layout.alignment: Qt.AlignHCenter
      Layout.preferredWidth: 180
      horizontalAlignment: Qt.AlignHCenter
      text: itemTitle
      elide: Text.ElideRight
      color: Utils.getFgColor(backgroundColor)
    }
  }
}
