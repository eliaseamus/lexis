import QtQuick
import QtQuick.Controls

Menu {
  MenuItem {
    text: qsTr("Select")
    onTriggered: toggleSelection()
  }
  MenuItem {
    text: qsTr("Edit")
    enabled: !gridItem.isSelected
    onTriggered: {
      libraryView.editItem(buildItemDict());
    }
  }
  MenuItem {
    text: qsTr("Move")
    enabled: libraryView.movableTypes.indexOf(type) !== -1
    onTriggered: {
      if (libraryView.isSelectMode) {
        moveDialog.ids = libraryView.selectedItems.map((item) => item["itemID"]);
      } else {
        moveDialog.ids = [itemID];
      }
      moveDialog.sourceTable = parentTable;
      moveDialog.model = library.getStructure();
      moveDialog.view.expandRecursively();
      moveDialog.open();
    }
  }
  MenuItem {
    text: qsTr("Delete")
    onTriggered: {
      if (libraryView.isSelectMode) {
        libraryView.deleteSelectedItems();
      } else {
        libraryView.deleteItem(buildItemDict());
      }
    }
  }
  MenuItem {
    text: qsTr("Time info")
    enabled: !gridItem.isSelected
    onTriggered: {
      timeInfoDialog.creationTime = creationTime;
      timeInfoDialog.modificationTime = modificationTime;
      timeInfoDialog.open();
    }
  }
}
