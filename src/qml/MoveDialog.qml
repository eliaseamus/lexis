import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
  id: moveDialog
  property string currentTable
  property alias model: view.model
  // property alias view: view

  ColumnLayout {
    anchors.fill: parent

    TreeView {
      id: view
      Layout.preferredHeight: 300
      Layout.preferredWidth: 700
      selectionModel: ItemSelectionModel {}
      delegate: Item {
        implicitWidth: padding + label.x + label.implicitWidth + padding
        implicitHeight: label.implicitHeight * 1.5

        readonly property real indentation: 20
        readonly property real padding: 5

        // Assigned to by TreeView:
        required property TreeView treeView
        required property bool isTreeNode
        required property bool expanded
        required property int hasChildren
        required property int depth
        required property int row
        required property int column
        required property bool current

        // Rotate indicator when expanded by the user
        // (requires TreeView to have a selectionModel)
        property Animation indicatorAnimation: NumberAnimation {
            target: indicator
            property: "rotation"
            from: expanded ? 0 : 90
            to: expanded ? 90 : 0
            duration: 100
            easing.type: Easing.OutQuart
        }
        TableView.onPooled: indicatorAnimation.complete()
        TableView.onReused: if (current) indicatorAnimation.start()
        onExpandedChanged: indicator.rotation = expanded ? 90 : 0

        Rectangle {
            id: background
            anchors.fill: parent
            color: row === treeView.currentRow ? settings.accentColor :
                   (treeView.alternatingRows && row % 2 !== 0) ? "lightgrey" : "white"
        }

        Label {
            id: indicator
            x: padding + (depth * indentation)
            anchors.verticalCenter: parent.verticalCenter
            visible: isTreeNode && hasChildren
            text: "▶"

            TapHandler {
                onSingleTapped: {
                    let index = treeView.index(row, column)
                    treeView.selectionModel.setCurrentIndex(index, ItemSelectionModel.NoUpdate)
                    treeView.toggleExpanded(row)
                }
            }
        }

        Label {
            id: label
            x: padding + (isTreeNode ? (depth + 1) * indentation : 0)
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width - padding - x
            clip: true
            text: model.display
        }
      }
    }

    OkCancel {
      Layout.topMargin: 10
      okay: function () {
        moveDialog.accept()
      }
      cancel:  function () {
        moveDialog.reject()
      }
    }
  }



  function init() {
    view.model = library.getStructure();
    view.expandRecursively();
  }

}
