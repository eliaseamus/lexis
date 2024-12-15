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
  property var movableTypes: ["Word", "Subject group"]
  property var tables: []
  property bool isStartPage: true
  property var pages: []
  property var itemToDelete
  property var selectedItems: []
  property bool isSelectMode
  signal quitSelectMode

  ColumnLayout {
    id: layout
    anchors.fill: parent
    anchors.topMargin: 20
    anchors.leftMargin: 20
    anchors.rightMargin: 20

    PrettyLabel {
      id: pageTitle
      visible: false
      Layout.alignment: Qt.AlignCenter
      Layout.rightMargin: sideBar.width
    }

    Item {Layout.fillHeight: true}

    // No language selected
    PrettyButton {
      id: pickLanguage
      Layout.alignment: Qt.AlignCenter
      Layout.rightMargin: sideBar.width
      visible: settings.currentLanguage.length === 0
      text: qsTr("Select a language to learn")

      onClicked: {
        selectLanguageDialog.init()
        selectLanguageDialog.open()
      }
    }

    // Language is selected but its library has no entries
    PrettyLabel {
      id: prompt
      Layout.alignment: Qt.AlignCenter
      Layout.rightMargin: sideBar.width
      visible: settings.currentLanguage.length > 0 &&
               library.sections.length === 0
      title: qsTr("Library is empty.\n" +
                  "Add the first item using \"+\" button\n" +
                  "in the right bottom corner.")
    }

    Item {Layout.fillHeight: true}

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
            parentTable: tables.length == 0 ?
                         settings.currentLanguage :
                         tables[tables.length - 1]
            model: modelData.model

            Connections {
              target: toolBar
              function onSearchRequest(query) {
                model.setFilterFixedString(query)
                visible = model.rowCount() > 0
              }
            }
          }
        }
      }
    }

    ToolBar {id: toolBar}
  }

  Connections {
    target: toolBar

    function onAddNewItem() {
      itemConfiguration.clear();
      clearSelectedItems();
      stackView.push(itemConfiguration);
    }

    function onGoBack() {
      libraryView.pop()
    }
  }

  Connections {
    target: deleteItemDialog

    function onAccepted() {
      if (isSelectMode) {
        selectedItems.forEach((item) => {
          library.deleteItem(item["itemID"], item["type"]);
        });
        clearSelectedItems();
      } else {
        library.deleteItem(itemToDelete["itemID"], itemToDelete["type"]);
      }
      libraryView.refresh();
    }
  }

  Connections {
    target: main

    function onActiveChanged() {
      if (dragItems.visible && !active) {
        dragItems.visible = false;
      }
    }
  }

  LibraryItemConfiguration {
    id: itemConfiguration
    visible: false
    displayedTypes: sectionNames
    types: sectionTypeManager.librarySectionNames()
    typesNum: 8
  }

  ItemView {
    id: itemView
    visible: false
  }

  SelectLanguageDialog {
    id: selectLanguageDialog
    x: (main.width - width) / 2 - sideBar.width
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
  }

  DeleteDialog {
    id: deleteItemDialog
    x: (main.width - width) / 2 - sideBar.width
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
  }

  TimeInfoDialog {
    id: timeInfoDialog
    x: (main.width - width) / 2 - sideBar.width
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
  }

  MoveDialog {
    id: moveDialog
    x: (main.width - width) / 2 - sideBar.width
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
  }

  RowLayout {
    id: dragItems
    spacing: -195
    width: 150
    height: 150
    visible: false
    property var items: []
    signal drop
    opacity: 0.8
    Repeater {
      model: dragItems.items
      Layout.alignment: Qt.AlignCenter
      SectionItem {
        required property int index
        backgroundColor: dragItems.items[index]["itemColor"]
        borderColor: "lightgrey"
        itemTitle: dragItems.items[index]["title"]
        imageSource: dragItems.items[index]["imageUrl"]
        MouseArea {
          hoverEnabled: true
          anchors.fill: parent
          cursorShape: Qt.DragMoveCursor
        }
      }
    }
    function contains(itemID) {
      let index = dragItems.items.findIndex((item) => {return item["itemID"] === itemID;});
      return index !== -1;
    }
  }

  SectionTypeManager {
    id: sectionTypeManager
  }

  function displayItem(item) {
    itemView.itemID = item["itemID"];
    itemView.title = item["title"];
    itemView.imageUrl = item["imageUrl"];
    itemView.itemColor = item["itemColor"];
    itemView.audioUrl = item["audioUrl"];
    itemView.meaning = item["meaning"];
    itemView.init();
    stackView.push(itemView);
  }

  function editItem(item) {
    itemConfiguration.itemID = item["itemID"];
    itemConfiguration.currentType = item["type"];
    itemConfiguration.title = item["title"];
    itemConfiguration.image = item["imageUrl"];
    itemConfiguration.backgroundColor = item["itemColor"];
    itemConfiguration.meaning = item["meaning"];
    itemConfiguration.init();
    stackView.push(itemConfiguration);
  }

  function deleteItem(item) {
    itemToDelete = item;
    deleteItemDialog.targets = [item];
    deleteItemDialog.backgroundColor = item["itemColor"];
    deleteItemDialog.open();
  }

  function deleteSelectedItems() {
    deleteItemDialog.targets = selectedItems
    deleteItemDialog.backgroundColor = settings.accentColor;
    deleteItemDialog.open();
  }

  function selectItem(item) {
    selectedItems.push(item);
    isSelectMode = true;
  }

  function deselectItem(id) {
    let index = selectedItems.findIndex((item) => {return item["itemID"] === id;});
    if (index !== -1) {
      selectedItems.splice(index, 1);
    }
    if (selectedItems.length == 0) {
      isSelectMode = false;
    }
  }

  function clearSelectedItems() {
    selectedItems = [];
    isSelectMode = false;
    quitSelectMode();
  }

  function changeLanguage(language) {
    isStartPage = true;
    pages = [];
    pageTitle.title = "";
    pageTitle.visible = false;
    tables = [];
    itemConfiguration.typesNum = 8;
    library.openTable(language);
    refresh();
  }

  function moveItems(ids, sourceTable, targetTable) {
    ids.forEach((id) => library.moveItem(id, sourceTable, targetTable));
    dragItems.visible = false;
    clearSelectedItems();
    refresh();
  }

  function loadPage(parentTable, parentID, page) {
    tables.push(`${parentTable}_${parentID}`);
    isStartPage = false;
    itemConfiguration.typesNum = 2;
    library.openChildTable(parentID);
    pages.push(page);
    pageTitle.title = page;
    pageTitle.visible = true;
    refresh();
  }

  function refresh() {
    librarySections.model = library.sections;
    const sectionsLength = library.sections.length;
    itemsSection.visible = sectionsLength > 0;
    prompt.visible = settings.currentLanguage.length > 0 && sectionsLength === 0;
    toolBar.refresh();
    itemConfiguration.clear();
  }

  function pop() {
    if (toolBar.isSearchActive) {
      toolBar.toggleSearchLine();
    } else if (isSelectMode) {
      if (!dragItems.Drag.active) {
        clearSelectedItems();
      }
    } else if (tables.length > 1) {
      tables.pop();
      var table = tables[tables.length - 1];
      pages.pop();
      pageTitle.title = pages[pages.length - 1];
      library.openTable(table);
      refresh();
    } else if (!isStartPage) {
      changeLanguage(settings.currentLanguage);
    }
  }

  function hideSearchLine() {
    searchLine.display = false
    searchHide.running = true
  }

}
