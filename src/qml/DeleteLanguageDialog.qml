import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
  id: deleteLanguageDialog
  property string language

  ColumnLayout {
    PrettyLabel {
      title: qsTr("Are you sure you want to\ndelete this language from your library?")
      horizontalAlignment: Qt.AlignHCenter
      Layout.alignment: Qt.AlignHCenter
      Layout.topMargin: 10
      Layout.bottomMargin: 20
    }
    Rectangle {
      Layout.preferredHeight: 200
      Layout.preferredWidth: 300
      Layout.alignment: Qt.AlignCenter
      color: palette.base
      Image {
        anchors.fill: parent
        anchors.centerIn: parent
        anchors.leftMargin: 30
        anchors.rightMargin: 30
        fillMode: Image.PreserveAspectFit
        source: language.length > 0 ? "icons/flags/%1.png".arg(language) : ""
      }
    }
    OkCancel {
      Layout.topMargin: 10
      okay: function () {
        library.deleteLanguage(language);
        var index = settings.languages.indexOf(language);
        if (index !== -1) {
         settings.languages.splice(index, 1);
        }
        if (language === settings.currentLanguage) {
          settings.currentLanguage = ""
          startPage.refresh();
        }
        deleteLanguageDialog.accept();
      }
      cancel: function () {
        deleteLanguageDialog.reject();
      }
    }
  }

}
