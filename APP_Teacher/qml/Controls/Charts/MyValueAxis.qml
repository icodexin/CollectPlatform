import QtGraphs

ValueAxis {
    function rescale(minValue, maxValue, minRange = 2) {
        console.assert(minRange > 0, "minRange must be greater than 0");

        if (minValue === maxValue) {
            minValue -= minRange / 2;
            maxValue += minRange / 2;
        }

        min = minValue - Math.abs(minValue) * 0.05
        max = maxValue + Math.abs(maxValue) * 0.05
    }
}