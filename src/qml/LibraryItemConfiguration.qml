import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QLexis

Rectangle {
  id: newItem
  property list<string> displayedTypes // translated type names
  property list<string> types
  property int typesNum
  property bool editMode
  property int itemID
  property int currentType
  property string title
  property string image
  property color backgroundColor: palette.base

  function init() {
    editMode = true
    type.currentIndex = currentType
    titleItem.setText(title)
    imageItem.source = image
    imageItem.visible = image.length > 0
    cover.color = backgroundColor
  }

  function clear() {
    editMode = false
    type.currentIndex = 0
    titleItem.clear()
    imageItem.source = ""
    imageItem.visible = false
    cover.color = palette.base
  }

  ColumnLayout {
    id: metaData
    anchors.fill: parent
    anchors.margins: 20

    ComboBox {
      id: type
      currentIndex: currentType
      Layout.fillWidth: true
      model: displayedTypes.slice(0, typesNum)
    }

    TextComplete {
      id: titleItem
      text: title
      Layout.fillWidth: true
      placeholder: qsTr("Title")
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
      color: backgroundColor
      Layout.fillHeight: true
      Layout.fillWidth: true
      Layout.topMargin: 20
      Layout.bottomMargin: 20

      Image {
        id: imageItem
        visible: source.toString().length > 0
        source: image
        fillMode: Image.PreserveAspectFit
        anchors.fill: parent
        anchors.centerIn: parent
        anchors.margins: 20

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
        visible: !imageItem.visible
        text: qsTr("Pick an image")
        Material.background: settings.accentColor
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: -sideBar.width / 2

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
      Layout.rightMargin: sideBar.width
      okTooltipVisible: !imagePicker.hasQuery
      okTooltipText: qsTr("Insert title")
      okay: function () {
        if (imagePicker.hasQuery) {
          if (editMode) {
            library.updateItem(newDbRecord, currentType);
          } else {
            library.addItem(newDbRecord);
          }
          libraryView.refresh()
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
    itemID: itemID
    title: titleItem.text
    modificationTime: new Date(Date.now())
    type: sectionTypeManager.librarySectionType(types[type.currentIndex])
    imageUrl: imageItem.source
    color: cover.color
  }

  ImagePicker {
    id: imagePicker
    query: titleItem.text
    property bool hasQuery: query.trim().length > 0
    visible: false
  }

  Connections {
    target: imagePicker

    function onImagePicked(url) {
      imageItem.source = url
      imageItem.visible = true
    }
  }
}
