import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
  id: buttonBox
  property var okay: function () {}
  property bool okTooltipVisible
  property string okTooltipText

  Layout.alignment: Qt.AlignCenter
  Button {
    id: ok
    text: "Ok"
    Material.background: Material.accentColor
    onClicked: okay()

    ToolTip.visible: hovered && okTooltipVisible
    ToolTip.text: qsTr(okTooltipText)
  }

  Button {
    id: cancel
    text: "Cancel"
    onClicked: stackView.pop()
  }
}
