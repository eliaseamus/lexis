import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
  id: buttonBox
  property var okay: function () {}
  property var cancel: function () {}
  property bool okTooltipVisible
  property string okTooltipText

  Layout.alignment: Qt.AlignCenter
  Button {
    id: okButton
    text: qsTr("Ok")
    Material.background: settings.accentColor
    onClicked: okay()

    contentItem: Label {
      text: okButton.text
      color: settings.fgColor
      verticalAlignment: Text.AlignVCenter
    }

    ToolTip.visible: hovered && okTooltipVisible
    ToolTip.text: qsTr(okTooltipText)
  }

  Button {
    id: cancelButton
    text: qsTr("Cancel")
    Material.background: settings.accentColor
    onClicked: cancel()

    contentItem: Label {
      text: cancelButton.text
      color: settings.fgColor
      verticalAlignment: Text.AlignVCenter
    }
  }
}
