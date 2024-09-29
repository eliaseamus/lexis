import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QLexis

Item {
  id: librarySection
  property string title
  property LibraryItemModel model
  Layout.fillWidth: true
  Layout.fillHeight: true

  ColumnLayout {
    anchors.fill: parent
    Text {
      text: title
      Layout.bottomMargin: 10
    }

    GridView {
      Layout.fillWidth: true
      Layout.fillHeight: true
      width: 800
      height: 400
      cellWidth: 220
      cellHeight: 220
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
