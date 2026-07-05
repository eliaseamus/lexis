import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QLexis

Dialog {
  id: suggestionDialog
  property var suggestions: []
  property string wordTitle: ""
  property int currentParentId: 0
  property int selectedGroupId: -1
  property bool moveOnly: false

  modal: true
  padding: 16
  implicitWidth: Math.min(main.width - sideBar.width - 80, 520)

  function formatPath(breadcrumb, fallbackTitle) {
    if (breadcrumb && breadcrumb.length > 0) {
      return breadcrumb.split(" \u203a ").join(" \u2192 ")
    }
    if (fallbackTitle && fallbackTitle.length > 0) {
      return fallbackTitle
    }
    return qsTr("Start page")
  }

  function init() {
    if (suggestions.length > 0) {
      selectedGroupId = suggestions[0].groupId
      optionGroup.buttons[0].checked = true
      return
    }
    selectedGroupId = currentParentId
  }

  ColumnLayout {
    anchors.fill: parent
    spacing: 16

    PrettyLabel {
      Layout.fillWidth: true
      title: qsTr("Suggested group for \"%1\"").arg(suggestionDialog.wordTitle)
      horizontalAlignment: Qt.AlignHCenter
      wrapMode: Text.WordWrap
    }

    PrettyLabel {
      Layout.fillWidth: true
      dimmed: true
      title: suggestionDialog.moveOnly
             ? qsTr("Choose where to move this word:")
             : qsTr("Choose where to save this word:")
      wrapMode: Text.WordWrap
    }

    ButtonGroup {
      id: optionGroup
    }

    Repeater {
      model: suggestionDialog.suggestions
      RadioButton {
        required property var modelData
        required property int index
        Layout.fillWidth: true
        text: formatPath(modelData.breadcrumb, modelData.groupName)
               + " (" + qsTr("%1% match").arg(modelData.confidence) + ")"
        ButtonGroup.group: optionGroup
        onClicked: suggestionDialog.selectedGroupId = modelData.groupId
      }
    }

    RadioButton {
      id: keepCurrentOption
      Layout.fillWidth: true
      text: qsTr("Keep at current location (%1)")
            .arg(formatPath(library.parentBreadcrumb(suggestionDialog.currentParentId),
                            suggestionDialog.currentParentId === 0 ? qsTr("Start page") : ""))
      ButtonGroup.group: optionGroup
      onClicked: suggestionDialog.selectedGroupId = suggestionDialog.currentParentId
    }

    OkCancel {
      Layout.alignment: Qt.AlignHCenter
      okay: function () {
        suggestionDialog.accept()
      }
      cancel: function () {
        suggestionDialog.reject()
      }
    }
  }
}
