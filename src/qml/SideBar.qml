import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Item {
  id: sideBar
  width: 55
  Rectangle {
    anchors.fill: parent
    color: palette.base
    ColumnLayout {
      anchors.fill: parent
      RoundButton {
        id: interfaceLanguage
        flat: true
        icon.source: "icons/language.png"
        ToolTip {
          visible: interfaceLanguage.hovered
          text: qsTr("Interface language")
        }
        onClicked: {
          languageDialog.init()
          languageDialog.open()
        }
      }
      RoundButton {
        id: sortRole
        flat: true
        icon.source: "icons/sort.png"
        ToolTip {
          visible: sortRole.hovered
          text: qsTr("Sort rule")
        }
        onClicked: {
          sortRoleDialog.init()
          sortRoleDialog.open()
        }
      }
      RoundButton {
        id: appColor
        flat: true
        icon.source: "icons/color.png"
        ToolTip {
          visible: appColor.hovered
          text: qsTr("Color")
        }
        onClicked: colorDialog.open()
      }
      Item {
        Layout.fillHeight: true
      }
    }
  }

  InterfaceLanguageDialog {
    id: languageDialog
    x: (main.width - width) / 2
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
  }

  SortRoleDialog {
    id: sortRoleDialog
    x: (main.width - width) / 2
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
    onAccepted: {
      library.sections.forEach((section) => {
        section.sort(settings.sortRole)
      })
    }
  }

  ColorDialog {
    id: colorDialog
    selectedColor: settings.accentColor
    onAccepted: {
      settings.accentColor = selectedColor
      var colorSum = selectedColor.r + selectedColor.g + selectedColor.b
      var threshold = 2 * 0.93
      if (colorSum > threshold) {
        settings.fgColor = "black"
      } else {
        settings.fgColor = "white"
      }
    }
  }

  Shortcut {
    sequence: "Alt+1"
    onActivated: {
      languageDialog.init()
      languageDialog.open()
    }
  }

  Shortcut {
    sequence: "Alt+2"
    onActivated: {
      sortRoleDialog.init()
      sortRoleDialog.open()
    }
  }

  Shortcut {
    sequence: "Alt+3"
    onActivated: colorDialog.open()
  }
}
