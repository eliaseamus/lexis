import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QLexis

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
      Layout.bottomMargin: 50
      placeholder: qsTr("Search")
    }

    Repeater {
      id: librarySections
      model: library.sections
      Layout.fillWidth: true
      Layout.fillHeight: true

      SectionView {
        required property LibrarySection modelData
        title: modelData.name
        model: modelData.model
      }
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

  Library {
    id: library
  }

  SectionTypeManager {
    id: sectionTypeManager
  }

}
