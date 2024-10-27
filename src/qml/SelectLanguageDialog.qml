import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import "utils.js" as Utils

Dialog {
  id: selectLanguage
  property string selectedLanguage
  property list<string> languages: ["en", "es", "de", "fr", "ru", "it", "pl", "uk", "tr"]
  property list<string> disabledLanguages

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
        property bool isEnabled: disabledLanguages.indexOf(modelData) === -1
        width: 200
        height: 200
        color: selectedLanguage === modelData ? settings.accentColor : palette.base
        border.color: mouseArea.containsMouse ? settings.accentColor : palette.base
        border.width: 2
        radius: 10

        ColumnLayout {
          anchors.fill: parent
          anchors.margins: 10
          ImageWithEffect {
            Layout.fillHeight: true
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectFit
            source: "icons/flags/%1.png".arg(modelData)
            isSaturated: isEnabled === false
            ToolTip {
              visible: mouseArea.containsMouse
              text: Utils.getFullLanguageName(modelData)
            }
          }
        }
        MouseArea {
          id: mouseArea
          enabled: isEnabled
          anchors.fill: parent
          hoverEnabled: true
          cursorShape: isEnabled ? Qt.PointingHandCursor : Qt.ArrowCursor
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
        library.openTable(selectedLanguage)
        var index = settings.languages.indexOf(selectedLanguage);
        if (index === -1) {
          settings.languages.push(selectedLanguage)
        }
        stackView.pop(null)
        libraryView.refresh()
        selectLanguage.accept()
      }
      cancel: function () {
        selectLanguage.reject()
      }
    }
  }

  function init(disableAddedLanguages) {
    if (disableAddedLanguages === true) {
      disabledLanguages = settings.languages;
      for (let i = 0; i < languages.length; i++) {
        if (disabledLanguages.indexOf(languages[i]) === -1) {
          selectedLanguage = languages[i];
          break;
        }
      }
    } else {
      disabledLanguages = [];
      selectedLanguage = languages[0];
    }
  }

}
