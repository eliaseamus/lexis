import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "utils.js" as Utils

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
          width: 150
          height: 150
          color: selectedLanguage === modelData ? settings.accentColor : "lightgrey"
          border.color: selectedLanguage === modelData || mouseArea.containsMouse ?
                        settings.accentColor :
                        "lightgrey"
          border.width: 2
          radius: 10

          ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            Image {
              Layout.fillHeight: true
              Layout.fillWidth: true
              fillMode: Image.PreserveAspectFit
              source: "qrc:/qt/qml/QLexis/icons/flags/%1.png".arg(modelData)
            }
            ToolTip {
              visible: mouseArea.containsMouse
              text: Utils.getFullLanguageName(modelData)
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

    OkCancel {
      Layout.topMargin: 20
      okay: function () {
        if (settings.interfaceLanguage !== selectedLanguage) {
          settings.interfaceLanguage = selectedLanguage
          appManager.changeLanguage(settings.interfaceLanguage)
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
