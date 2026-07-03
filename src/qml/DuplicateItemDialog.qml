import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QLexis
import "utils.js" as Utils

Dialog {
  id: duplicateDialog
  property var matches: []
  property var duplicateGroups: []
  property string message: qsTr("An item with this title already exists in your library.")
  property bool resolvable: true
  modal: true

  implicitWidth: Math.min(main.width - sideBar.width - 80, 560)
  padding: 16

  signal duplicatesResolved()

  onAboutToShow: initFromMatches(matches)

  function initFromMatches(items) {
    matches = items;
    const grouped = {};
    for (let i = 0; i < items.length; i++) {
      const item = items[i];
      const key = item.title.toLocaleLowerCase();
      if (!grouped[key]) {
        grouped[key] = {
          displayTitle: item.title,
          items: [],
          selectedId: item.itemId
        };
      }
      grouped[key].items.push(item);
    }
    duplicateGroups = Object.values(grouped);
  }

  function formatPath(item) {
    const parts = [];
    if (item.parentPath && item.parentPath.length > 0) {
      parts.push(...item.parentPath.split(" \u203a "));
    } else {
      parts.push(qsTr("Start page"));
    }
    return parts.join(" \u2192 ");
  }

  function selectCopy(groupIndex, itemId) {
    const groups = duplicateGroups.slice();
    groups[groupIndex] = Object.assign({}, groups[groupIndex], { selectedId: itemId });
    duplicateGroups = groups;
  }

  function resolveDuplicates() {
    for (let i = 0; i < duplicateGroups.length; i++) {
      const group = duplicateGroups[i];
      library.resolveDuplicateGroup(group.selectedId, group.items);
    }
    library.reloadCurrentFolder();
    duplicatesResolved();
    close();
  }

  ColumnLayout {
    anchors.fill: parent
    spacing: 16

    PrettyLabel {
      Layout.fillWidth: true
      Layout.preferredWidth: duplicateDialog.implicitWidth - duplicateDialog.padding * 2
      title: duplicateDialog.message
      horizontalAlignment: Qt.AlignHCenter
      wrapMode: Text.WordWrap
    }

    PrettyLabel {
      Layout.fillWidth: true
      Layout.preferredWidth: duplicateDialog.implicitWidth - duplicateDialog.padding * 2
      visible: duplicateDialog.resolvable && duplicateDialog.duplicateGroups.length > 0
      dimmed: true
      title: qsTr("Select the copy to keep in each group. Other copies will be deleted.")
      horizontalAlignment: Qt.AlignHCenter
      wrapMode: Text.WordWrap
    }

    ScrollView {
      Layout.fillWidth: true
      Layout.preferredHeight: Math.min(420, groupsLayout.implicitHeight + 8)
      clip: true
      ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

      ColumnLayout {
        id: groupsLayout
        width: duplicateDialog.implicitWidth - duplicateDialog.padding * 2
        spacing: 24

        Repeater {
          model: duplicateDialog.duplicateGroups

          ColumnLayout {
            required property var modelData
            required property int index
            property int groupIndex: index
            Layout.fillWidth: true
            spacing: 12

            PrettyLabel {
              Layout.fillWidth: true
              Layout.alignment: Qt.AlignHCenter
              visible: duplicateDialog.duplicateGroups.length > 1
              dimmed: true
              title: qsTr("Duplicate word: %1").arg(modelData.displayTitle)
              horizontalAlignment: Qt.AlignHCenter
              wrapMode: Text.WordWrap
            }

            GridLayout {
              Layout.fillWidth: true
              Layout.alignment: Qt.AlignHCenter
              columns: Math.min(2, modelData.items.length)
              columnSpacing: 20
              rowSpacing: 20

              Repeater {
                model: modelData.items

                delegate: Item {
                  id: copyCardHost
                  required property var modelData
                  property color cardBackground:
                    modelData.itemColor.length > 0 ? modelData.itemColor : "white"
                  property color cardTextColor: Utils.getFgColor(cardBackground)
                  property bool isSelected:
                    duplicateDialog.duplicateGroups[groupIndex].selectedId === modelData.itemId

                  Layout.preferredWidth: 200
                  Layout.preferredHeight: 280
                  Layout.alignment: Qt.AlignHCenter

                  MouseArea {
                    id: copyCardArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: duplicateDialog.selectCopy(groupIndex, modelData.itemId)

                    Rectangle {
                      anchors.fill: parent
                      color: copyCardHost.cardBackground
                      radius: 10
                      border.color: copyCardHost.isSelected ? settings.accentColor : "lightgrey"
                      border.width: copyCardHost.isSelected ? 3 : 2

                      ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 8

                        Icon {
                          Layout.preferredWidth: 120
                          Layout.preferredHeight: 120
                          Layout.alignment: Qt.AlignHCenter
                          iconRadius: 120
                          iconColor: copyCardHost.cardBackground
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
                          color: copyCardHost.cardTextColor
                        }

                        Text {
                          Layout.fillWidth: true
                          Layout.alignment: Qt.AlignHCenter
                          horizontalAlignment: Text.AlignHCenter
                          text: modelData.type
                          font.pixelSize: 11
                          opacity: 0.85
                          color: copyCardHost.cardTextColor
                        }

                        Item {
                          Layout.fillHeight: true
                        }

                        Rectangle {
                          Layout.fillWidth: true
                          Layout.preferredHeight: pathText.implicitHeight + 12
                          radius: 6
                          color: Qt.rgba(0, 0, 0, 0.12)
                          border.color: copyCardHost.isSelected ? settings.accentColor : "transparent"
                          border.width: copyCardHost.isSelected ? 1 : 0

                          Text {
                            id: pathText
                            width: parent.width - 12
                            anchors.centerIn: parent
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                            font.pixelSize: 11
                            text: duplicateDialog.formatPath(modelData)
                            color: copyCardHost.cardTextColor
                          }
                        }

                        Text {
                          Layout.alignment: Qt.AlignHCenter
                          visible: copyCardHost.isSelected
                          text: qsTr("Keep this copy")
                          font.bold: true
                          font.pixelSize: 11
                          color: copyCardHost.cardTextColor
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    RowLayout {
      Layout.fillWidth: true
      Layout.alignment: Qt.AlignHCenter
      spacing: 16

      Item {
        Layout.fillWidth: true
      }

      PrettyButton {
        visible: duplicateDialog.resolvable
        enabled: duplicateDialog.duplicateGroups.length > 0
        text: qsTr("Keep selected copies")
        onClicked: duplicateDialog.resolveDuplicates()
      }

      PrettyButton {
        text: qsTr("Cancel")
        onClicked: duplicateDialog.close()
      }

      Item {
        Layout.fillWidth: true
      }
    }
  }
}
