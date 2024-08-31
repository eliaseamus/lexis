import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
  id: buttonBox
  anchors.bottom: parent.bottom
  anchors.horizontalCenter: parent.horizontalCenter
  Button {
    id: ok
    text: "Ok"
  }
  Button {
    id: cancel
    text: "Cancel"
  }
}
