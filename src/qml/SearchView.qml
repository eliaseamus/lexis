import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QLexis
import "utils.js" as Utils

Item {
  id: searchView
  property string query: ""
  property bool active: query.trim().length > 0

  signal resultSelected(var result)
  signal dismissed()

  visible: active
  z: 2

  Rectangle {
    anchors.fill: parent
    color: "#66000000"

    MouseArea {
      anchors.fill: parent
      onClicked: searchView.dismissed()
    }
  }

  Pane {
    id: panel
    anchors.top: parent.top
    anchors.topMargin: 70
    anchors.horizontalCenter: parent.horizontalCenter
    width: Math.min(parent.width - sideBar.width - 80, 640)
    height: Math.min(parent.height - 120, 480)
    padding: 12

    ColumnLayout {
      anchors.fill: parent
      spacing: 8

      PrettyLabel {
        Layout.fillWidth: true
        title: qsTr("Search results")
      }

      ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true

        ListView {
          id: resultsList
          model: searchView.active ? library.search(searchView.query) : []
          spacing: 6
          boundsBehavior: Flickable.StopAtBounds

          delegate: ItemDelegate {
            id: resultRow
            required property var modelData
            property color cardBackground:
              modelData.itemColor.length > 0 ? modelData.itemColor : "white"
            property color rowBackground: resultRow.hovered ? settings.accentColor : cardBackground
            property color rowTextColor: Utils.getFgColor(rowBackground)

            width: resultsList.width
            implicitHeight: resultLayout.implicitHeight + 16
            hoverEnabled: true

            background: Rectangle {
              color: resultRow.rowBackground
              radius: 8
            }

            onClicked: searchView.resultSelected(modelData)

            RowLayout {
              id: resultLayout
              anchors.fill: parent
              anchors.margins: 8
              spacing: 12

              Icon {
                Layout.preferredWidth: 56
                Layout.preferredHeight: 56
                iconRadius: 56
                iconColor: resultRow.cardBackground
                iconTitle: modelData.title
                image: modelData.hasImage ? library.itemImageUrl(modelData.itemId).toString() : ""
              }

              ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                  Layout.fillWidth: true
                  text: modelData.title
                  color: resultRow.rowTextColor
                  font.bold: true
                  elide: Text.ElideRight
                }

                Text {
                  Layout.fillWidth: true
                  visible: modelData.breadcrumb.length > 0
                  text: modelData.breadcrumb
                  color: resultRow.rowTextColor
                  opacity: 0.75
                  elide: Text.ElideRight
                  font.pixelSize: 12
                }

                Text {
                  Layout.fillWidth: true
                  visible: modelData.type === "Word" && modelData.meaning.length > 0
                  text: modelData.meaning
                  color: resultRow.rowTextColor
                  opacity: 0.75
                  elide: Text.ElideRight
                  font.italic: true
                  font.pixelSize: 12
                }
              }

              Text {
                text: modelData.type
                color: resultRow.rowTextColor
                opacity: 0.75
                font.pixelSize: 11
              }
            }
          }
        }
      }

      PrettyLabel {
        Layout.fillWidth: true
        visible: searchView.active && resultsList.count === 0
        dimmed: true
        title: qsTr("No matches found")
      }
    }
  }
}
