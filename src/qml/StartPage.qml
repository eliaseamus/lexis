import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

Pane {
  id: startPage
  leftPadding: 0
  rightPadding: 0

  TextField {
    id: searchLine
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.topMargin: 50
    anchors.leftMargin: 50
    anchors.rightMargin: 50
    placeholderText: qsTr("Search")
  }

  RoundButton {
    id: addLibraryItem
    text: "+"
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 10
    anchors.right: parent.right
    anchors.rightMargin: 50
    Material.background: Material.Red
    onClicked: stackView.push(newItem)
  }

  NewLibraryItem {
    id: newItem
    visible: false
  }

}
