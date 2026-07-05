import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QLexis

Dialog {
  id: sortRole
  property list<string> wordSortRoles: appSettings.wordSectionSortRoleKeys()
  property list<string> groupSortRoles: appSettings.groupSectionSortRoleKeys()
  property int selectedWordRoleIndex
  property int selectedGroupRoleIndex

  ButtonGroup {
    id: wordOptions
  }

  ButtonGroup {
    id: groupOptions
  }

  ColumnLayout {
    PrettyLabel {
      title: qsTr("Sort by")
      Layout.alignment: Qt.AlignHCenter
      Layout.bottomMargin: 24
    }

    PrettyLabel {
      title: qsTr("Words")
      Layout.bottomMargin: 8
    }

    Repeater {
      model: [qsTr("Creation time"), qsTr("Frequency"), qsTr("Modification time"), qsTr("Title")]
      RadioButton {
        required property string modelData
        required property int index
        text: modelData
        ButtonGroup.group: wordOptions
        onClicked: {
          selectedWordRoleIndex = index
        }
      }
    }

    PrettyLabel {
      title: qsTr("Subject groups")
      Layout.topMargin: 16
      Layout.bottomMargin: 8
    }

    Repeater {
      model: [qsTr("Creation time"), qsTr("Modification time"), qsTr("Title")]
      RadioButton {
        required property string modelData
        required property int index
        text: modelData
        ButtonGroup.group: groupOptions
        onClicked: {
          selectedGroupRoleIndex = index
        }
      }
    }

    OkCancel {
      Layout.topMargin: 30
      okay: function () {
        settings.wordSortRole = wordSortRoles[selectedWordRoleIndex]
        settings.groupSortRole = groupSortRoles[selectedGroupRoleIndex]
        appSettings.setWordSortRole(settings.wordSortRole)
        appSettings.setGroupSortRole(settings.groupSortRole)
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

  function effectiveWordSortRole() {
    if (settings.wordSortRole !== undefined && settings.wordSortRole.length > 0) {
      return settings.wordSortRole
    }
    if (settings.sortRole !== undefined && settings.sortRole.length > 0) {
      return settings.sortRole
    }
    return "Title"
  }

  function effectiveGroupSortRole() {
    if (settings.groupSortRole !== undefined && settings.groupSortRole.length > 0) {
      return settings.groupSortRole
    }
    if (settings.sortRole === "Frequency") {
      return "Title"
    }
    if (settings.sortRole !== undefined && settings.sortRole.length > 0) {
      return settings.sortRole
    }
    return "Title"
  }

  function init() {
    selectedWordRoleIndex = wordSortRoles.indexOf(effectiveWordSortRole())
    selectedGroupRoleIndex = groupSortRoles.indexOf(effectiveGroupSortRole())
    if (selectedWordRoleIndex !== -1) {
      wordOptions.buttons[selectedWordRoleIndex].checked = true
    }
    if (selectedGroupRoleIndex !== -1) {
      groupOptions.buttons[selectedGroupRoleIndex].checked = true
    }
  }

}
