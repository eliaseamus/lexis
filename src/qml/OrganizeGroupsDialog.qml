import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
  id: organizeDialog
  property var proposals: []

  signal createRequested(string name, var wordIds, color groupColor)

  readonly property var palette: ["#a8e6cf", "#dcedc1", "#ffd3b6", "#ffaaa5",
                                  "#ff8b94", "#1b85b8", "#c3cb71", "#ffd4e5",
                                  "#d4ffea", "#eecbff", "#feffa3", "#dbdcff"]

  modal: true
  padding: 16
  implicitWidth: Math.min(main.width - sideBar.width - 80, 560)

  ColumnLayout {
    anchors.fill: parent
    spacing: 16

    PrettyLabel {
      Layout.fillWidth: true
      title: qsTr("Suggested groups")
      horizontalAlignment: Qt.AlignHCenter
    }

    PrettyLabel {
      Layout.fillWidth: true
      dimmed: true
      title: qsTr("These words look related. Review the group name, uncheck words you want to leave out, and create the groups you like.")
      wrapMode: Text.WordWrap
    }

    ScrollView {
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.preferredHeight: Math.min(contentHeight, main.height * 0.5)
      clip: true

      ColumnLayout {
        width: parent.width
        spacing: 12

        Repeater {
          id: proposalRepeater
          model: organizeDialog.proposals

          Frame {
            id: proposalCard
            required property var modelData
            required property int index
            Layout.fillWidth: true

            ColumnLayout {
              anchors.fill: parent
              spacing: 8

              RowLayout {
                Layout.fillWidth: true
                spacing: 8

                TextField {
                  id: groupName
                  Layout.fillWidth: true
                  text: proposalCard.modelData.name
                  placeholderText: qsTr("Group name")
                }

                PrettyLabel {
                  dimmed: true
                  title: qsTr("%1% match").arg(proposalCard.modelData.confidence)
                }
              }

              Flow {
                Layout.fillWidth: true
                spacing: 4

                Repeater {
                  id: wordRepeater
                  model: proposalCard.modelData.words

                  CheckBox {
                    required property var modelData
                    text: modelData.title
                    checked: true
                  }
                }
              }

              PrettyButton {
                Layout.alignment: Qt.AlignRight
                text: qsTr("Create group")
                enabled: groupName.text.trim().length > 0
                onClicked: {
                  const wordIds = [];
                  for (let i = 0; i < wordRepeater.count; i++) {
                    const box = wordRepeater.itemAt(i);
                    if (box.checked) {
                      wordIds.push(box.modelData.itemId);
                    }
                  }
                  if (wordIds.length > 0) {
                    const groupColor = organizeDialog.palette[
                      Math.floor(Math.random() * organizeDialog.palette.length)];
                    organizeDialog.createRequested(groupName.text.trim(), wordIds, groupColor);
                  }
                }
              }
            }
          }
        }
      }
    }

    PrettyButton {
      Layout.alignment: Qt.AlignHCenter
      text: qsTr("Close")
      onClicked: organizeDialog.close()
    }
  }
}
