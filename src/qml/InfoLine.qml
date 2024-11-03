import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RowLayout {
  property string infoTitle
  property string infoText
  Layout.fillWidth: true
  PrettyLabel {title: infoTitle}
  Item {Layout.fillWidth: true}
  Label {text: infoText}
}
