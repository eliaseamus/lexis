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
        width: 200
        height: 200
        color: mouseArea.containsPress ? itemColor.darker(1.1) : itemColor
        border.color: mouseArea.containsMouse ? settings.accentColor : palette.base
        border.width: 2
        radius: 10
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
          onClicked: (mouse) => {
            if (mouse.button === Qt.LeftButton) {
              if (type == "Word") {
                libraryView.displayItem(title, imageUrl, itemColor)
              } else {
                libraryView.loadPage(parentTable, itemID, title)
              }
            } else if (mouse.button === Qt.RightButton) {
              contextMenu.popup()
            }
          }
          Menu {
            id: contextMenu
            MenuItem {
              text: qsTr("Edit")
              onTriggered: {
                var item = [];
                item["itemID"] = itemID;
                item["title"] = title;
                item["type"] = sectionTypeManager.librarySectionType(type);
                item["imageUrl"] = imageUrl;
                item["itemColor"] = itemColor;
                libraryView.editItem(item);
              }
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
              onTriggered: {
                timeInfoDialog.creationTime = creationTime;
                timeInfoDialog.modificationTime = modificationTime;
                timeInfoDialog.open();
              }
            }
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
