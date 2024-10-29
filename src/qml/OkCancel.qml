import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
  id: buttonBox
  property var okay
  property var cancel
  property bool okTooltipVisible
  property string okTooltipText
  Layout.alignment: Qt.AlignCenter

  PrettyButton {
    id: okButton
    text: qsTr("Ok")
    onClicked: okay()

    ToolTip.visible: hovered && okTooltipVisible
    ToolTip.text: qsTr(okTooltipText)
  }

  PrettyButton {
    id: cancelButton
    text: qsTr("Cancel")
    onClicked: cancel()
  }
}
