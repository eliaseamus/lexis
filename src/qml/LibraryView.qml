import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QLexis

Pane {
  id: libraryView
  leftPadding: 0
  rightPadding: 0
  property var sectionNames: [
    qsTr("Word"),
    qsTr("Subject group"),
    qsTr("Book"),
    qsTr("Article"),
    qsTr("Movie"),
    qsTr("Series"),
    qsTr("Album"),
    qsTr("Song")
  ]
  property var tables: []
  property bool isStartPage: true

  ColumnLayout {
    id: layout
    anchors.fill: parent
    anchors.topMargin: 20
    anchors.leftMargin: 20
    anchors.rightMargin: 20

    Item {
      Layout.fillHeight: true
    }

    // No language selected
    Button {
      id: pickLanguage
      Layout.alignment: Qt.AlignCenter
      Layout.rightMargin: sideBar.width
      visible: settings.currentLanguage.length === 0
      text: qsTr("Select a language to learn")
      Material.background: settings.accentColor
      onClicked: {
        selectLanguageDialog.init()
        selectLanguageDialog.open()
      }

      contentItem: Label {
        text: pickLanguage.text
        color: settings.fgColor
        verticalAlignment: Text.AlignVCenter
      }
    }

    // Language is selected but its library has no entries
    PrettyLabel {
      id: prompt
      Layout.alignment: Qt.AlignCenter
      Layout.rightMargin: sideBar.width
      visible: settings.currentLanguage.length > 0 && library.sections.length === 0
      title: qsTr("Library is empty.\n" +
                   "Add the first item using \"+\" button\n" +
                   "in the right bottom corner.")
    }

    Item {
      Layout.fillHeight: true
    }

    // Library items
    ScrollView {
      id: itemsSection
      visible: library.sections.length > 0
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
            parentTable: tables.length == 0 ? settings.currentLanguage : tables[tables.length - 1]
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

    // Footer
    RowLayout {
      id: toolBar
      RoundButton {
        id: back
        icon.source: "icons/back.png"
        icon.color: settings.fgColor
        enabled: false
        Material.background: settings.accentColor
        onClicked: pop()

        ToolTip {
          visible: back.hovered
          text: qsTr("Back")
        }
      }

      Item {
        Layout.fillWidth: true
      }

      RowLayout {
        Layout.rightMargin: sideBar.width
        RoundButton {
          id: search
          icon.source: "icons/search.png"
          icon.color: settings.fgColor
          enabled: library.sections.length > 0
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
      }

      Item {
        Layout.fillWidth: true
      }

      RoundButton {
        id: addLibraryItem
        enabled: settings.currentLanguage.length > 0
        icon.source: "icons/plus.png"
        icon.color: settings.fgColor
        Material.background: settings.accentColor
        onClicked: {
          libraryItem.clear()
          stackView.push(libraryItem)
        }

        ToolTip {
          visible: addLibraryItem.hovered
          text: qsTr("New item")
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

  SelectLanguageDialog {
    id: selectLanguageDialog
    x: (main.width - width) / 2
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
  }

  LibraryItemConfiguration {
    id: libraryItem
    visible: false
    displayedTypes: sectionNames
    types: sectionTypeManager.librarySectionNames()
    typesNum: 8
  }

  DeleteDialog {
    id: deleteItemDialog
    x: (main.width - width) / 2
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
  }

  SectionTypeManager {
    id: sectionTypeManager
  }

  Shortcut {
    id: searchCmd
    enabled: library.sections.length > 0
    sequence: StandardKey.Find
    onActivated: toolBar.toggleSearchLine()
  }

  Shortcut {
    id: newItemCmd
    enabled: settings.currentLanguage.length > 0
    sequence: StandardKey.New
    onActivated: {
      libraryItem.clear()
      stackView.push(libraryItem)
    }
  }

  function changeLanguage(language) {
    tables = [];
    libraryItem.typesNum = 8;
    library.openTable(language);
    refresh();
  }

  function load(parentTable, parentID) {
    tables.push(`${parentTable}_${parentID}`);
    isStartPage = false;
    libraryItem.typesNum = 2;
    library.openChildTable(parentID);
    refresh();
  }

  function refresh() {
    librarySections.model = library.sections;
    const isSearchEnabled = library.sections.length > 0;
    const isNewItemEnabled = settings.currentLanguage.length > 0;
    back.enabled = tables.length > 0;
    itemsSection.visible = isSearchEnabled;
    searchCmd.enabled = isSearchEnabled;
    search.enabled = isSearchEnabled;
    newItemCmd.enabled = isNewItemEnabled;
    addLibraryItem.enabled = isNewItemEnabled;
    prompt.visible = isNewItemEnabled && !isSearchEnabled;
    libraryItem.clear();
  }

  function pop() {
    if (search.visible && searchLine.visible) {
      searchLine.display = false;
      searchHide.running = true;
    } else if (tables.length > 1) {
      tables.pop();
      var table = tables.pop();
      library.openTable(table);
      refresh();
    } else if (!isStartPage) {
      isStartPage = true;
      changeLanguage(settings.currentLanguage);
    }
  }

  function hideSearchLine() {
    searchLine.display = false
    searchHide.running = true
  }

}
