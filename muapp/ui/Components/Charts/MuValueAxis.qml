import QtGraphs

ValueAxis {
    property real minRange: 2

    function rescale(minValue, maxValue, paddingRatio = 0.05) {
        console.assert(minRange > 0, "minRange must be greater than 0");

        if (isNaN(minValue) || isNaN(maxValue))
            return

        if (minValue === maxValue) {
            minValue -= minRange / 2;
            maxValue += minRange / 2;
        } else if (maxValue - minValue < minRange) {
            const mid = (minValue + maxValue) / 2;
            minValue = mid - minRange / 2;
            maxValue = mid + minRange / 2;
        }

        min = minValue - Math.abs(minValue) * paddingRatio
        max = maxValue + Math.abs(maxValue) * paddingRatio
    }
}