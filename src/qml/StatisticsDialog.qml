import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
  id: statisticsDialog
  property bool libraryMode: false
  property int itemId: -1
  property var stats: ({})

  property var sectionNames: [
    qsTr("Word"),
    qsTr("Subject group"),
    qsTr("Book"),
    qsTr("Article"),
    qsTr("Movie"),
    qsTr("Series"),
    qsTr("Album"),
    qsTr("Song")
  ]

  title: libraryMode ? qsTr("Library statistics") : qsTr("Item statistics")
  modal: true
  implicitWidth: libraryMode ? Math.min(main.width - sideBar.width - 80, 560)
                               : Math.min(main.width - sideBar.width - 80, 480)
  padding: 16

  onAboutToShow: loadStats()

  function loadStats() {
    stats = libraryMode ? library.languageStatistics() : library.itemStatistics(itemId)
  }

  function formatDateTime(value) {
    if (value === undefined || value === null) {
      return ""
    }
    return Qt.formatDateTime(value)
  }

  function formatLocation(breadcrumb) {
    if (!breadcrumb || breadcrumb.length === 0) {
      return qsTr("Start page")
    }
    return breadcrumb.split(" \u203a ").join(" \u2192 ")
  }

  function statText(value) {
    if (value === undefined || value === null) {
      return ""
    }
    return String(value)
  }

  function formatTypeName() {
    if (stats.type !== undefined && stats.type >= 0 && stats.type < sectionNames.length) {
      return sectionNames[stats.type]
    }
    return statText(stats.typeName)
  }

  ScrollView {
    anchors.fill: parent
    clip: true

    ColumnLayout {
      width: statisticsDialog.availableWidth
      spacing: 12

      PrettyLabel {
        visible: !libraryMode && stats.title !== undefined
        Layout.fillWidth: true
        title: statText(stats.title)
      }

      InfoLine {
        visible: !libraryMode
        infoTitle: qsTr("Type:")
        infoText: formatTypeName()
      }

      InfoLine {
        visible: !libraryMode
        infoTitle: qsTr("Location:")
        infoText: formatLocation(stats.breadcrumb)
      }

      InfoLine {
        visible: !libraryMode
        infoTitle: qsTr("Creation:")
        infoText: formatDateTime(stats.creationTime)
      }

      InfoLine {
        visible: !libraryMode
        infoTitle: qsTr("Last edit:")
        infoText: formatDateTime(stats.modificationTime)
      }

      InfoLine {
        visible: !libraryMode && stats.directChildren !== undefined && stats.directChildren > 0
        infoTitle: qsTr("Direct children:")
        infoText: statText(stats.directChildren)
      }

      InfoLine {
        visible: !libraryMode && stats.descendants !== undefined && stats.descendants > 0
        infoTitle: qsTr("All descendants:")
        infoText: statText(stats.descendants)
      }

      InfoLine {
        visible: libraryMode
        infoTitle: qsTr("Total items:")
        infoText: statText(stats.totalItems !== undefined ? stats.totalItems : 0)
      }

      InfoLine {
        visible: libraryMode
        infoTitle: qsTr("Words:")
        infoText: statText(stats.wordCount !== undefined ? stats.wordCount : 0)
      }

      InfoLine {
        visible: libraryMode
        infoTitle: qsTr("Subject groups:")
        infoText: statText(stats.subjectGroupCount !== undefined ? stats.subjectGroupCount : 0)
      }

      InfoLine {
        visible: libraryMode
        infoTitle: qsTr("Added this month:")
        infoText: statText(stats.addedThisMonth !== undefined ? stats.addedThisMonth : 0)
      }

      InfoLine {
        visible: libraryMode
        infoTitle: qsTr("Added last 7 days:")
        infoText: statText(stats.addedLast7Days !== undefined ? stats.addedLast7Days : 0)
      }

      InfoLine {
        visible: libraryMode
        infoTitle: qsTr("Added last 30 days:")
        infoText: statText(stats.addedLast30Days !== undefined ? stats.addedLast30Days : 0)
      }

      ColumnLayout {
        visible: stats.wordsByCategory !== undefined && stats.wordsByCategory.length > 0
        Layout.fillWidth: true
        spacing: 8

        PrettyLabel {
          title: qsTr("Words by subject group")
        }

        CategoryPieChart {
          Layout.fillWidth: true
          categories: stats.wordsByCategory !== undefined ? stats.wordsByCategory : []
        }
      }

      PrettyButton {
        text: qsTr("Ok")
        Layout.alignment: Qt.AlignHCenter
        Layout.topMargin: 8
        onClicked: statisticsDialog.accept()
      }
    }
  }
}
