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
    initialItem: startPage
  }

  Shortcut {
    sequence: "Esc"
    onActivated: {
      if (stackView.currentItem == startPage) {
        startPage.hideSearchLine()
      } else {
        popStack()
      }
    }
  }

  Shortcut {
    sequence: StandardKey.Quit
    onActivated: Qt.quit()
  }

  StartPage {
    id: startPage
  }

  function popStack() {
    stackView.pop()
  }
}
