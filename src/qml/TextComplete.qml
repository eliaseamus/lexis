import QtQuick
import QtQuick.Controls

Item {
  id: textComplete
  property string placeholder: ""
  anchors.fill: parent

  TextField {
    id: textField
    placeholderText: placeholder

    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right

    onTextEdited: {
      predictor.request(text)
    }

    Rectangle {
      id: completionsBox
      visible: false
      radius: 5

      anchors.top: parent.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      height: childrenRect.height + 8

      border.width: 2
      border.color: Material.accentColor
      Column {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 4
        spacing: 2
        Repeater {
          id: completions
          property int currentIndex: -1

          Label {
            id: completion
            required property string modelData
            required property int index
            text: modelData
            anchors.left: parent.left
            anchors.right: parent.right
            background: Rectangle {
              anchors.fill: parent
              anchors.margins: -2
              color: completion.index == completions.currentIndex ?
                     Material.accentColor :
                     Material.backgroundColor
            }
          }
        }
      }
    }

    Connections {
      target: predictor

      function onPredictionsReceived(predictions) {
        completions.model = predictions
        completionsBox.visible = completions.model.length > 0 ? true : false
      }
    }

    Keys.onPressed: function onPressed(event)  {
      if (completionsBox.visible) {
        switch (event.key) {
          case Qt.Key_Escape:
            completionsBox.visible = false
            completions.currentIndex = -1
            event.accepted = true
            break
          case Qt.Key_Up:
            completions.currentIndex--

            if (completions.currentIndex < 0)
              completions.currentIndex = completions.model.length - 1

            textField.text = completions.model[completions.currentIndex]
            event.accepted = true
            break
          case Qt.Key_Down:
            completions.currentIndex++

            if (completions.currentIndex > completions.model.length - 1)
                completions.currentIndex = 0

            textField.text = completions.model[completions.currentIndex]
            event.accepted = true
            break
          case Qt.Key_Return:
            textField.text = completions.model[completions.currentIndex]
            textField.accepted()
            event.accepted = true;
            completionsBox.visible = false
            break
        }
      } else {
        switch (event.key) {
          case Qt.Key_Down:
            completionsBox.visible = completions.model.length > 0 ? true : false
            event.accepted = true
            break
        }
      }
    }
  }

}
