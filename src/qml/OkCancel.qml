import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
  id: buttonBox
  property var okay: function () {}
  Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
  Button {
    id: ok
    text: "Ok"
    Material.background: Material.accentColor
    onClicked: okay()
  }

  Button {
    id: cancel
    text: "Cancel"
    onClicked: stackView.pop()
  }
}
