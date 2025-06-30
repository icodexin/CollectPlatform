import QtGraphs
import DelegateUI

GraphsTheme {
    theme: GraphsTheme.Theme.BlueSeries
    backgroundVisible: false
    gridVisible: false
    plotAreaBackgroundVisible: false

    labelFont {
        family: DelTheme.Primary.fontPrimaryFamily
        pixelSize: DelTheme.Primary.fontPrimarySize
    }
    labelTextColor: DelTheme.Primary.colorTextBase

    axisX.mainWidth: 1
    axisX.mainColor: DelTheme.Primary.colorTextBase
    axisXLabelFont {
        family: DelTheme.Primary.fontPrimaryFamily
        pixelSize: 10
    }

    axisY.mainWidth: 1
    axisY.mainColor: DelTheme.Primary.colorTextBase
    axisYLabelFont {
        family: DelTheme.Primary.fontPrimaryFamily
        pixelSize: 10
    }

    grid.mainWidth: 1
    grid.mainColor: DelTheme.Primary.colorTextSecondary
    grid.subColor: DelTheme.Primary.colorTextTertiary
}