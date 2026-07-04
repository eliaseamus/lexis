import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QLexis
import "utils.js" as Utils

Dialog {
  id: confirmDialog
  property var matches: []
  modal: true
  padding: 16
  implicitWidth: Math.min(main.width - sideBar.width - 80, Math.max(480, cardsGrid.implicitWidth + padding * 2))

  function formatPath(item) {
    if (item.breadcrumb && item.breadcrumb.length > 0) {
      return item.breadcrumb.split(" \u203a ").join(" \u2192 ");
    }
    return qsTr("Start page") + " \u2192 " + item.title;
  }

  ColumnLayout {
    anchors.fill: parent
    spacing: 16

    PrettyLabel {
      Layout.fillWidth: true
      Layout.preferredWidth: confirmDialog.implicitWidth - confirmDialog.padding * 2
      title: qsTr("This word already exists in your library.")
      horizontalAlignment: Qt.AlignHCenter
      wrapMode: Text.WordWrap
    }

    ScrollView {
      Layout.fillWidth: true
      Layout.preferredHeight: Math.min(320, cardsGrid.implicitHeight + 8)
      clip: true
      ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

      GridLayout {
        id: cardsGrid
        width: confirmDialog.implicitWidth - confirmDialog.padding * 2
        columns: Math.min(2, Math.max(1, confirmDialog.matches.length))
        columnSpacing: 20
        rowSpacing: 20
        Layout.alignment: Qt.AlignHCenter

        Repeater {
          model: confirmDialog.matches

          Rectangle {
            required property var modelData
            property color cardBackground:
              modelData.itemColor.length > 0 ? modelData.itemColor : "white"
            property color cardTextColor: Utils.getFgColor(cardBackground)

            Layout.preferredWidth: 200
            Layout.preferredHeight: 260
            Layout.alignment: Qt.AlignHCenter
            radius: 10
            color: cardBackground
            border.color: "lightgrey"
            border.width: 2

            ColumnLayout {
              anchors.fill: parent
              anchors.margins: 12
              spacing: 8

              Icon {
                Layout.preferredWidth: 120
                Layout.preferredHeight: 120
                Layout.alignment: Qt.AlignHCenter
                iconRadius: 120
                iconColor: cardBackground
                iconTitle: modelData.title
                image: modelData.hasImage ?
                       library.itemImageUrl(modelData.itemId).toString() : ""
              }

              Text {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                text: modelData.title
                font.bold: true
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                elide: Text.ElideRight
                color: cardTextColor
              }

              Item {
                Layout.fillHeight: true
              }

              Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: pathText.implicitHeight + 12
                radius: 6
                color: Qt.rgba(0, 0, 0, 0.12)

                Text {
                  id: pathText
                  width: parent.width - 12
                  anchors.centerIn: parent
                  horizontalAlignment: Text.AlignHCenter
                  wrapMode: Text.WordWrap
                  font.pixelSize: 11
                  text: confirmDialog.formatPath(modelData)
                  color: cardTextColor
                }
              }
            }
          }
        }
      }
    }

    PrettyLabel {
      Layout.fillWidth: true
      Layout.preferredWidth: confirmDialog.implicitWidth - confirmDialog.padding * 2
      dimmed: true
      title: qsTr("Add another entry with the same title?")
      horizontalAlignment: Qt.AlignHCenter
      wrapMode: Text.WordWrap
    }

    OkCancel {
      Layout.alignment: Qt.AlignHCenter
      okay: function () {
        confirmDialog.accept();
      }
      cancel: function () {
        confirmDialog.reject();
      }
    }
  }
}
