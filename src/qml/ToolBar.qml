import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RowLayout {
  id: toolBar
  property bool isSearchActive: searchLine.display

  signal goBack
  signal searchRequest(query: string)
  signal addNewItem

  RoundButton {
    id: back
    icon.source: "icons/back.png"
    icon.color: settings.fgColor
    enabled: false
    Material.background: settings.accentColor
    onClicked: goBack()

    ToolTip {
      visible: back.hovered
      text: qsTr("Back")
    }
  }

  Item {Layout.fillWidth: true}

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
      onTextChanged: {
        searchRequest(searchLine.text)
      }
    }
  }

  Item {Layout.fillWidth: true}

  RoundButton {
    id: addLibraryItem
    enabled: settings.currentLanguage.length > 0
    icon.source: "icons/plus.png"
    icon.color: settings.fgColor
    Material.background: settings.accentColor
    onClicked: addNewItem()

    ToolTip {
      visible: addLibraryItem.hovered
      text: qsTr("New item")
    }
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
    onActivated: addNewItem()
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

  function refresh() {
    const isSearchEnabled = library.sections.length > 0;
    const isNewItemEnabled = settings.currentLanguage.length > 0;
    back.enabled = !libraryView.isStartPage;
    searchCmd.enabled = isSearchEnabled;
    search.enabled = isSearchEnabled;
    newItemCmd.enabled = isNewItemEnabled;
    addLibraryItem.enabled = isNewItemEnabled;
  }
}
