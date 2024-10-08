import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
  id: interfaceLanguage

  ButtonGroup {
    id: options
  }

  ColumnLayout {
    RowLayout {
      Image {
        Layout.preferredWidth: 220
        Layout.preferredHeight: 200
        source: "icons/uk.png"
        fillMode: Image.PreserveAspectFit
        Rectangle {
          color: "#00000000"
          anchors.centerIn: parent
          width: 195
          height: 100
          border.color: palette.base
        }
      }
      Item {
        Layout.fillWidth: true
      }
      RadioButton {
        Layout.alignment: Qt.AlignHCenter
        ButtonGroup.group: options
      }
    }
    RowLayout {
      Image {
        Layout.preferredWidth: 220
        Layout.maximumHeight: 200
        source: "icons/rus.png"
        fillMode: Image.PreserveAspectFit
        Rectangle {
          color: "#00000000"
          anchors.centerIn: parent
          width: 195
          height: 131
          border.color: "lightGrey"
        }
      }
      Item {
        Layout.fillWidth: true
      }
      RadioButton {
        Layout.alignment: Qt.AlignHCenter
        ButtonGroup.group: options
      }
    }
  }
}
