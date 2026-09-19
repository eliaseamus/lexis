import QtQuick
import QtQuick.Controls

Menu {
  readonly property bool singleTarget: !gridItem.isSelected || libraryView.selectedItems.length === 1

  MenuItem {
    text: qsTr("Select")
    onTriggered: toggleSelection()
  }
  MenuItem {
    text: qsTr("Edit")
    visible: singleTarget
    height: visible ? implicitHeight : 0
    onTriggered: {
      libraryView.editItem(buildItemDict());
    }
  }
  MenuItem {
    text: qsTr("Move")
    visible: libraryView.movableTypes.indexOf(type) !== -1
    height: visible ? implicitHeight : 0
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
    text: qsTr("Suggest group")
    visible: type === "Word" && singleTarget
    height: visible ? implicitHeight : 0
    onTriggered: libraryView.suggestGroupForItem(buildItemDict())
  }
  MenuItem {
    text: libraryView.knownActionLabel(known === true)
    visible: libraryView.canMarkKnown(type === "Word")
    height: visible ? implicitHeight : 0
    onTriggered: libraryView.toggleKnownForContext(buildItemDict())
  }
  MenuItem {
    text: libraryView.pinnedActionLabel(pinned === true)
    visible: libraryView.canMarkKnown(type === "Word")
    height: visible ? implicitHeight : 0
    onTriggered: libraryView.togglePinnedForContext(buildItemDict())
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
    visible: singleTarget
    height: visible ? implicitHeight : 0
    onTriggered: {
      statisticsDialog.libraryMode = false
      statisticsDialog.itemId = itemID
      statisticsDialog.open()
    }
  }
  MenuItem {
    text: qsTr("Quiz")
    visible: singleTarget
    height: visible ? implicitHeight : 0
    onTriggered: {
      if (type === "Word") {
        libraryView.openQuiz(librarySection.currentParentId, librarySection.title)
      } else {
        libraryView.openQuiz(itemID, title)
      }
    }
  }
}
