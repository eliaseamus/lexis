import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Item {
  id: image
  property url source
  property int fillMode
  property bool isSaturated

  Image {
    id: sourceItem
    anchors.fill: parent
    fillMode: image.fillMode
    source: image.source
  }

  MultiEffect {
    source: sourceItem
    anchors.fill: sourceItem
    saturation: isSaturated ? -1.0 : 0.0
  }
}
