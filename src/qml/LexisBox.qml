import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ComboBox {
  id: control
  Layout.preferredHeight: 55

  delegate: ItemDelegate {
    width: control.width
    contentItem: Text {
      text: modelData
      color: highlighted ? settings.fgColor : "black"
      font: control.font
      elide: Text.ElideRight
      verticalAlignment: Text.AlignVCenter
    }
    highlighted: control.highlightedIndex === index
  }

  popup: Popup {
    y: control.height - 1
    width: control.width
    implicitHeight: contentItem.implicitHeight
    padding: 1

    contentItem: ListView {
      clip: true
      implicitHeight: contentHeight
      model: control.popup.visible ? control.delegateModel : null
      currentIndex: control.highlightedIndex
      highlight: Component {
        Rectangle {
          color: settings.accentColor
        }
      }

      ScrollIndicator.vertical: ScrollIndicator {}
    }

    background: Rectangle {
      border.color: "grey"
      radius: 5
    }
  }
}
