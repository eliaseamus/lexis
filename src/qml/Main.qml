import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QLexis

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

  Library {
    id: library
  }

  SectionTypeManager {
    id: sectionTypeManager
  }
}
