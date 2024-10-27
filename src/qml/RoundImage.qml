import QtQuick
import QtQuick.Effects

Item {
  id: roundImage
  property string source

  Image {
    id: sourceItem
    source: roundImage.source
    anchors.fill: parent
    anchors.centerIn: parent
    visible: false
  }

  MultiEffect {
    source: sourceItem
    anchors.fill: sourceItem
    anchors.centerIn: sourceItem
    maskEnabled: true
    maskSource: mask
  }

  Item {
    id: mask
    width: sourceItem.width
    height: sourceItem.height
    layer.enabled: true
    visible: false
    Rectangle {
      width: sourceItem.width
      height: sourceItem.height
      radius: width / 2
    }
  }

}
