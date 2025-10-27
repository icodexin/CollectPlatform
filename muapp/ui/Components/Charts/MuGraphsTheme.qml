import QtGraphs
import HuskarUI.Basic

GraphsTheme {
    theme: GraphsTheme.Theme.BlueSeries
    backgroundVisible: false
    gridVisible: false
    plotAreaBackgroundVisible: false

    labelFont {
        family: HusTheme.Primary.fontPrimaryFamily
        pixelSize: HusTheme.Primary.fontPrimarySize
    }
    labelTextColor: HusTheme.Primary.colorTextBase

    axisX.mainWidth: 1
    axisX.mainColor: HusTheme.Primary.colorTextBase
    axisXLabelFont {
        family: HusTheme.Primary.fontPrimaryFamily
        pixelSize: 10
    }

    axisY.mainWidth: 1
    axisY.mainColor: HusTheme.Primary.colorTextBase
    axisYLabelFont {
        family: HusTheme.Primary.fontPrimaryFamily
        pixelSize: 10
    }

    grid.mainWidth: 1
    grid.mainColor: HusTheme.Primary.colorTextSecondary
    grid.subColor: HusTheme.Primary.colorTextTertiary
}