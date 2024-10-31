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
  property var pages: []

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
      itemConfiguration.clear()
      stackView.push(itemConfiguration)
    }

    function onGoBack() {
      libraryView.pop()
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

  SectionTypeManager {
    id: sectionTypeManager
  }

  function displayItem(title, image, color) {
    itemView.title = title;
    itemView.imageUrl = image;
    itemView.itemColor = color;
    itemView.init();
    stackView.push(itemView);
  }

  function editItem(item) {
    itemConfiguration.itemID = Number(item["itemID"]);
    itemConfiguration.currentType = Number(item["type"]);
    itemConfiguration.title = String(item["title"]);
    itemConfiguration.image = String(item["imageUrl"]);
    itemConfiguration.backgroundColor = String(item["itemColor"]);
    itemConfiguration.init();
    stackView.push(itemConfiguration);
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
    } else if (tables.length > 1) {
      tables.pop();
      var table = tables.pop();
      pages.pop();
      pageTitle.title = pages.pop();
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
