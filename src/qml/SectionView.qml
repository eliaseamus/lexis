import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QLexis
import "utils.js" as Utils

Item {
  id: librarySection
  property string title
  property LibraryItemProxyModel model
  property string itemToDelete
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
        property bool isImage: imageUrl.toString().length > 0
        width: 200
        height: 200
        color: mouseArea.containsPress ? itemColor.darker(1.1) : itemColor
        border.color: mouseArea.containsMouse ? settings.accentColor : palette.base
        border.width: 2
        radius: 10
        ColumnLayout {
          anchors.fill: parent
          anchors.margins: 10
          Image {
            Layout.preferredHeight: 150
            Layout.preferredWidth: 150
            Layout.alignment: Qt.AlignCenter
            fillMode: Image.PreserveAspectFit
            source: imageUrl
            visible: isImage
          }
          TextIcon {
            id: textIcon
            Layout.preferredHeight: 150
            Layout.preferredWidth: 150
            Layout.alignment: Qt.AlignCenter
            iconRadius: 180
            iconColor: itemColor
            iconTitle: title
            visible: !isImage
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
            } else if (mouse.button === Qt.RightButton) {
              contextMenu.popup()
            }
          }
          Menu {
            id: contextMenu
            MenuItem {
              text: qsTr("Edit")
              onTriggered: {
              }
            }
            MenuItem {
              text: qsTr("Delete")
              onTriggered: {
                itemToDelete = title;
                itemToDeleteType = type;
                deleteLanguageDialog.target = "\"%1\"".arg(title);
                deleteLanguageDialog.imageSource = imageUrl;
                deleteLanguageDialog.backgroundColor = itemColor;
                deleteLanguageDialog.open();
              }
            }
          }
        }
      }
    }
  }

  DeleteDialog {
    id: deleteLanguageDialog
    x: (main.width - width) / 2
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
  }

  Connections {
    target: deleteLanguageDialog

    function onAccepted() {
      library.deleteItem(sectionTypeManager.librarySectionType(itemToDeleteType), itemToDelete);
      startPage.refresh()
    }
  }

}
