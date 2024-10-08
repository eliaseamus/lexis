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
        id: language
        flat: true
        icon.source: "icons/language.png"
        ToolTip {
          visible: language.hovered
          text: qsTr("Interface language")
        }
        onClicked: languageDialog.open()
      }
      RoundButton {
        id: sortPrinciple
        flat: true
        icon.source: "icons/sort.png"
        ToolTip {
          visible: sortPrinciple.hovered
          text: qsTr("Sort principle")
        }
        onClicked: sortPrincipleDialog.open()
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

  SortPrincipleDialog {
    id: sortPrincipleDialog
    x: (main.width - width) / 2
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
  }

  ColorDialog {
    id: colorDialog
    selectedColor: settings.accentColor
    onAccepted: {
      settings.accentColor = selectedColor
    }
  }
}
