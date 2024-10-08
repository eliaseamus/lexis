import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
  id: interfaceLanguage

  ButtonGroup {
    id: options
  }

  ColumnLayout {
    PrettyLabel {
      title: "Interface language"
      Layout.alignment: Qt.AlignHCenter
    }

    RowLayout {
      RadioButton {
        id: en
        Layout.alignment: Qt.AlignVCenter
        ButtonGroup.group: options
        onCheckedChanged: {
          if (checked) {
            settings.language = "en"
          }
        }
      }
      Item {
        Layout.fillWidth: true
      }
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
    }
    RowLayout {
      RadioButton {
        id: ru
        Layout.alignment: Qt.AlignVCenter
        ButtonGroup.group: options
        onCheckedChanged: {
          if (checked) {
            settings.language = "ru"
          }
        }
      }
      Item {
        Layout.fillWidth: true
      }
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
    }

    Component.onCompleted: {
      if (settings.language === "en") {
        en.checked = true
      } else if (settings.language === "ru") {
        ru.checked = true
      }
    }
  }
}
