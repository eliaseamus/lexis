import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtWebView

SplitView {
  id: imagePicker
  orientation: Qt.Horizontal
  property string query
  signal imagePicked(url: string)

  Item {
    SplitView.fillWidth: true
    SplitView.preferredWidth: main.width / 2
    ColumnLayout {
      anchors.fill: parent
      WebView {
        id: searchResults
        Layout.fillHeight: true
        Layout.fillWidth: true
        visible: false
        url: "https://cse.google.com/cse?cx=%1#gsc.tab=1&gsc.q=%2".arg(CSE_ID).arg(query)

        onLoadingChanged: (loadRequest) => {
          if (query.trim().length == 0)
            return;

          switch (loadRequest.status) {
            case WebView.LoadSucceededStatus:
              runJavaScript("
                const ids = [\"cse-header\", \"cse-footer\"];
                const classes = [\"gsc-tabsArea\", \"gsc-above-wrapper-area\"];
                ids.forEach(removeElementByID);
                classes.forEach(removeElementByClassName);

                function removeElementByID(id, index, array) {
                  var element = document.getElementById(id);
                  element.parentNode.removeChild(element);
                }

                function removeElementByClassName(name, index, array) {
                  const elements = document.getElementsByClassName(name);
                  while (elements.length > 0) {
                    elements[0].parentNode.removeChild(elements[0]);
                  }
                }
              ")
              searchResults.visible = true;
              searchBar.visible = true;
              spinner.visible = false;
              break;
          }
        }
      }
      RowLayout {
        id: searchBar
        visible: false
        Layout.fillWidth: true
        TextField {
          id: searchText
          placeholderText: qsTr("Search")
          text: query
          Layout.fillWidth: true
        }
        PrettyButton {
          text: qsTr("Search")
          onClicked: {
            searchResults.url = "https://cse.google.com/cse?cx=%1#gsc.tab=1&gsc.q=%2"
                                .arg(CSE_ID).arg(searchText.text.replace(" ", "+"));
          }
        }
      }
    }

    BusyIndicator {
      id: spinner
      visible: true
      anchors.centerIn: parent
    }

  }

  DropArea {
    id: dropArea
    SplitView.fillWidth: true
    SplitView.preferredWidth: main.width / 2

    Rectangle {
      id: dropBackground
      property color defaultColor: "#efefef"
      anchors.fill: parent
      color: defaultColor
    }

    ColumnLayout {
      anchors.fill: parent

      Image {
        id: image
        visible: false
        fillMode: Image.PreserveAspectFit
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.margins: 50
      }

      Label {
        visible: image.status == Image.Null
        text: qsTr("Pick an image\nand drop it in this area")
        Layout.fillWidth: true
        Layout.fillHeight: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
      }

      OkCancel {
        okay: function () {
          if (image.visible) {
            imagePicked(image.source);
          }
          popStack();
        }
        cancel: function () {
          popStack();
        }
      }
    }

    onEntered: {
      dropBackground.color = Qt.darker(dropBackground.defaultColor, 1.25)
    }

    onExited: {
      dropBackground.color = dropBackground.defaultColor
    }

    onDropped: (drop) => {
      if (!drop.hasUrls) {
        return;
      }
      image.source = drop.urls[0]
      image.visible = true
      dropBackground.color = dropBackground.defaultColor
    }
  }

  function init(source) {
    image.source = source;
    image.visible = source.length > 0;
    searchText.text = query;
    searchBar.visible = false;
    searchResults.visible = false;
    searchResults.reload();
    spinner.visible = true;
  }

}
