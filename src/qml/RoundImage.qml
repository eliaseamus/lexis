import QtQuick
import QtQuick.Effects

Item {
  id: roundImage
  property string source
  property int imageRadius
  property int imageFillMode: Image.PreserveAspectCrop

  Image {
    id: sourceItem
    source: roundImage.source
    anchors.fill: parent
    anchors.centerIn: parent
    fillMode: imageFillMode
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
      radius: imageRadius ? imageRadius : width / 2
    }
  }

}
