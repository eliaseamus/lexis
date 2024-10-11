import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QLexis

Dialog {
  id: sortRole

  ButtonGroup {
    id: options
  }

  ColumnLayout {
    PrettyLabel {
      title: qsTr("Sort by")
      Layout.alignment: Qt.AlignHCenter
      Layout.bottomMargin: 40
    }

    Repeater {
      model: [qsTr("Modification time"), qsTr("Creation time"), qsTr("Title")]
      RadioButton {
        required property string modelData
        text: modelData
        ButtonGroup.group: options
      }
    }

    OkCancel {
      Layout.topMargin: 30
      okay: function () {
        settings.sortRole = options.checkedButton.text
        sortRole.accept()
      }
      cancel: function () {
        sortRole.reject()
      }
    }
  }

  function init() {
    options.buttons.forEach((button) => {
      if (button.text === settings.sortRole) {
        button.checked = true
      }
    })
  }

  AppSettings {
    id: appSettings
  }
}
