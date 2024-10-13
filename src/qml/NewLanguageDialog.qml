import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
  id: newLanguage
  property string selectedLanguage
  property var languages

  ColumnLayout {
    GridView {
      id: grid
      Layout.alignment: Qt.AlignCenter
      Layout.topMargin: 20
      Layout.leftMargin: 20
      width: 3 * cellWidth
      height: 3 * cellHeight
      cellWidth: 215
      cellHeight: 215
      model: languages
      interactive: false
      delegate: Rectangle {
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
    OkCancel {
      okay: function () {
        settings.currentLanguage = selectedLanguage
        settings.languages.push(selectedLanguage)
        library.changeLanguage(selectedLanguage)
        newLanguage.accept()
      }
      cancel: function () {
        newLanguage.reject()
      }
    }
  }

  function init() {
    var langs = ["en", "es", "de", "fr", "ru", "it", "pl", "uk", "tr"];
    settings.languages.forEach((lang) => {
      var index = langs.indexOf(lang);
      if (index !== -1) {
       langs.splice(index, 1);
      }
    })
    languages = langs
    selectedLanguage = newLanguage.languages[0]
  }

}
