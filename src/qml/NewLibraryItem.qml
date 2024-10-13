import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QLexis

Rectangle {
  id: newItem

  function clear() {
    type.currentIndex = 0
    title.clear()
    author.clear()
    year.text = ""
    bc.checked = false
    image.source = ""
    image.visible = false
    cover.color = "white"
  }

  ColumnLayout {
    id: metaData
    anchors.fill: parent
    anchors.margins: 20

    ComboBox {
      id: type
      Layout.fillWidth: true
      model: startPage.sectionNames
    }

    TextComplete {
      id: title
      Layout.fillWidth: true
      placeholder: qsTr("Title")
    }

    RowLayout {
      id: mediaData
      visible: type.currentIndex !== 0

      TextComplete {
        id: author
        Layout.fillWidth: true
        placeholder: qsTr("Author")
      }

      TextField {
        id: year
        placeholderText: qsTr("Year")
        validator: IntValidator {bottom: 0; top: 9999;}
      }

      CheckBox {
        id: bc
        visible: type.currentText === qsTr("Book")
        text: qsTr("BC")
        ToolTip {
          visible: bc.hovered
          text: qsTr("Before Christ")
        }
      }
    }

    RowLayout {
      Layout.alignment: Qt.AlignHCenter
      Layout.fillWidth: true
      Layout.topMargin: 20
      spacing: 20
      Repeater {
        id: backgroundColors
        model: [palette.base, "#a8e6cf", "#dcedc1", "#ffd3b6", "#ffaaa5",
                "#ff8b94", "#1b85b8", "#c3cb71", "#ffd4e5", "#e4ddd7",
                "#d4ffea", "#eecbff", "#feffa3", "#dbdcff", "#e3dfa4"]
        RoundButton {
          required property string modelData
          Material.background: modelData
          onClicked: cover.color = modelData
        }
      }

      RoundButton {
        id: customColor
        text: "+"
        Material.background: "white"
        onClicked: colorDialog.open()

        ColorDialog {
          id: colorDialog
          onAccepted: {
            customColor.Material.background = selectedColor
            cover.color = selectedColor
          }
        }
      }
    }

    Rectangle {
      id: cover
      color: palette.base
      Layout.fillHeight: true
      Layout.fillWidth: true
      Layout.topMargin: 20
      Layout.bottomMargin: 20

      Image {
        id: image
        visible: false
        fillMode: Image.PreserveAspectFit
        anchors.fill: parent
        anchors.centerIn: parent
        anchors.topMargin: 20
        anchors.bottomMargin: 20

        MouseArea {
          anchors.fill: parent
          cursorShape: Qt.PointingHandCursor
          onClicked: (mouse) => {
            if (mouse.button === Qt.LeftButton) {
              stackView.push(imagePicker)
            }
          }
        }
      }

      Button {
        id: pickImage
        visible: !image.visible
        text: qsTr("Pick an image")
        Material.background: settings.accentColor
        anchors.centerIn: parent

        contentItem: Label {
          text: pickImage.text
          color: settings.fgColor
          verticalAlignment: Text.AlignVCenter
        }

        onClicked: {
          if (imagePicker.hasQuery)
            stackView.push(imagePicker)
        }

        ToolTip {
          visible: pickImage.hovered && !imagePicker.hasQuery
          text: qsTr("Insert title")
        }
      }

    }

    OkCancel {
      okTooltipVisible: title.text.trim().length == 0
      okTooltipText: qsTr("Insert title")
      okay: function () {
        if (!okTooltipVisible) {
          library.addItem(newDbRecord);
          librarySections.model = library.sections
          popStack();
          newItem.clear();
        }
      }
      cancel: function () {
        popStack();
        newItem.clear();
      }
    }
  }

  LibraryItem {
    id: newDbRecord
    title: title.text
    creationTime: new Date(Date.now())
    modificationTime: new Date(Date.now())
    type: sectionTypeManager.librarySectionType(type.currentIndex)
    author: author.text
    year: mediaData.visible ? parseInt(year.text) : -1
    bc: bc.checked
    imageUrl: image.source
    color: cover.color
  }

  ImagePicker {
    id: imagePicker
    query: "%1 %2".arg(title.text).arg(author.text)
    property bool hasQuery: query.trim().length > 0
    visible: false
  }

  Connections {
    target: imagePicker

    function onImagePicked(url) {
      image.source = url
      image.visible = true
    }
  }
}
