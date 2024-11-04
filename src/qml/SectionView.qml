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
  property int itemToDelete: -1
  property string itemToDeleteType
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

        if (width < 2 * cellWidth)
          rows = count

        return Math.round(rows);
      }

      cellWidth: 215
      cellHeight: 215
      model: librarySection.model

      delegate: Rectangle {
        id: gridItem
        property bool isSelected
        width: 200
        height: 200
        color: mouseArea.containsPress ? itemColor.darker(1.1) : itemColor
        border.color: mouseArea.containsMouse ? settings.accentColor : palette.base
        border.width: 2
        radius: 10
        RoundButton {
          flat: true
          visible: isSelected
          icon.source: "icons/check.png"
          icon.color: settings.fgColor
          Material.background: settings.accentColor
        }
        ColumnLayout {
          anchors.fill: parent
          anchors.margins: 10
          Icon {
            Layout.preferredHeight: 150
            Layout.preferredWidth: 150
            Layout.alignment: Qt.AlignCenter
            image: imageUrl
            iconRadius: 150
            iconColor: itemColor
            iconTitle: title
          }
          Text {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 180
            horizontalAlignment: Qt.AlignHCenter
            text: title
            elide: Text.ElideRight
            color: Utils.getFgColor(itemColor)
          }
        }
        MouseArea {
          id: mouseArea
          anchors.fill: parent
          hoverEnabled: true
          cursorShape: Qt.PointingHandCursor
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
            }
            MenuItem {
              text: qsTr("Delete")
              onTriggered: {
                itemToDelete = itemID;
                itemToDeleteType = type;
                deleteItemDialog.target = title;
                deleteItemDialog.imageUrl = imageUrl;
                deleteItemDialog.backgroundColor = itemColor;
                deleteItemDialog.open();
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

        function buildItemDict() {
          var item = [];
          item["itemID"] = itemID;
          item["title"] = title;
          item["creationTime"] = creationTime;
          item["modificationTime"] = modificationTime;
          item["type"] = type;
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

  Connections {
    target: deleteItemDialog

    function onAccepted() {
      if (itemToDelete == -1) {
        return;
      }
      library.deleteItem(itemToDelete, sectionTypeManager.librarySectionType(itemToDeleteType));
      libraryView.refresh()
    }
  }

}
