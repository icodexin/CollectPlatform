import QtGraphs

LineSeries {
    id: series

    /**
     * 移除指定时间点之前的数据点
     * @param time - 时间点, Date对象或毫秒时间戳
     */
    function removePointsBeforeTime(time) {
        const msec = time instanceof Date ? time.getTime() : time;
        let count = 0;

        for (let i = 0; i < series.count - 1; i++) {
            // 如果下一个点在目标时间之后, 那么当前点不应该删除,
            // 删除了当前点后，下一个点作为第一个点，前面没有连线, 会有一段空白
            const point = series.at(i + 1);
            if (point.x < msec) {
                count++;
            } else {
                break;
            }
        }

        if (count > 0)
            series.removeMultiple(0, count);
    }
}