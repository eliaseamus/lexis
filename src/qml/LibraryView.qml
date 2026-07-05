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
  property var parentStack: []
  property bool isStartPage: true
  property var pages: []
  property var itemToDelete
  property var selectedItems: []
  property bool isSelectMode
  property alias toolBar: toolBar
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
      visible: library.sections.length > 0 && !searchView.active
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
            currentParentId: parentStack.length === 0 ? 0 : parentStack[parentStack.length - 1]
            model: modelData.model
          }
        }
      }
    }

    ToolBar {id: toolBar}
  }

  SearchView {
    id: searchView
    anchors.fill: parent
    anchors.rightMargin: sideBar.width

    Connections {
      target: toolBar
      function onSearchRequest(query) {
        searchView.query = query
      }
    }

    onDismissed: {
      if (toolBar.isSearchActive) {
        toolBar.toggleSearchLine()
      }
    }

    onResultSelected: (result) => openSearchResult(result)
  }

  Connections {
    target: toolBar

    function onAddNewItem() {
      itemConfiguration.clear();
      clearSelectedItems();
      stackView.push(itemConfiguration);
    }

    function onStartQuiz() {
      openCurrentScopeQuiz();
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

    onDuplicateSelected: (itemId) => openDuplicateWord(itemId)
  }

  QuizView {
    id: quizView
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

  StatisticsDialog {
    id: statisticsDialog
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

  Dialog {
    id: quizErrorDialog
    property string message: ""
    x: (main.width - width) / 2 - sideBar.width
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
    standardButtons: Dialog.Ok
    title: qsTr("Quiz")
    Label {
      text: quizErrorDialog.message
      wrapMode: Text.WordWrap
      width: Math.min(360, main.width - sideBar.width - 80)
    }
  }

  SubjectGroupSuggestionDialog {
    id: subjectGroupSuggestionDialog
    property int pendingItemId: -1
    property int pendingParentId: 0
    parent: ApplicationWindow.overlay
    x: (main.width - width) / 2 - sideBar.width
    y: (main.height - height) / 2
    moveOnly: true

    onAccepted: {
      if (pendingItemId > 0 && selectedGroupId >= 0 && selectedGroupId !== pendingParentId) {
        library.moveItem(pendingItemId, selectedGroupId);
        refresh();
      }
    }
  }

  Dialog {
    id: suggestGroupInfoDialog
    property string message: ""
    x: (main.width - width) / 2 - sideBar.width
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
    standardButtons: Dialog.Ok
    title: qsTr("Suggest group")
    Label {
      text: suggestGroupInfoDialog.message
      wrapMode: Text.WordWrap
      width: Math.min(360, main.width - sideBar.width - 80)
    }
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
    itemView.frequencyTier = item["frequencyTier"] !== undefined ? item["frequencyTier"] : "";
    itemView.init();
    stackView.push(itemView);
  }

  function openDuplicateWord(itemId) {
    const item = library.getItem(itemId);
    if (!item) {
      return;
    }
    library.readAudio(itemId);
    itemView.itemID = item.itemID;
    itemView.title = item.title;
    itemView.imageUrl = item.imageUrl;
    itemView.itemColor = item.color;
    itemView.audioUrl = item.audioUrl;
    itemView.meaning = item.meaning;
    itemView.frequencyTier = item.frequencyTier !== undefined ? item.frequencyTier : "";
    itemView.init();
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

  function suggestGroupForItem(item) {
    const itemId = item["itemID"];
    const parentId = library.itemParentId(itemId);
    const suggestions = library.suggestSubjectGroups(item["title"], item["meaning"] || "",
                                                     itemId, parentId);
    if (suggestions.length === 0) {
      suggestGroupInfoDialog.message = qsTr("No strong group match found for this word.");
      suggestGroupInfoDialog.open();
      return;
    }
    subjectGroupSuggestionDialog.pendingItemId = itemId;
    subjectGroupSuggestionDialog.pendingParentId = parentId;
    subjectGroupSuggestionDialog.wordTitle = item["title"];
    subjectGroupSuggestionDialog.suggestions = suggestions;
    subjectGroupSuggestionDialog.currentParentId = parentId;
    subjectGroupSuggestionDialog.init();
    subjectGroupSuggestionDialog.open();
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
    parentStack = [];
    itemConfiguration.typesNum = 8;
    library.openLanguage(language);
    refresh();
  }

  function moveItems(ids, targetParentId) {
    ids.forEach((id) => library.moveItem(id, targetParentId));
    dragItems.visible = false;
    clearSelectedItems();
    refresh();
  }

  function loadPage(parentID, page) {
    parentStack.push(parentID);
    isStartPage = false;
    itemConfiguration.typesNum = 2;
    library.openFolder(parentID);
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
    } else if (parentStack.length > 0) {
      parentStack.pop();
      pages.pop();
      if (parentStack.length > 0) {
        library.openFolder(parentStack[parentStack.length - 1]);
        pageTitle.title = pages[pages.length - 1];
        pageTitle.visible = true;
      } else {
        library.openRoot();
        pageTitle.title = "";
        pageTitle.visible = false;
        isStartPage = true;
      }
      refresh();
    } else if (!isStartPage) {
      changeLanguage(settings.currentLanguage);
    }
  }

  function openSearchResult(result) {
    if (toolBar.isSearchActive) {
      toolBar.toggleSearchLine()
    }
    clearSelectedItems()

    const path = library.ancestorPath(result.itemId)
    parentStack = []
    pages = []

    if (path.length <= 1) {
      library.openRoot()
      isStartPage = true
      pageTitle.title = ""
      pageTitle.visible = false
    } else {
      for (let i = 0; i < path.length - 1; i++) {
        parentStack.push(path[i].id)
        pages.push(path[i].title)
      }
      library.openFolder(parentStack[parentStack.length - 1])
      pageTitle.title = pages[pages.length - 1]
      pageTitle.visible = true
      isStartPage = false
    }
    refresh()

    if (result.type === "Word") {
      const item = library.getItem(result.itemId)
      if (item) {
        library.readAudio(result.itemId)
        displayItem({
          "itemID": item.itemID,
          "title": item.title,
          "creationTime": item.creationTime,
          "modificationTime": item.modificationTime,
          "type": item.type,
          "imageUrl": item.imageUrl,
          "itemColor": item.color,
          "audioUrl": item.audioUrl,
          "meaning": item.meaning,
          "frequencyTier": item.frequencyTier !== undefined ? item.frequencyTier : ""
        })
      }
    } else {
      loadPage(result.itemId, result.title)
    }
  }

  function hideSearchLine() {
    if (toolBar.isSearchActive) {
      toolBar.toggleSearchLine()
    }
  }

  function openLibraryStatistics() {
    statisticsDialog.libraryMode = true
    statisticsDialog.open()
  }

  function openQuiz(scopeRootId, scopeTitle) {
    hideSearchLine()
    clearSelectedItems()
    const scopeError = quizScopeErrorMessage(scopeRootId)
    if (scopeError.length > 0) {
      quizErrorDialog.message = scopeError
      quizErrorDialog.open()
      return
    }
    quizView.init(scopeRootId, scopeTitle)
    stackView.push(quizView)
  }

  function quizScopeErrorMessage(scopeRootId) {
    const wordCount = library.wordsInScope(scopeRootId).length
    if (wordCount === 0) {
      return qsTr("This scope has no words to quiz.")
    }
    if (wordCount < 2) {
      return qsTr("Need at least two words in this scope to start a quiz.")
    }
    return ""
  }

  function openCurrentScopeQuiz() {
    const scopeRootId = parentStack.length === 0 ? 0 : parentStack[parentStack.length - 1]
    const scopeTitle = pages.length === 0 ? settings.currentLanguage : pages[pages.length - 1]
    openQuiz(scopeRootId, scopeTitle)
  }

}
