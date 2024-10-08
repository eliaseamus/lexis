import QtQuick
import QtCore
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

Window {
  id: main
  width: 1200
  height: 800
  visible: true

  RowLayout {
    anchors.fill: parent
    SideBar {
      Layout.fillHeight: true
    }

    StackView {
      Layout.fillHeight: true
      Layout.fillWidth: true
      id: stackView
      initialItem: startPage
    }
  }

  Settings {
    id: settings
    property string language: "en"
    property string sortPrinciple: "modTime"
    property color accentColor: Material.accentColor
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
