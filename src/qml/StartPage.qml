import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QLexis

Pane {
  id: startPage
  leftPadding: 0
  rightPadding: 0
  property var sectionNames: [
    qsTr("Subject group"),
    qsTr("Book"),
    qsTr("Article"),
    qsTr("Movie"),
    qsTr("Series"),
    qsTr("Album"),
    qsTr("Song")
  ]

  ColumnLayout {
    id: layout
    anchors.fill: parent
    anchors.topMargin: 20
    anchors.leftMargin: 20
    anchors.rightMargin: 20

    Button {
      id: pickLanguage
      Layout.alignment: Qt.AlignCenter
      visible: settings.currentLanguage.length === 0
      text: qsTr("Select a language to learn")
      Material.background: settings.accentColor
      onClicked: {
        newLanguageDialog.init()
        newLanguageDialog.open()
      }

      contentItem: Label {
        text: pickLanguage.text
        color: settings.fgColor
        verticalAlignment: Text.AlignVCenter
      }
    }

    ScrollView {
      visible: settings.currentLanguage.length > 0
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
            title: sectionNames[modelData.type]
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

    RowLayout {
      id: toolBar
      RoundButton {
        id: search
        icon.source: "icons/search.png"
        icon.color: settings.fgColor
        visible: library.sections.length > 0
        Layout.alignment: Qt.AlignLeft | Qt.AlignBottom | Qt.AlignVCenter
        Material.background: settings.accentColor
        onClicked: toolBar.toggleSearchLine()

        ToolTip {
          visible: search.hovered
          text: qsTr("Search")
        }
      }

      TextField {
        id: searchLine
        property int length: 0
        property bool display: false
        visible: false
        Layout.preferredWidth: length
        Layout.preferredHeight: 40
        placeholderText: qsTr("Search")

        PropertyAnimation {
          id: searchShow
          target: searchLine
          property: "length"
          to: 200
          duration: 300
          onRunningChanged: {
            if (!running) {
              searchLine.forceActiveFocus()
            }
          }
        }
        PropertyAnimation {
          id: searchHide
          target: searchLine
          property: "length"
          to: 0
          duration: 300
          onRunningChanged: {
            if (!running) {
              searchLine.visible = false
              searchLine.text = ""
              search.forceActiveFocus()
            }
          }
        }
      }

      Item {
        Layout.fillWidth: true
      }

      RoundButton {
        id: addLibraryItem
        visible: settings.currentLanguage.length > 0
        icon.source: "icons/plus.png"
        icon.color: settings.fgColor
        Layout.alignment: Qt.AlignRight | Qt.AlignBottom | Qt.AlignVCenter
        Material.background: settings.accentColor
        onClicked: stackView.push(newItem)

        ToolTip {
          visible: addLibraryItem.hovered
          text: qsTr("Add new item")
        }
      }
      function toggleSearchLine() {
        if (searchLine.display) {
          searchLine.display = false
          searchHide.running = true
        } else {
          searchLine.display = true
          searchLine.visible = true
          searchShow.running = true
        }
      }
    }
  }

  NewLanguageDialog {
    id: newLanguageDialog
    x: (main.width - width) / 2
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
  }

  NewLibraryItem {
    id: newItem
    visible: false
  }

  SectionTypeManager {
    id: sectionTypeManager
  }

  Shortcut {
    enabled: library.sections.length > 0
    sequence: StandardKey.Find
    onActivated: toolBar.toggleSearchLine()
  }

  Shortcut {
    enabled: settings.currentLanguage.length > 0
    sequence: StandardKey.New
    onActivated: stackView.push(newItem)
  }

  function refresh() {
    librarySections.model = library.sections
  }

  function hideSearchLine() {
    searchLine.display = false
    searchHide.running = true
  }

}
