import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QLexis

Item {
  id: librarySection
  property string title
  property LibraryItemModel model

  ColumnLayout {
    Text {
      text: title
      Layout.bottomMargin: 10
    }

    GridView {
      Layout.fillWidth: true
      cellWidth: 220
      cellHeight: 200
      width: 600
      height: 200
      model: librarySection.model
      delegate: Rectangle {
        width: 200
        height: 200
        color: itemColor
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
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            text: title
          }
        }
      }
    }
  }

}
