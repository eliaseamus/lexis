import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QLexis

Item {
  id: librarySection
  property string title
  property LibraryItemModel model
  Layout.fillWidth: true
  Layout.topMargin: 30
  Layout.preferredHeight: grid.rowCount * grid.cellHeight + titleShelf.height + 30

  ColumnLayout {
    anchors.fill: parent
    Label {
      id: titleShelf
      text: title
      color: "white"
      Layout.topMargin: 10
      Layout.bottomMargin: 10
      Layout.leftMargin: 5
      background: Rectangle {
        anchors.fill: parent
        anchors.margins: -5
        color: Material.accentColor
        radius: 10
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
        width: 200
        height: 200
        color: mouseArea.containsPress ? itemColor.darker(1.3) : itemColor
        border.color: Material.accentColor
        border.width: mouseArea.containsMouse ? 2 : 0
        radius: 10
        ColumnLayout {
          anchors.fill: parent
          anchors.margins: 10
          Image {
            Layout.fillHeight: true
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectFit
            source: imageUrl
          }
          Text {
            Layout.alignment: Qt.AlignHCenter
            text: title
          }
        }
        MouseArea {
          id: mouseArea
          anchors.fill: parent
          hoverEnabled: true
          cursorShape: Qt.PointingHandCursor
          onClicked: (mouse) => {
            if (mouse.button === Qt.LeftButton) {
            }
          }
        }
      }
    }
  }

}
