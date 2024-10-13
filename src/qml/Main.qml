import QtQuick
import QtCore
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QLexis

Window {
  id: main
  width: 1300
  height: 900
  visible: true
  Material.accent: settings.accentColor

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
    Component.onCompleted: appManager.changeLanguage(settings.interfaceLanguage)
  }

  Library {
    id: library
  }

  Settings {
    id: settings
    property string interfaceLanguage: "en"
    property string currentLanguage
    property list<string> languages: []
    property string sortRole: "Modification time"
    property color accentColor: Material.accentColor
    property color fgColor: "white"
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
