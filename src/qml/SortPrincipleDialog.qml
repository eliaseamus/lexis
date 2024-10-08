import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
  id: sortPrinciple

  ButtonGroup {
    id: options
  }

  ColumnLayout {
    PrettyLabel {
      title: "Sort by"
      Layout.alignment: Qt.AlignHCenter
    }

    RadioButton {
      id: modTime
      text: qsTr("Modification time")
      ButtonGroup.group: options
      onCheckedChanged: {
        if (checked) {
          settings.sortPrinciple = "modTime"
        }
      }
    }

    RadioButton {
      id: crTime
      text: qsTr("Creation time")
      ButtonGroup.group: options
      onCheckedChanged: {
        if (checked) {
          settings.sortPrinciple = "crTime"
        }
      }
    }

    RadioButton {
      id: itemTitle
      text: qsTr("Title")
      ButtonGroup.group: options
      onCheckedChanged: {
        if (checked) {
          settings.sortPrinciple = "title"
        }
      }
    }

    Component.onCompleted: {
      if (settings.sortPrinciple === "modTime") {
        modTime.checked = true
      } else if (settings.sortPrinciple === "crTime") {
        crTime.checked = true
      } else if (settings.sortPrinciple === "title") {
        itemTitle.checked = true
      }
    }

  }

}
