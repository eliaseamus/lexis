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
    text: "Ok"
    Material.background: Material.accentColor
    onClicked: okay()

    ToolTip.visible: hovered && okTooltipVisible
    ToolTip.text: qsTr(okTooltipText)
  }

  Button {
    text: "Cancel"
    onClicked: cancel()
  }
}
