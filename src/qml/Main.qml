import QtQuick
import QtCore
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QLexis

Window {
  id: main
  width: 1450
  height: 950
  visible: true
  Material.accent: settings.accentColor

  RowLayout {
    anchors.fill: parent
    SideBar {
      id: sideBar
      z: 1
      Layout.fillHeight: true
    }

    StackView {
      id: stackView
      Layout.fillHeight: true
      Layout.fillWidth: true
      initialItem: libraryView
    }
    Component.onCompleted: appManager.changeLanguage(settings.interfaceLanguage);
  }

  Library {
    id: library
  }

  Settings {
    id: settings
    property string interfaceLanguage: "en"
    property string currentLanguage
    property list<string> languages: []
    property string sortRole: "Title"
    property color accentColor: Material.accentColor
    property color fgColor: "white"
  }

  Shortcut {
    sequences: ["Esc", "Alt+Left"]
    onActivated: popStack()
  }

  Shortcut {
    sequence: StandardKey.Quit
    onActivated: Qt.quit()
  }

  LibraryView {
    id: libraryView
  }

  function popStack() {
    if (stackView.currentItem == libraryView) {
      libraryView.pop();
    } else {
      stackView.pop();
    }
  }
}
