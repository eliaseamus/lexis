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
      cellWidth: 165
      cellHeight: 165
      model: languages
      interactive: false
      delegate: Rectangle {
        required property string modelData
        property bool isEnabled: disabledLanguages.indexOf(modelData) === -1
        width: 150
        height: 150
        color: selectedLanguage === modelData ? settings.accentColor : "lightgrey"
        border.color: mouseArea.containsMouse || selectedLanguage === modelData ?
                      settings.accentColor :
                      "lightgrey"
        border.width: 2
        radius: 10

        ColumnLayout {
          anchors.fill: parent
          anchors.margins: 10
          ImageWithEffect {
            Layout.fillHeight: true
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectFit
            source: "qrc:/qt/qml/QLexis/icons/flags/%1.png".arg(modelData)
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
        var index = settings.languages.indexOf(selectedLanguage);
        if (index === -1) {
          settings.languages.push(selectedLanguage)
        }
        libraryView.changeLanguage(selectedLanguage)
        stackView.pop(null)
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
