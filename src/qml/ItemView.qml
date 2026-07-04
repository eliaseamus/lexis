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
  property var duplicateWords: []

  signal duplicateSelected(int itemId)

  ColumnLayout {
    anchors.fill: parent

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
            icon.source: "qrc:/qt/qml/QLexis/icons/audio.png"
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
            icon.source: "qrc:/qt/qml/QLexis/icons/back.png"
            icon.color: settings.fgColor
            Material.background: settings.accentColor
            onClicked: popStack()

            ToolTip {
              visible: back.hovered
              text: qsTr("Back")
            }
          }
          Item {Layout.fillWidth: true}
          ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            visible: body.visible && duplicateWords.length > 0
            spacing: 8

            Repeater {
              model: duplicateWords

              Item {
                required property var modelData
                Layout.alignment: Qt.AlignHCenter
                implicitWidth: pathLink.implicitWidth + 16
                implicitHeight: pathLink.implicitHeight + 8

                MouseArea {
                  id: pathLinkArea
                  anchors.fill: parent
                  hoverEnabled: true
                  cursorShape: Qt.PointingHandCursor
                  onClicked: duplicateSelected(modelData.itemId)

                  PrettyLabel {
                    id: pathLink
                    anchors.centerIn: parent
                    dimmed: !pathLinkArea.containsMouse
                    title: formatDuplicatePath(modelData)
                  }
                }
              }
            }
          }
          Item {Layout.fillWidth: true}
          RoundButton {
            id: insertMeaning
            icon.source: "qrc:/qt/qml/QLexis/icons/plus.png"
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

    BusyIndicator {
      id: spinner
      visible: true
      Layout.alignment: Qt.AlignCenter
    }

    NetworkErrorScreen {
      id: networkErrorScreen
      visible: false
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.rightMargin: sideBar.width
    }
  }

  MediaPlayer {
    id: pronunciation
    audioOutput: AudioOutput {}
  }

  Connections {
    target: dictionary

    function onErrorOccured(error) {
      if (meaning.length > 0) {
        showMeaningFallback()
        return
      }
      networkErrorScreen.errorString = error;
      spinner.visible = false;
      networkErrorScreen.visible = true;
    }

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
      networkErrorScreen.visible = false;
      body.visible = true;
      if (dictionaryText.length > 0) {
        dictionaryPage.textFormat = Text.RichText;
        dictionaryPage.text = dictionaryText;
      } else {
        showMeaningFallback();
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

  function formatDuplicatePath(item) {
    if (item.breadcrumb && item.breadcrumb.length > 0) {
      return item.breadcrumb.split(" \u203a ").join(" \u2192 ");
    }
    return qsTr("Start page") + " \u2192 " + item.title;
  }

  function showMeaningFallback() {
    dictionaryPage.textFormat = Text.PlainText;
    if (meaning.length > 0) {
      dictionaryPage.text = meaning;
    } else {
      dictionaryPage.text = "";
    }
    insertMeaning.enabled = true;
    spinner.visible = false;
    networkErrorScreen.visible = false;
    body.visible = true;
  }

  function init() {
    spinner.visible = true;
    networkErrorScreen.visible = false;
    body.visible = false;
    transcription.title = "";
    transcription.visible = false;
    duplicateWords = library.findByTitle(title, itemID);
    dictionaryPage.text = "";
    insertMeaning.enabled = false;
    if (meaning.length > 0) {
      showMeaningFallback();
      return;
    }
    dictionary.get(title);
  }
}
