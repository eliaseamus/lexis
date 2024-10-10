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
      title: qsTr("Interface language")
      Layout.alignment: Qt.AlignHCenter
      Layout.bottomMargin: 20
    }

    RowLayout {
      RadioButton {
        id: en
        text: "en"
        Layout.alignment: Qt.AlignVCenter
        ButtonGroup.group: options
        contentItem: Label {}
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
        text: "ru"
        Layout.alignment: Qt.AlignVCenter
        ButtonGroup.group: options
        contentItem: Label {}
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

    OkCancel {
      Layout.topMargin: 30
      okay: function () {
        settings.language = options.checkedButton.text
        appManager.changeLanguage(settings.language)
        interfaceLanguage.accept()
      }
      cancel: function () {
        interfaceLanguage.reject()
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
