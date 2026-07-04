import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QLexis

Dialog {
  id: moveDialog
  property list<int> ids
  property int sourceParentId
  property int selectedParentId: sourceParentId
  property var filteredMatches: []
  property alias model: view.model
  property alias view: view

  implicitWidth: Math.min(main.width - sideBar.width - 80, 520)
  padding: 16

  onOpened: prepareDialog()

  function prepareDialog() {
    searchField.text = "";
    filteredMatches = [];
    selectedParentId = sourceParentId;
    Qt.callLater(selectSourceParent);
  }

  function selectSourceParent() {
    if (!view.model) {
      return;
    }
    const index = view.model.findIndexById(sourceParentId);
    if (!index.valid) {
      return;
    }
    centerOnIndex(index);
    selectedParentId = sourceParentId;
  }

  function centerOnIndex(index) {
    expandToIndex(index);
    view.selectionModel.setCurrentIndex(index, ItemSelectionModel.ClearAndSelect);
    Qt.callLater(function () {
      view.positionViewAtIndex(index, TableView.AlignCenter);
    });
  }

  function expandToIndex(index) {
    let current = index;
    while (current.valid) {
      view.expand(current);
      current = view.model.parent(current);
    }
  }

  function updateFilteredMatches() {
    const query = searchField.text.trim();
    filteredMatches = query.length > 0 ? view.model.findMatches(query) : [];
    if (query.length === 0) {
      Qt.callLater(function () {
        const index = view.model.findIndexById(selectedParentId);
        if (index.valid) {
          centerOnIndex(index);
        }
      });
    }
  }

  ColumnLayout {
    anchors.fill: parent
    spacing: 12

    TextField {
      id: searchField
      Layout.fillWidth: true
      placeholderText: qsTr("Filter subject groups")
      onTextChanged: moveDialog.updateFilteredMatches()
    }

    Rectangle {
      Layout.fillWidth: true
      Layout.preferredHeight: Math.min(400, main.height / 2)
      border.color: "lightgrey"

      TreeView {
        id: view
        anchors.fill: parent
        anchors.margins: 1
        ScrollBar.vertical: ScrollBar {}
        visible: searchField.text.trim().length === 0
        clip: true
        columnWidthProvider: function(column) {
          return width - 2;
        }
        selectionModel: ItemSelectionModel {
          onCurrentChanged: {
            if (searchField.text.trim().length === 0 && currentIndex.valid) {
              selectedParentId = view.model.columnData(currentIndex, 1);
            }
          }
        }
        delegate: TreeViewDelegate {
          implicitWidth: treeView.width
          contentItem: Label {
            width: treeView.width - leftMargin * 2
            clip: false
            text: column > 0 ? "" : model.display
            elide: Text.ElideRight
            color: row === treeView.currentRow ? settings.fgColor : "black"
          }
          background: Rectangle {
            anchors.fill: parent
            color: row === treeView.currentRow ? settings.accentColor :
                   (treeView.alternatingRows && row % 2 !== 0) ? "lightgrey" : "white"
          }
        }
      }

      ListView {
        id: filteredView
        anchors.fill: parent
        anchors.margins: 1
        visible: searchField.text.trim().length > 0
        clip: true
        model: moveDialog.filteredMatches
        ScrollBar.vertical: ScrollBar {}

        delegate: ItemDelegate {
          id: filteredRow
          required property var modelData
          required property int index
          property bool isSelected: modelData.id === moveDialog.selectedParentId

          width: filteredView.width
          onClicked: moveDialog.selectedParentId = modelData.id

          contentItem: Label {
            text: filteredRow.modelData.breadcrumb
            elide: Text.ElideRight
            color: filteredRow.isSelected || filteredRow.hovered ?
                   settings.fgColor : "black"
          }

          background: Rectangle {
            implicitHeight: 40
            color: filteredRow.isSelected || filteredRow.hovered ? settings.accentColor :
                   (filteredRow.index % 2 !== 0) ? "lightgrey" : "white"
          }
        }
      }
    }

    OkCancel {
      Layout.alignment: Qt.AlignHCenter
      okay: function () {
        if (sourceParentId !== selectedParentId) {
          libraryView.moveItems(ids, selectedParentId);
        } else {
          libraryView.clearSelectedItems();
        }
        moveDialog.accept();
      }
      cancel: function () {
        moveDialog.reject();
      }
    }
  }
}
