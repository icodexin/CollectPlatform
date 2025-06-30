function rescaleTimeAxisBySeries(axis, series, timeSize) {
    const minTime = series.at(0).x;
    const maxTime = series.at(series.count - 1).x;

    let start, end;

    if (maxTime - minTime >= timeSize) {
        start = maxTime - timeSize;
        end = maxTime;
    } else {
        start = minTime;
        end = minTime + timeSize;
    }

    axis.min = new Date(start);
    axis.max = new Date(end);
}

function rescaleValueAxisBySeries(axis, series) {
    const startPoint = series.at(0);
    let minValue = startPoint.y;
    let maxValue = startPoint.y;

    for (let i = 1; i < series.count; i++) {
        const point = series.at(i);
        minValue = Math.min(minValue, point.y);
        maxValue = Math.max(maxValue, point.y);
    }

    if (minValue === maxValue) {
        minValue -= 1;
        maxValue += 1;
    }

    axis.min = minValue - Math.abs(minValue) * 0.05;
    axis.max = maxValue + Math.abs(maxValue) * 0.05;
}

function rescaleValueAxisBySeriesList(axis, seriesList) {
    let minValue = Infinity
    let maxValue = -Infinity;

    for (let series of seriesList) {
        for (let i = 0; i < series.count; i++) {
            const point = series.at(i);
            minValue = Math.min(minValue, point.y);
            maxValue = Math.max(maxValue, point.y);
        }
    }

    if (minValue === maxValue) {
        minValue -= 1;
        maxValue += 1;
    }

    axis.min = minValue - Math.abs(minValue) * 0.05;
    axis.max = maxValue + Math.abs(maxValue) * 0.05;
}

/**
 * 删除指定时间之前的点
 * @param {Object} series - 包含数据点的序列对象
 * @param {number} time - 阈值时间; x值小于此时间的点将被删除。
 * @param {Function} callback - 回调函数, 用于处理被删除点的y值
 */
function removePointsBeforeTime(series, time, callback) {
    let count = 0;

    for (let i = 0; i < series.count - 1; i++) { // 最后一个点一定不会被删除
        // 如果下一个点在目标时间之后, 那么当前点不应该删除,
        // 否则下一个点是目标时间后的第一个点时, 删除了当前点,
        // 导致下一个点前面没有连线了，会有一段空白
        const point = series.at(i + 1);
        if (point.x < time) {
            callback(point.y)
            count++;
        } else {
            break;
        }
    }

    if (count > 0) {
        series.removeMultiple(0, count);
    }
}