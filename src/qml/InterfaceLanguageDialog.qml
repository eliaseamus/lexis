import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
  id: interfaceLanguage
  property string selectedLanguage

  ColumnLayout {
    PrettyLabel {
      title: qsTr("Interface language")
      Layout.alignment: Qt.AlignHCenter
      Layout.bottomMargin: 25
    }

    RowLayout {
      Repeater {
        model: ["en", "ru"]
        Rectangle {
          required property string modelData
          width: 200
          height: 200
          color: selectedLanguage === modelData ? settings.accentColor : palette.base
          border.color: mouseArea.containsMouse ? settings.accentColor : palette.base
          border.width: 2
          radius: 10

          ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            Image {
              Layout.fillHeight: true
              Layout.fillWidth: true
              fillMode: Image.PreserveAspectFit
              source: "icons/flags/%1.png".arg(modelData)
            }
          }
          MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: (mouse) => {
              if (mouse.button === Qt.LeftButton) {
                selectedLanguage = modelData
              }
            }
          }
        }
      }
    }

    CheckBox {
      id: inputLanguage
      text: qsTr("Use as input language")
      ToolTip {
        visible: inputLanguage.hovered
        text: qsTr("Impacts on text autocomplete")
      }
    }

    OkCancel {
      Layout.topMargin: 20
      okay: function () {
        if (settings.interfaceLanguage !== selectedLanguage) {
          settings.interfaceLanguage = selectedLanguage
          appManager.changeLanguage(settings.interfaceLanguage)
        }

        if (inputLanguage.checked) {
          settings.inputLanguage = selectedLanguage
        } else {
          settings.inputLanguage = settings.currentLanguage
        }

        interfaceLanguage.accept()
      }
      cancel: function () {
        interfaceLanguage.reject()
      }
    }
  }

  function init() {
    interfaceLanguage.selectedLanguage = settings.interfaceLanguage
  }

}
