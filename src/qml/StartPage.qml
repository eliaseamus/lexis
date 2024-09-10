import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
  id: startPage
  leftPadding: 0
  rightPadding: 0

  ColumnLayout {
    id: layout
    anchors.fill: parent
    anchors.topMargin: 50
    anchors.leftMargin: 50
    anchors.rightMargin: 50

    TextComplete {
      id: searchLine
      Layout.fillWidth: true
      placeholder: qsTr("Search")
    }

    RoundButton {
      id: addLibraryItem
      text: "+"
      Layout.alignment: Qt.AlignRight | Qt.AlignBottom
      Material.background: Material.accentColor
      onClicked: stackView.push(newItem)
    }
  }

  NewLibraryItem {
    id: newItem
    visible: false
  }

}
