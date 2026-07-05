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
  property string wordFrequencyTier: ""
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
  Rectangle {
    id: frequencyBadge
    visible: wordFrequencyTier.length > 0
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.margins: 8
    height: 22
    width: badgeLabel.implicitWidth + 12
    radius: 11
    color: Utils.frequencyTierColor(wordFrequencyTier)
    z: 1

    Text {
      id: badgeLabel
      anchors.centerIn: parent
      text: Utils.frequencyTierLabel(wordFrequencyTier)
      color: "white"
      font.pixelSize: 11
      font.bold: true
    }

    ToolTip {
      visible: frequencyBadgeMouseArea.containsMouse
      text: qsTr("Word frequency: %1").arg(Utils.frequencyTierLabel(wordFrequencyTier))
    }

    MouseArea {
      id: frequencyBadgeMouseArea
      anchors.fill: parent
      hoverEnabled: true
      acceptedButtons: Qt.NoButton
    }
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
