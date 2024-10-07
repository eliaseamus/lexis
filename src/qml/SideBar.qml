import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

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
        id: sortOrder
        flat: true
        icon.source: "icons/sort.png"
        ToolTip {
          visible: sortOrder.hovered
          text: qsTr("Sort order")
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
      }
      Item {
        Layout.fillHeight: true
      }
    }
  }

  InterfaceLanguage {
    id: languageDialog
    modal: true
    x: (main.width - width) / 2
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
  }
}
