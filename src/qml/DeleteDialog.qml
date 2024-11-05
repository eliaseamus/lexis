import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
  id: deleteDialog
  property var targets: []
  property color backgroundColor: palette.base

  ColumnLayout {
    PrettyLabel {
      title: qsTr("Are you sure you want to<br>" +
                  "delete <b>%1</b> from your library?")
                  .arg(targets.map((item) => item["title"]).join(", "))
      format: Text.RichText
      horizontalAlignment: Qt.AlignHCenter
      Layout.alignment: Qt.AlignHCenter
      Layout.topMargin: 10
      Layout.bottomMargin: 20
      Layout.preferredWidth: 300
      wrapMode: Text.WordWrap
    }
    Rectangle {
      Layout.preferredHeight: 200
      Layout.preferredWidth: 300
      Layout.alignment: Qt.AlignCenter
      color: backgroundColor
      radius: 10
      RowLayout {
        anchors.fill: parent
        spacing: -285
        Repeater {
          model: targets
          Layout.alignment: Qt.AlignCenter
          Icon {
            required property int index
            width: 200
            height: 200
            image: targets[index]["imageUrl"]
            Layout.alignment: Qt.AlignCenter
            iconRadius: 170
            iconColor: backgroundColor
            iconTitle: targets[index]["title"]
          }
        }
      }
    }
    OkCancel {
      Layout.topMargin: 10
      okay: function () {
        deleteDialog.accept()
      }
      cancel:  function () {
        deleteDialog.reject()
      }
    }
  }

}
