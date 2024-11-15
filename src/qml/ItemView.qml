import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import QLexis

Pane {
  property int itemID
  property string title
  property string imageUrl
  property color itemColor
  property alias audioUrl: pronunciation.source
  property string meaning

  ColumnLayout {
    anchors.fill: parent
    Item {Layout.fillHeight: true}
    BusyIndicator {
      id: spinner
      visible: true
      Layout.alignment: Qt.AlignCenter
    }
    RowLayout {
      id: body
      visible: false
      Layout.fillWidth: true
      Layout.fillHeight: true
      ColumnLayout {
        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.preferredWidth: main.width / 2 - sideBar.width
        RowLayout {
          Layout.fillWidth: true
          Layout.maximumWidth: parent.width
          Layout.bottomMargin: 20
          Text {
            text: title
            Layout.leftMargin: 20
            font.pointSize: 40
          }
          RoundButton {
            id: speaker
            Layout.alignment: Qt.AlignVCenter
            Layout.leftMargin: 10
            icon.color: settings.fgColor
            Material.background: settings.accentColor
            icon.source: "icons/audio.png"
            ToolTip {
              visible: speaker.hovered
              text: qsTr("Pronunciation")
            }
            onClicked: {
              if (audioUrl.toString().length === 0) {
                audioUrl = library.readAudio(itemID);
              }
              pronunciation.play();
            }
          }
          Item {Layout.fillWidth: true}
          PrettyLabel {
            id: transcription
            visible: false
            Layout.alignment: Qt.AlignVCenter
            Layout.rightMargin: 30
            font.pointSize: 20
          }
        }
        ScrollView {
          Layout.fillWidth: true
          Layout.fillHeight: true
          Layout.maximumWidth: parent.width
          ScrollBar.horizontal: ScrollBar {policy: ScrollBar.AlwaysOff}
          Text {
            id: dictionaryPage
            font.pointSize: 16
            width: parent.width
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
          }
          onWidthChanged: {
            dictionaryPage.width = width
          }
        }
        Item {Layout.fillHeight: true}
        RowLayout {
          Layout.fillWidth: true
          Layout.alignment: Qt.AlignBottom
          RoundButton {
            id: back
            icon.source: "icons/back.png"
            icon.color: settings.fgColor
            Material.background: settings.accentColor
            onClicked: popStack()

            ToolTip {
              visible: back.hovered
              text: qsTr("Back")
            }
          }
          Item {Layout.fillWidth: true}
          RoundButton {
            id: insertMeaning
            icon.source: "icons/plus.png"
            icon.color: settings.fgColor
            Material.background: settings.accentColor
            onClicked: {
              inputDialog.text = dictionaryPage.text;
              inputDialog.open();
            }

            ToolTip {
              visible: insertMeaning.hovered
              text: qsTr("Add meaning")
            }
          }
        }
      }
      Rectangle {
        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignCenter
        Layout.preferredWidth: main.width / 2
        radius: 10
        color: itemColor
        Icon {
          anchors.fill: parent
          anchors.centerIn: parent
          anchors.margins: 50
          image: imageUrl
          iconRadius: {
            var minSide = parent.width < parent.height ? parent.width : parent.height;
            return 3 * minSide / 4;
          }
          iconColor: itemColor
          iconTitle: title
        }
      }
    }
    Item {Layout.fillHeight: true}
  }

  MediaPlayer {
    id: pronunciation
    audioOutput: AudioOutput {}
  }

  Connections {
    target: dictionary

    function onDefinitionsReady(definitions) {
      const hasMultipleDefinitions = definitions.length > 1;
      var dictionaryText = String();
      if (hasMultipleDefinitions) {
        dictionaryText += "<ol>";
      }
      definitions.forEach((definition) => {
        if (definition.transcription.length > 0 && transcription.title.length == 0) {
          transcription.title = definition.transcription;
          transcription.visible = true;
        }
        if (hasMultipleDefinitions) {
          dictionaryText += "<li>";
        }
        dictionaryText += definition.partOfSpeech;
        var translationText = String("<ul>");
        definition.translations.forEach((translation) => {
          translationText += "<li>";
          translationText += translation.text;
          if (translation.synonyms.length > 0) {
            translationText += " (%1)".arg(translation.synonyms.join(", "));
          }
          if (translation.meanings.length > 0) {
            translationText += ": %1".arg(translation.meanings.join(", "));
          }
          translationText += "</li>";
        });
        translationText += "</ul>";
        dictionaryText += translationText;
        if (hasMultipleDefinitions) {
          dictionaryText += "</li><br>";
        }
      });
      if (hasMultipleDefinitions) {
        dictionaryText += "</ol>";
      }
      spinner.visible = false;
      body.visible = true;
      if (dictionaryText.length > 0) {
        dictionaryPage.textFormat = Text.RichText;
        dictionaryPage.text = dictionaryText;
      } else {
        dictionaryPage.textFormat = Text.PlainText;
        insertMeaning.enabled = true;
      }
    }
  }

  Dictionary {
    id: dictionary
  }

  InputDialog {
    id: inputDialog
    x: (main.width - width) / 2 - sideBar.width
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
    onAccepted: {
      dictionaryPage.text = inputDialog.text;
      library.updateMeaning(itemID, dictionaryPage.text);
    }
  }

  function init() {
    spinner.visible = true;
    body.visible = false;
    transcription.title = "";
    transcription.visible = false;
    if (meaning.length > 0) {
      dictionaryPage.textFormat = Text.PlainText;
      dictionaryPage.text = meaning;
      insertMeaning.enabled = true;
      spinner.visible = false;
      body.visible = true;
    } else {
      dictionaryPage.text = "";
      insertMeaning.enabled = false;
      dictionary.get(title);
    }
  }

}
