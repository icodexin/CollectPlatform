/**
 * 根据序列调整时间轴
 * @param {Object} axis - 时间轴对象
 * @param {Object} series - 包含数据点的序列对象
 * @param {number} timeSize - 时间轴跨度, 单位秒
 */
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

/**
 * 根据序列调整值轴
 * @param {Object} axis - 值轴对象
 * @param {Object|Array} series - 包含数据点的序列对象或序列对象列表
 * @param {number} [paddingRatio=0.05] - 相对扩展比例
 * @param {number} [minPadding=1e-6] - 最小扩展值
 * @param {Array} [defaultRange=[0,1]] - 无数据时的默认范围
 */
function rescaleValueAxisBySeries(
    axis, series,
    paddingRatio = 0.05,
    minPadding = 1e-6,
    defaultRange = [0, 1]
) {
    let minValue = Infinity;
    let maxValue = -Infinity;

    const seriesList = Array.isArray(series) ? series : [series];

    for (const s of seriesList) {
        for (let i = 0; i < s.count; i++) {
            minValue = Math.min(minValue, point.y);
            maxValue = Math.max(maxValue, point.y);
        }
    }

    if (minValue === Infinity || maxValue === -Infinity) {
        [minValue, maxValue] = defaultRange;
    } else if (minValue === maxValue) {
        minValue -= 1;
        maxValue += 1;
    }

    const padding = Math.max((maxValue - minValue) * paddingRatio, minPadding);
    axis.min = minValue - padding;
    axis.max = maxValue + padding;
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
        // 删除了当前点后，下一个点作为第一个点，前面没有连线, 会有一段空白
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

function createObject(source, parent, properties) {
    return new Promise((resolve, reject) => {
        let component
        if (typeof source === "string") {
            component = Qt.createComponent(source);
        } else if (source instanceof Component) {
            component = source;
        } else {
            reject("Invalid source: " + source);
            return;
        }

        function doCreation() {
            if (component.status === Component.Ready) {
                const obj = component.createObject(parent, properties);
                if (obj !== null) {
                    resolve(obj);
                } else {
                    reject("Failed to create object from: " + source);
                }
            } else if (component.status === Component.Error) {
                reject("Failed to load component: " + component.errorString());
            }
        }

        if (component.status === Component.Ready || component.status === Component.Error) {
            doCreation();
        } else {
            component.statusChanged.connect(doCreation);
        }
    });
}