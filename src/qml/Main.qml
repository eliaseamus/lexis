import QtQuick
import QtQuick.Window
import QtQuick.Controls

Window {
  id: main
  width: 1200
  height: 800
  visible: true

  StackView {
    id: stackView
    anchors.fill: parent
    initialItem: StartPage {}
  }
}
