import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Button {
  id: button
  Material.background: settings.accentColor

  contentItem: Label {
    text: button.text
    color: settings.fgColor
    verticalAlignment: Text.AlignVCenter
  }
}
