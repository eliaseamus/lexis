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

    TextField {
      id: searchLine
      Layout.fillWidth: true
      placeholderText: qsTr("Search")
    }

    ScrollView {
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.leftMargin: 20

      ColumnLayout {
        anchors.fill: parent
        Repeater {
          id: librarySections
          model: library.sections
          Layout.fillWidth: true
          Layout.fillHeight: true

          SectionView {
            required property LibrarySection modelData
            title: modelData.name
            model: modelData.model

            Connections {
              target: searchLine
              function onTextChanged() {
                model.setFilterFixedString(searchLine.text)
                visible = model.rowCount() > 0
              }
            }
          }
        }
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
