import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "utils.js" as Utils

Dialog {
  id: quizSettingsDialog
  modal: true
  padding: 16
  implicitWidth: Math.min(main.width - sideBar.width - 80, 420)

  readonly property var tierOptions: [
    { key: "core", label: Utils.frequencyTierLabel("core") },
    { key: "common", label: Utils.frequencyTierLabel("common") },
    { key: "intermediate", label: Utils.frequencyTierLabel("intermediate") },
    { key: "advanced", label: Utils.frequencyTierLabel("advanced") },
    { key: "rare", label: Utils.frequencyTierLabel("rare") },
    { key: "unranked", label: qsTr("Unranked") }
  ]

  ColumnLayout {
    anchors.fill: parent
    spacing: 12

    PrettyLabel {
      Layout.fillWidth: true
      title: qsTr("Quiz settings")
      horizontalAlignment: Qt.AlignHCenter
    }

    CheckBox {
      id: skipKnownBox
      Layout.fillWidth: true
      text: qsTr("Skip known words")
    }

    PrettyLabel {
      Layout.fillWidth: true
      Layout.topMargin: 8
      title: qsTr("Frequency tiers")
    }

    PrettyLabel {
      Layout.fillWidth: true
      dimmed: true
      title: qsTr("Leave all unchecked to include every tier.")
      wrapMode: Text.WordWrap
    }

    Repeater {
      id: tierRepeater
      model: quizSettingsDialog.tierOptions

      CheckBox {
        required property var modelData
        Layout.fillWidth: true
        text: modelData.label
      }
    }

    OkCancel {
      Layout.alignment: Qt.AlignHCenter
      Layout.topMargin: 12
      okay: function () {
        const tiers = []
        for (let i = 0; i < tierRepeater.count; i++) {
          const box = tierRepeater.itemAt(i)
          if (box.checked) {
            tiers.push(box.modelData.key)
          }
        }
        settings.quizSkipKnown = skipKnownBox.checked
        settings.quizFrequencyTiers = tiers
        quizSettingsDialog.accept()
      }
      cancel: function () {
        quizSettingsDialog.reject()
      }
    }
  }

  function init() {
    skipKnownBox.checked = settings.quizSkipKnown
    const selected = settings.quizFrequencyTiers
    for (let i = 0; i < tierRepeater.count; i++) {
      const box = tierRepeater.itemAt(i)
      box.checked = selected.indexOf(box.modelData.key) !== -1
    }
  }
}
