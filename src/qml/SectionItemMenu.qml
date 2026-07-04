import QtQuick
import QtQuick.Controls

Menu {
  MenuItem {
    text: qsTr("Select")
    onTriggered: toggleSelection()
  }
  MenuItem {
    text: qsTr("Edit")
    enabled: !gridItem.isSelected || libraryView.selectedItems.length === 1
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
      moveDialog.sourceParentId = librarySection.currentParentId;
      moveDialog.model = library.getStructure();
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
    text: qsTr("Statistics")
    enabled: !gridItem.isSelected || libraryView.selectedItems.length === 1
    onTriggered: {
      statisticsDialog.libraryMode = false
      statisticsDialog.itemId = itemID
      statisticsDialog.open()
    }
  }
  MenuItem {
    text: qsTr("Quiz")
    enabled: !gridItem.isSelected || libraryView.selectedItems.length === 1
    onTriggered: {
      if (type === "Word") {
        libraryView.openQuiz(librarySection.currentParentId, librarySection.title)
      } else {
        libraryView.openQuiz(itemID, title)
      }
    }
  }
}
