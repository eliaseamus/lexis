import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QLexis

Dialog {
  id: sortRole
  property list<string> sortRoles: appSettings.sectionSortRoleKeys()
  property int selectedRoleIndex

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
      model: [qsTr("Creation time"), qsTr("Modification time"), qsTr("Title")]
      RadioButton {
        required property string modelData
        required property int index
        text: modelData
        ButtonGroup.group: options
        onClicked: {
          selectedRoleIndex = index
        }
      }
    }

    OkCancel {
      Layout.topMargin: 30
      okay: function () {
        settings.sortRole = sortRoles[selectedRoleIndex]
        sortRole.accept()
      }
      cancel: function () {
        sortRole.reject()
      }
    }
  }

  AppSettings {
    id: appSettings
  }

  function init() {
    selectedRoleIndex = sortRoles.indexOf(settings.sortRole)
    if (selectedRoleIndex != -1) {
      options.buttons[selectedRoleIndex].checked = true
    }
  }

}
