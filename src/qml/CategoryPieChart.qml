import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCharts

RowLayout {
  id: root
  property var categories: []
  property var chartSlices: []
  property real legendTotal: 0

  readonly property int topCategoryLimit: 6
  readonly property int chartSize: 200
  readonly property int legendSpacing: 16
  readonly property color othersColor: "#9e9e9e"

  Layout.fillWidth: true
  Layout.preferredHeight: Math.max(chartSize, legendLayout.implicitHeight)
  Layout.minimumHeight: chartSize
  spacing: legendSpacing

  function totalCount(entries) {
    let total = 0
    for (let i = 0; i < entries.length; i++) {
      total += entries[i].count
    }
    return total
  }

  function categoryLabel(entry) {
    if (entry.others) {
      return qsTr("Others")
    }
    if (entry.standalone) {
      return qsTr("Standalone")
    }
    return entry.categoryName
  }

  function sliceColor(index, entry) {
    if (entry.others) {
      return othersColor
    }
    if (entry.standalone) {
      return Qt.darker(settings.accentColor, 1.8)
    }
    const hue = (index * 67) % 360
    return Qt.hsla(hue / 360, 0.45, 0.55, 1)
  }

  function formatPercentage(count, total) {
    if (total === 0) {
      return "0.0%"
    }
    return (count * 100 / total).toFixed(1) + "%"
  }

  function sliceLegendLabel(entry, total) {
    return categoryLabel(entry) + " — " + entry.count + " (" + formatPercentage(entry.count, total) + ")"
  }

  function buildChartSlices(source) {
    const total = totalCount(source)
    if (total === 0) {
      return []
    }

    const sorted = source.slice().sort((left, right) => {
      if (left.count !== right.count) {
        return right.count - left.count
      }
      return categoryLabel(left).localeCompare(categoryLabel(right))
    })

    const slices = []
    let othersCount = 0

    for (let i = 0; i < sorted.length; i++) {
      if (i < topCategoryLimit) {
        slices.push({
          "categoryName": sorted[i].categoryName,
          "count": sorted[i].count,
          "standalone": sorted[i].standalone === true,
          "others": false
        })
      } else {
        othersCount += sorted[i].count
      }
    }

    if (othersCount > 0) {
      slices.push({
        "categoryName": qsTr("Others"),
        "count": othersCount,
        "standalone": false,
        "others": true
      })
    }

    return slices
  }

  function rebuildChart() {
    chartSlices = buildChartSlices(categories)
    const total = totalCount(categories)

    pieSeries.clear()
    for (let i = 0; i < chartSlices.length; i++) {
      const entry = chartSlices[i]
      const slice = pieSeries.append(categoryLabel(entry), entry.count)
      slice.color = sliceColor(i, entry)
      slice.labelVisible = false
    }
    legendTotal = total
  }

  ChartView {
    id: chartView
    Layout.preferredWidth: root.chartSize
    Layout.preferredHeight: root.chartSize
    Layout.alignment: Qt.AlignVCenter
    antialiasing: true
    backgroundRoundness: 0
    backgroundColor: "transparent"
    margins.top: 0
    margins.bottom: 0
    margins.left: 0
    margins.right: 0
    legend.visible: false

    PieSeries {
      id: pieSeries
    }
  }

  ColumnLayout {
    id: legendLayout
    Layout.fillWidth: true
    Layout.alignment: Qt.AlignVCenter
    spacing: 8

    Repeater {
      model: root.chartSlices
      RowLayout {
        required property var modelData
        required property int index
        Layout.fillWidth: true
        spacing: 8

        Label {
          Layout.fillWidth: true
          horizontalAlignment: Text.AlignRight
          wrapMode: Text.WordWrap
          text: root.sliceLegendLabel(modelData, root.legendTotal)
        }

        Rectangle {
          Layout.preferredWidth: 12
          Layout.preferredHeight: 12
          Layout.alignment: Qt.AlignTop
          Layout.topMargin: 3
          radius: 2
          color: root.sliceColor(index, modelData)
        }
      }
    }
  }

  onCategoriesChanged: rebuildChart()
  Component.onCompleted: rebuildChart()
}
