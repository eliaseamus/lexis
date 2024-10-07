import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
  id: interfaceLanguage

  ButtonGroup {
    id: options
  }

  RowLayout {
    ColumnLayout {
      Layout.alignment: Qt.AlignVCenter
      Image {
        Layout.maximumWidth: 300
        Layout.maximumHeight: 300
        source: "icons/uk.png"
        fillMode: Image.PreserveAspectFit
      }
      RadioButton {
        Layout.alignment: Qt.AlignHCenter
        ButtonGroup.group: options
      }
    }
    ColumnLayout {
      Layout.alignment: Qt.AlignVCenter
      Image {
        Layout.maximumWidth: 300
        Layout.maximumHeight: 300
        source: "icons/rus.png"
        fillMode: Image.PreserveAspectFit
      }
      RadioButton {
        Layout.alignment: Qt.AlignHCenter
        ButtonGroup.group: options
      }
    }
  }
}
