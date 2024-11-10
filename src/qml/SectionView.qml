import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QLexis
import "utils.js" as Utils

Item {
  id: librarySection
  property string title
  property string parentTable
  property LibraryItemProxyModel model
  Layout.fillWidth: true
  Layout.preferredHeight: grid.rowCount * grid.cellHeight + titleShelf.height + 30

  ColumnLayout {
    anchors.fill: parent
    RowLayout {
      Layout.fillWidth: true
      PrettyLabel {
        id: titleShelf
        title: librarySection.title
        Layout.topMargin: 10
        Layout.bottomMargin: 10
      }
      Item {
        Layout.fillWidth: true
      }

      RoundButton {
        id: sortOrder
        enabled: librarySection.model.size() > 1
        hoverEnabled: true
        Layout.alignment: Qt.AlignRight
        flat: true
        property url iconSource: getIconSource()
        icon.source: iconSource
        onClicked: {
          librarySection.model.toggleSort();
          iconSource = getIconSource();
        }
        ToolTip {
          visible: sortOrder.hovered
          text: qsTr("Sort order")
        }

        function getIconSource() {
          var order = librarySection.model.sortOrder;
          return order === Qt.AscendingOrder ?
            "icons/sort-ascending.png" :
            "icons/sort-descending.png";
        }
      }
      Connections {
        target: librarySection.model

        function onChanged() {
          sortOrder.enabled = librarySection.model.size() > 1;
        }
      }
    }

    GridView {
      id: grid
      Layout.fillWidth: true
      Layout.fillHeight: true
      interactive: false
      property int rowCount: {
        var totalCellWidth = count * cellWidth;
        var rows = totalCellWidth / width + 0.5;

        if (width < 2 * cellWidth) {
          rows = count;
        }

        return Math.round(rows);
      }

      cellWidth: 215
      cellHeight: 215
      model: librarySection.model

      delegate: SectionItem {
        id: gridItem
        backgroundColor: (mouseArea.containsPress || mouseArea.drag.active) ||
                         (libraryView.isSelectMode && isSelected) ?
                         itemColor.darker(1.2) :
                         itemColor
        borderColor: (mouseArea.containsMouse && !mouseArea.drag.active) || dropArea.containsDrop ?
                     settings.accentColor :
                     palette.base
        itemTitle: title
        imageSource: imageUrl

        MouseArea {
          id: mouseArea
          anchors.fill: parent
          drag.target: dragItems
          Drag.hotSpot.x: dragItems.width / 2
          Drag.hotSpot.y: dragItems.height / 2
          Drag.source: mouseArea
          hoverEnabled: true
          cursorShape: drag.active ? Qt.DragMoveCursor : Qt.PointingHandCursor
          acceptedButtons: Qt.LeftButton | Qt.RightButton
          onPressAndHold: toggleSelection()
          onClicked: (mouse) => {
            if (mouse.button === Qt.LeftButton) {
              if (libraryView.isSelectMode) {
                toggleSelection();
              } else {
                if (type == "Word") {
                  library.readAudio(itemID);
                  libraryView.displayItem(buildItemDict());
                } else {
                  libraryView.loadPage(parentTable, itemID, title)
                }
              }
            } else if (mouse.button === Qt.RightButton) {
              contextMenu.popup()
            }
          }
          onReleased: {
            if (drag.active) {
              dragItems.drop();
              dragItems.Drag.active = false;
              dragItems.visible = false;
            }
          }
          onPositionChanged: (mouse) => {
            if (libraryView.movableTypes.indexOf(type) === -1) {
              return;
            }
            const point = mapToItem(libraryView,
                                    mapFromItem(grid,
                                                gridItem.x + mouse.x - dragItems.width / 2,
                                                gridItem.y + mouse.y - dragItems.height / 2));
            dragItems.x = point.x;
            dragItems.y = point.y;
            if (drag.active) {
              if (!dragItems.visible) {
                if (libraryView.isSelectMode) {
                  var items = libraryView.selectedItems;
                  items = items.filter((item) => {
                    const typeName = sectionTypeManager.librarySectionTypeName(item["type"]);
                    return libraryView.movableTypes.indexOf(typeName) !== -1;
                  });
                  let index = items.findIndex((item) => {return item["itemID"] === itemID;});
                  if (index !== -1) {
                    var currentItem = items[index];
                    items.splice(index, 1);
                    items.push(currentItem);
                  } else {
                    toggleSelection();
                    return;
                  }
                  dragItems.items = items;
                } else {
                  dragItems.items = [buildItemDict()];
                }
                dragItems.Drag.active = true;
                dragItems.visible = true;
              }
            }
          }

          Menu {
            id: contextMenu
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
        }

        Connections {
          target: libraryView
          function onQuitSelectMode() {
            gridItem.isSelected = false;
          }
        }

        DropArea {
          id: dropArea
          anchors.fill: parent
          property bool containsDrop
          onEntered: {
            if (type == "Word") {
              return;
            } else if (dragItems.contains(itemID)) {
              return;
            }
            containsDrop = true;
          }
          onExited: {
            containsDrop = false;
          }
          Connections {
            target: dragItems
            function onDrop() {
              if (dropArea.containsDrop) {
                if (libraryView.isSelectMode) {
                  libraryView.clearSelectedItems();
                }
                const ids = dragItems.items.map((item) => item["itemID"]);
                const targetTable = "%1_%2".arg(parentTable).arg(itemID);
                libraryView.moveItems(ids, parentTable, targetTable);
              }
            }
          }
        }

        function buildItemDict() {
          var item = [];
          item["itemID"] = itemID;
          item["title"] = title;
          item["creationTime"] = creationTime;
          item["modificationTime"] = modificationTime;
          item["type"] = sectionTypeManager.librarySectionType(type);
          item["imageUrl"] = imageUrl;
          item["itemColor"] = itemColor;
          item["audioUrl"] = audioUrl;
          item["meaning"] = meaning;
          return item;
        }

        function toggleSelection() {
          if (!gridItem.isSelected) {
            libraryView.selectItem(buildItemDict());
            gridItem.isSelected = true;
          } else {
            libraryView.deselectItem(itemID);
            gridItem.isSelected = false;
          }
        }
      }
    }
  }
}
