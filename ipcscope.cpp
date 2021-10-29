#include "ipcscope.h"

QT_CHARTS_USE_NAMESPACE

IPCScope::IPCScope(QWidget *parent, ScopeType scopeType) :
    QGraphicsView(new QGraphicsScene, parent),
    mScopeName(""),
    mOpenGLEnabled(false),
    mActiveGraphIdx(-1),
    mActiveMarkerIdx(-1),
    mMarkerTableVisible(true),
    mZoomDirection(zdBothDirections),
    mZoomWeight(0.9),
    mZoomRangeX(0.1,1),
    mZoomRangeY(0.1,1),
    mLegendVisible(true)
{
    this->setRenderHint(QPainter::NonCosmeticDefaultPen);
    mBaseFont = QFont("Times new roman", 14, 1, false);
    mMarkerFont = QFont("Arial", 12, 1, false);
    mMarkerFont.setBold(true);
    mGraphColors << "blue" << "magenta" << "cyan" << "green" << "yellow" << "grey" << "black";

    /* Create a chart */
    mChart = new QChart();

    /* Create axes according to the scope type */
    mScopeType = scopeType;
    if(scopeType == stpLinear){
        QValueAxis *axisX = new QValueAxis;
        QValueAxis *axisY = new QValueAxis;
        mAxesList.append(axisX);
        mAxesList.append(axisY);
        axisX->setTickInterval(10);
        axisX->setTickType(QValueAxis::TicksDynamic);
        axisX->setMinorTickCount(4);
        axisY->setTickInterval(10);
        axisY->setTickType(QValueAxis::TicksDynamic);
        axisY->setMinorTickCount(4);
    } else if (scopeType == stpSemiLogX){
        QLogValueAxis *axisX = new QLogValueAxis;
        axisX->setLabelFormat("%.0e");
        axisX->setBase(10.0);
        axisX->setMinorTickCount(-1);
        QValueAxis *axisY = new QValueAxis;
        axisY->setTickInterval(10);
        axisY->setTickType(QValueAxis::TicksDynamic);
        axisY->setMinorTickCount(4);
        mAxesList.append(axisX);
        mAxesList.append(axisY);
    } else if (scopeType == stpSemiLogY){
        QValueAxis *axisX = new QValueAxis;
        QLogValueAxis *axisY = new QLogValueAxis;
        mAxesList.append(axisX);
        mAxesList.append(axisY);
        axisY->setLabelFormat("%.0e");
        axisY->setBase(10.0);
        axisX->setTickInterval(10);
        axisX->setTickType(QValueAxis::TicksDynamic);
        axisX->setMinorTickCount(4);
    } else{
        QLogValueAxis *axisX = new QLogValueAxis;
        QLogValueAxis *axisY = new QLogValueAxis;
        mAxesList.append(axisX);
        mAxesList.append(axisY);
        axisX->setLabelFormat("%.0e");
        axisX->setBase(10.0);
        axisX->setMinorTickCount(-1);
        axisY->setLabelFormat("%.0e");
        axisY->setBase(10.0);
        axisY->setMinorTickCount(-1);
    }
    foreach(QAbstractAxis *axis, mAxesList){
        axis->setLabelsFont(mBaseFont);
    }

    /* Attach the axes to the chart */
    mChart->addAxis(mAxesList.at(0), Qt::AlignBottom);
    mChart->addAxis(mAxesList.at(1), Qt::AlignLeft);

    /* Add chart into the scene */
    scene()->addItem(mChart);

    /* Legend preparation */
    //mChart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);
    mChart->legend()->setAlignment(Qt::AlignLeft);
    mChart->legend()->detachFromChart();
    mChart->legend()->setBackgroundVisible(true);
    mChart->legend()->setFont(mBaseFont);
    QGraphicsLayout *layout = mChart->legend()->layout();
    layout->setMinimumHeight(-1);
    layout->setPreferredSize(-1,-1);
    mLegendPos = lpTopRight;
    updateLegendPosition();

    /* Create a rubber band to show the zoom area */
    mRubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    mRubberBandOrigin.setX(0);
    mRubberBandOrigin.setY(0);
    mRubberBand->setGeometry(QRect(mRubberBandOrigin, QSize()));
    mRubberBand->hide();
    QPalette palette;
    QColor color(0,202,0);
    color.setAlphaF(0.4);
    palette.setColor(QPalette::Active, QPalette::Highlight, color);
    mRubberBand->setPalette(palette);

    /* Create a marker table widget */
    mMarkerTable = new IPCMarkerTable(this);
    mMarkerTablePos = mpTopMidle;
    mMarkerTable->setFont(mMarkerFont);
    updateMarkerTablePosition();
    /* Default theme is dark */
    mScopeTheme = stDark;
    setScopeThemeDark();
}

IPCScope::~IPCScope()
{
    delete mChart;

    foreach(QAbstractAxis *axis, mAxesList){
        delete axis;
    }
    foreach(QAbstractSeries *series, mGraphsList){
        delete series;
    }
}

/*!
 * \brief IPCScope::updateLegendPosition. Update the legend position in the scope.
 */
void IPCScope::updateLegendPosition()
{
    /* Resize the Legend to fit its contents */
    QFont legendFont = mChart->legend()->font();
    QFontMetrics fontMetrics(legendFont);
    QRectF rect = fontMetrics.boundingRect("T");
    QGraphicsLayout *layout = mChart->legend()->layout();
    double left,top,right,bottom;
    layout->getContentsMargins(&left, &top, &right, &bottom);
    int nbGraphs = 0;
    foreach(QAbstractSeries *graph, mGraphsList){
        if(graph->isVisible()){
            nbGraphs++;
        }
    }
    qreal height = (double)nbGraphs * (top + rect.height()) + bottom + top;
    layout->setMinimumHeight(height);
    layout->setMaximumHeight(height);
    /* Move the legend to the corresponding coordinates */
    QSizeF legendSize = mChart->legend()->size();
    QRectF plotArea = mChart->plotArea();
    QPointF pos;
    switch(mLegendPos){
    case lpTopLeft:
        pos = plotArea.topLeft();
        break;
    case lpTopRight:
        pos = plotArea.topRight();
        pos.setX(pos.x() - legendSize.width());
        break;
    case lpBottomLeft:
        pos = plotArea.bottomLeft();
        pos.setY(pos.y() - legendSize.height());
        break;
    case lpBottomRight:
        pos = plotArea.bottomRight();
        pos.setX(pos.x() - legendSize.width());
        pos.setY(pos.y() - legendSize.height());
        break;
    }
    mChart->legend()->setPos(pos);
}
/*!
 * \brief IPCScope::updateMarkerTablePosition. Update the marker table position in the scope.
 */
void IPCScope::updateMarkerTablePosition()
{
    QSizeF legendSize = mChart->legend()->size();
    QSizeF tableSize = mMarkerTable->size();
    QRectF plotArea = mChart->plotArea();
    QPointF pos;
    switch(mMarkerTablePos){
    case mpTopLeft:
        pos = plotArea.topLeft();
        break;
    case mpTopMidle:
        pos = (plotArea.topLeft() + plotArea.topRight())/2;
        if(legendSize.width() + tableSize.width() > plotArea.width()/2){
            pos.setX(pos.x() - (legendSize.width() + tableSize.width() - plotArea.width()/2));
        }
        break;
    case mpTopRight:
        pos = plotArea.topRight();
        pos.setX(pos.x() - legendSize.width() - tableSize.width());
        break;
    }
    mMarkerTable->move(pos.toPoint());
}

/*!
 * \brief takeClosest. Search for the closest value in a vector.
 * \param target
 * \param candidates
 * \return
 */
static double takeClosest(double searchValue, const QVector<double> &vector)
{
    if(vector.length()==1){
        return vector.first();
    }
    QVector<double>::const_iterator it = std::lower_bound(vector.constBegin(), vector.constEnd(), searchValue);
    if(it == vector.constEnd()){
        return *(it-1);
    }else if (it == vector.constBegin()){
        return *it;
    }else{
        return searchValue-*(it-1) < *it-searchValue ? *(it-1) : *it;
    }
}

/*!
 * \brief IPCScope::getMinorTicks. From the tick interval value, calculate the number of minor ticks to give a readable subgrid values.
 * \param tickInterval
 * \return
 */
int IPCScope::getMinorTicks(double tickInterval)
{
    int ret = 1;
    double magnitude;
    double intPartf;
    int intPart;
    modf(getMantissa(tickInterval,&magnitude), &intPartf);
    intPart = intPartf;

    switch (intPart){
    case 1: ret = 4; break;
    case 2: ret = 3; break;
    case 3: ret = 2; break;
    case 4: ret = 3; break;
    case 5: ret = 4; break;
    case 6: ret = 2; break;
    case 7: ret = 6; break;
    case 8: ret = 3; break;
    case 9: ret = 2; break;
    }
    return ret;
}

/*!
 * \brief IPCScope::getMantissa. An input of 123.4 will return a mantissa of 1.234 and a magnitude of 100.
 * \param input
 * \param magnitude
 * \return
 */
double IPCScope::getMantissa(double input, double *magnitude) const
{
    double mag = qPow(10.0, qFloor(qLn(input)/qLn(10.0)));
    *magnitude = mag;
    return input/mag;
}

/*!
 * \brief IPCScope::cleanMantissa. Returns a number that is close to the input but has a clean, easier readable mantissa.
 * \param input
 * \return
 */
double IPCScope::cleanMantissa(double input) const
{
    double magnitude;
    const double mantissa = getMantissa(input, &magnitude);
    return takeClosest(mantissa, QVector<double>() << 1.0 << 2.0 << 3.0 << 5.0 << 10.0)*magnitude;
}

/*!
 * \brief IPCScope::cosmeticTicksInterval. Recalculate the ticks interval after zooming. Limit an excessive number of grid lines.
 */
void IPCScope::cosmeticTicksInterval()
{
    QAbstractAxis *xAxis = mAxesList.at(0);
    QAbstractAxis *yAxis = mAxesList.at(1);

    QRectF plotArea = mChart->plotArea();
    if(mScopeType == stpSemiLogX){
        QValueAxis *axis = static_cast<QValueAxis *>(yAxis);
        double height = plotArea.height();
        double deltaY = axis->max() - axis->min();
        double tickInterval = deltaY / (height / 120);
        tickInterval = cleanMantissa(tickInterval);
        axis->setTickInterval(tickInterval);
        axis->setMinorTickCount(getMinorTicks(tickInterval));
    } else if(mScopeType == stpSemiLogY){
        QValueAxis *axis = static_cast<QValueAxis *>(xAxis);
        double width = plotArea.width();
        double deltaX = axis->max() - axis->min();
        double tickInterval = deltaX / (width / 120);
        tickInterval = cleanMantissa(tickInterval);
        axis->setTickInterval(tickInterval);
        axis->setMinorTickCount(getMinorTicks(tickInterval));
    } else if (mScopeType == stpLinear){
        QValueAxis *xaxis = static_cast<QValueAxis *>(xAxis);
        double width = plotArea.width();
        double deltaX = xaxis->max() - xaxis->min();
        QValueAxis *yaxis = static_cast<QValueAxis *>(yAxis);
        double height = plotArea.height();
        double deltaY = yaxis->max() - yaxis->min();
        double tickIntervalX = deltaX / (width / 120);
        double tickIntervalY = deltaY / (height / 120);
        tickIntervalX = cleanMantissa(tickIntervalX);
        xaxis->setTickInterval(tickIntervalX);
        xaxis->setMinorTickCount(getMinorTicks(tickIntervalX));
        tickIntervalY = cleanMantissa(tickIntervalY);
        yaxis->setTickInterval(tickIntervalY);
        yaxis->setMinorTickCount(getMinorTicks(tickIntervalY));
    }
}

/*!
 * \brief IPCScope::updateGeometry. Update the geometry of different components in the scope.
 */
void IPCScope::updateGeometry()
{
    updateLegendPosition();
    updateMarkerTablePosition();
}

/*!
 * \brief IPCScope::resizeEvent. Reimplement resizeEvent.
 * \param event
 */
void IPCScope::resizeEvent(QResizeEvent *event)
{
    if (scene()) {
        scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
         mChart->resize(event->size());
    }
    updateGeometry();
    cosmeticTicksInterval();
    QGraphicsView::resizeEvent(event);
}

/*!
 * \brief IPCScope::wheelEvent. Reimplement wheelEvent() method. Set zoom range according to the zoom directions.
 * \param event
 */
void IPCScope::wheelEvent(QWheelEvent *event)
{
    if(mChart && mZoomDirection != zdNone){
        QRectF plotArea = mChart->plotArea();
        QPointF mousePos = event->posF();
        qreal scale = event->delta()>0?mZoomWeight:1/mZoomWeight;
        QRectF zoomArea = plotArea;

        if(mZoomDirection & zdVertical){
            zoomArea.setTop(mousePos.y() + (zoomArea.top() - mousePos.y())*scale);
            zoomArea.setBottom(mousePos.y() + (zoomArea.bottom() - mousePos.y())*scale);
        }
        if (mZoomDirection & zdHorizontal) {
            zoomArea.setLeft(mousePos.x() + (zoomArea.left() - mousePos.x())*scale);
            zoomArea.setRight(mousePos.x() + (zoomArea.right() - mousePos.x())*scale);
        }
        mChart->zoomIn(zoomArea);
        cosmeticTicksInterval();
        foreach(IPCMarker *marker, mMarkerList){
            marker->updatePosition();
        }
    }
    QGraphicsView::wheelEvent(event);
}

/*!
 * \brief IPCScope::mouseDoubleClickEven. Set zoom fit when users double click on the scope.
 * \param event
 */
void IPCScope::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(mChart){
        setZoomRange(mZoomRangeX.min(), mZoomRangeY.max(), mZoomRangeX.max(), mZoomRangeY.min());
//        cosmeticTicksInterval();
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}

/*!
 * \brief IPCScope::mousePressEvent. Show the rubber band to display the zoom area.
 * \param event
 */
void IPCScope::mousePressEvent(QMouseEvent *event)
{
    mRubberBandOrigin = event->pos();
    mRubberBand->setGeometry(QRect(mRubberBandOrigin, QSize()));
    mRubberBand->show();
}

/*!
 * \brief IPCScope::mouseMoveEvent. Update the zoom area when the mouse moves.
 * \param event
 */
void IPCScope::mouseMoveEvent(QMouseEvent *event)
{
    mRubberBand->setGeometry(QRect(mRubberBandOrigin, event->pos()).normalized());
}

/*!
 * \brief IPCScope::mouseReleaseEvent. Hide the rubber band zoom area. Zoom into the area.
 * \param event
 */
void IPCScope::mouseReleaseEvent(QMouseEvent *event)
{
    mRubberBand->hide();
    /* Zoom into the rubber band area */
    if(qAbs(mRubberBandOrigin.x() - event->pos().x()) > 2){ // Do not zoom if this is a double click event.
        mChart->zoomIn(QRect(mRubberBandOrigin, event->pos()).normalized());
        cosmeticTicksInterval();
        foreach(IPCMarker *marker, mMarkerList){
            marker->updatePosition();
        }
    }
}

/*!
 * \brief IPCScope::setOpenGLEnabled. Enable hardware accelerated graphs.
 * \param enable
 */
void IPCScope::setOpenGLEnabled(bool enable)
{
    mOpenGLEnabled = enable;

    QList<QAbstractSeries *> seriesList = mChart->series();
    foreach(QAbstractSeries *series, seriesList){
        series->setUseOpenGL(enable);
    }
}

/*!
 * \brief IPCScope::setActiveGraphIdx. Change the active graph (for markers for example)
 * \param graphIdx
 */
void IPCScope::setActiveGraphIdx(int graphIdx)
{
    if((graphIdx < 0) || (graphIdx > mGraphsList.length()-1)){
        qDebug() << Q_FUNC_INFO << "index out of range:" << graphIdx;
        return;
    }
    mActiveGraphIdx = graphIdx;
    // Update markers' position
    foreach(IPCMarker *marker, mMarkerList){
        marker->setGraph(mGraphsList[graphIdx]);
    }
}

/*!
 * \brief IPCScope::addGraph. Add a new graph into the scope.
 * \param graphName
 */
void IPCScope::addGraph(QString graphName, LineStyle lineStyle)
{
    QAbstractSeries *series = 0;

    int colorIndex = (mGraphsList.length()) % mGraphColors.length();

    if(lineStyle == lsScatter){
        // Create a scatter series
        series = new QScatterSeries;
        QScatterSeries *scatter = static_cast<QScatterSeries *>(series);
        scatter->setColor(QColor(mGraphColors.at(colorIndex)));
        scatter->setMarkerSize(1.0);
        scatter->setPen(QPen("black"));
    }else if (lineStyle == lsLine){
        // Create a line series
        series = new QLineSeries;
        QLineSeries *line = static_cast<QLineSeries *>(series);
        line->setPen(QPen(QBrush(QColor(mGraphColors.at(colorIndex))), 1.0));
    } else{
        // Area type
        QLineSeries *upperLineSeries = new QLineSeries;
        QColor color = QColor(mGraphColors.at(colorIndex));
        color.setAlphaF(0.4);
        QPen pen(color);
        pen.setWidth(1);
        series = new QAreaSeries(upperLineSeries, 0);
        QAreaSeries *s = static_cast<QAreaSeries *>(series);
        s->setPen(pen);
        s->setBrush(color);
    }
    // Set name
    series->setName(graphName);
    // Attatch the series to the chart
    mChart->addSeries(series);
    // Attatch axes. MUST addSeries before attaching axes
    foreach(QAbstractAxis *axis, mAxesList){
        series->attachAxis(axis);
    }
    // Set accelerated property
    series->setUseOpenGL(mOpenGLEnabled);    
    // Append the series to the list
    mGraphsList.append(series);
    updateGeometry();
}

/*!
 * \brief IPCScope::graph. Return a QAbstractSeries at index graphIdx. graphIdx must be a valid index
 * position in the list.
 * \param graphIdx
 * \return
 */
QAbstractSeries * const &IPCScope::graph(int graphIdx) const
{
    if((graphIdx < 0) || (graphIdx > mGraphsList.length()-1)){
        qDebug() << Q_FUNC_INFO << "index out of range:" << graphIdx;
    }
    return mGraphsList.at(graphIdx);
}

/*!
 * \brief IPCScope::graph. Return the last QAbstractSeries in the graph list. The list must not empty.
 * \return
 */
QAbstractSeries * const &IPCScope::graph() const
{
    if(mGraphsList.isEmpty()){
        qDebug() << Q_FUNC_INFO << "list is empty";
    }
    return mGraphsList.at(mGraphsList.length()-1);
}

/*!
 * \brief IPCScope::clearGraph. Remove the graph at index graphIdx from the scope.
 * \param graphIdx
 */
void IPCScope::clearGraph(int graphIdx)
{
    if((graphIdx < 0) || (graphIdx > mGraphsList.length()-1)){
        qDebug() << Q_FUNC_INFO << "index out of range:" << graphIdx;
        return;
    }
    QAbstractSeries *series = mGraphsList[graphIdx];

    // If the removed graph is also the active graph, we change the active graph to the next one (or the previous one if this is the last in the list)
    if(graphIdx == mActiveGraphIdx){
        int newGraphListLen = mGraphsList.length()-1;
        if(mActiveGraphIdx > newGraphListLen-1){
            mActiveGraphIdx = newGraphListLen-1;
        }
        // Remove from the list
        mGraphsList.removeAt(graphIdx);
        // Update the marker target graph
        if(mActiveGraphIdx >= 0){
            foreach(IPCMarker *marker, mMarkerList){
                marker->setGraph(mGraphsList[mActiveGraphIdx]);
            }
        }
    } else{
        // Simply remove the graph from the list
        mGraphsList.removeAt(graphIdx);
    }

    // Remove the series from the chart and from the internal list
    mChart->removeSeries(series);

    // Delete the series
    delete series;
    updateGeometry();
}

/*!
 * \brief IPCScope::clearGraphs. Clear all the graphs in the scope. Return the number of graphs cleared.
 * \return
 */
int IPCScope::clearGraphs()
{
    int count = 0;
    while(!mGraphsList.isEmpty())
    {
        clearGraph(0);
        count++;
    }

    return count;
}

/*!
 * \brief IPCScope::setGraphVisible. Set the graph visibility.
 * \param graphIdx
 * \param visible
 */
void IPCScope::setGraphVisible(int graphIdx, bool visible)
{
    if((graphIdx < 0) || (graphIdx > mGraphsList.length()-1)){
        qDebug() << Q_FUNC_INFO << "index out of range:" << graphIdx;
        return;
    }
    mGraphsList.at(graphIdx)->setVisible(visible);
}

/*!
 * \brief IPCScope::setGraphVisible. Set the visibility of the last graph in the list.
 * \param visible
 */
void IPCScope::setGraphVisible(bool visible)
{
    if(mGraphsList.isEmpty()){
        qDebug() << Q_FUNC_INFO << "Graph list is empty.";
        return;
    }
    mGraphsList.at(mGraphsList.length()-1)->setVisible(visible);
}

/*!
 * \brief IPCScope::setGraphName. Set name of the graph at index graphIdx.
 * \param graphIdx
 * \param name
 */
void IPCScope::setGraphName(int graphIdx, QString name)
{
    if((graphIdx < 0) || (graphIdx > mGraphsList.length()-1)){
        qDebug() << Q_FUNC_INFO << "index out of range:" << graphIdx;
        return;
    }
    mGraphsList.at(graphIdx)->setName(name);
    updateGeometry();
}

/*!
 * \brief IPCScope::setGraphName. Set name of the last graph in the list.
 * \param name
 */
void IPCScope::setGraphName(QString name)
{
    if(mGraphsList.isEmpty()){
        qDebug() << Q_FUNC_INFO << "Graph list is empty.";
        return;
    }
    mGraphsList.at(mGraphsList.length()-1)->setName(name);
    updateGeometry();
}

/*!
 * \brief IPCScope::setGraphColor. Change the color of a graph.
 * \param color
 */
void IPCScope::setGraphColor(int graphIdx, const QColor &color)
{
    if((graphIdx < 0) || (graphIdx > mGraphsList.length()-1)){
        qDebug() << Q_FUNC_INFO << "index out of range:" << graphIdx;
        return;
    }
    QAbstractSeries *s = mGraphsList.at(graphIdx);
    if((s->type() == QAbstractSeries::SeriesTypeLine)||(s->type() == QAbstractSeries::SeriesTypeScatter)){
        QXYSeries *series = static_cast<QXYSeries *>(s);
        series->setColor(color);
    } else if(s->type() == QAbstractSeries::SeriesTypeArea){
        QAreaSeries *series = static_cast<QAreaSeries *>(s);
        series->setColor(color);
        series->setBorderColor(color);
        series->setPen(QPen(color));
    }
}

/*!
 * \brief IPCScope::setGraphColor. Change the color of the last graph in the list.
 * \param color
 */
void IPCScope::setGraphColor(const QColor &color)
{
    if(mGraphsList.isEmpty()){
        qDebug() << Q_FUNC_INFO << "Graph list is empty.";
        return;
    }
    setGraphColor(mGraphsList.length()-1, color);
}

/*!
 * \brief IPCScope::setGraphData. Update a graph's data. This method is the fastest since it replaces the points vector if
 * the number of points in the graph is unchanged.
 * \param graphIdx
 * \param points
 */
void IPCScope::setGraphData(int graphIdx, QVector<QPointF> points)
{
    if((graphIdx < 0) || (graphIdx > mGraphsList.length()-1)){
        qDebug() << Q_FUNC_INFO << "index out of range:" << graphIdx;
        return;
    }
    QAbstractSeries *s = mGraphsList.at(graphIdx);
    if((s->type() == QAbstractSeries::SeriesTypeLine)||(s->type() == QAbstractSeries::SeriesTypeScatter)){
        QXYSeries *series = static_cast<QXYSeries *>(s);
        if(series->count() == points.length()){
            series->replace(points);
        } else{
            series->clear();
            for(int i = 0; i < points.length(); i++){
                series->append(points.at(i));
            }
        }
    } else if(s->type() == QAbstractSeries::SeriesTypeArea){
        QAreaSeries *series = static_cast<QAreaSeries *>(s);
        QLineSeries *upperLineSeries = series->upperSeries();
        if(upperLineSeries->count() == points.length()){
            upperLineSeries->replace(points);
        } else{
            upperLineSeries->clear();
            for(int i = 0; i < points.length(); i++){
                upperLineSeries->append(points.at(i));
            }
        }
    }

    // If the graphIdx is equal to the active graph index, we also update the marker position
    if(mActiveMarkerIdx == graphIdx){
        for(int i = 0; i < mMarkerList.length(); i++){
            IPCMarker *marker = mMarkerList.at(i);
            marker->updatePosition();
            // Udate the marker table
            mMarkerTable->setMarkerPos(i, marker->pos());
        }
    }
}

/*!
 * \brief IPCScope::setGraphData. Update the data of the last graph in the list.
 * \param points
 */
void IPCScope::setGraphData(QVector<QPointF> points)
{
    if(mGraphsList.isEmpty()){
        qDebug() << Q_FUNC_INFO << "Graph list is empty.";
        return;
    }
    setGraphData(mGraphsList.length()-1, points);
}

/*!
 * \brief IPCScope::setGraphData. For C Style convenience. Update the graph data from x array and y array.
 * \param graphIdx
 * \param x
 * \param y
 * \param len
 */
void IPCScope::setGraphData(int graphIdx, double *x, double *y, int len)
{
    if((graphIdx < 0) || (graphIdx > mGraphsList.length()-1)){
        qDebug() << Q_FUNC_INFO << "index out of range:" << graphIdx;
        return;
    }
    if(len <= 0){
        qDebug() << Q_FUNC_INFO << "Non positive length:" << len;
        return;
    }
    // Form a vector of points
    QVector<QPointF> points(len);
    double *xPtr = x;
    double *yPtr = y;
    for(int i = 0; i < len; i++){
        points[i].setX(*xPtr);
        points[i].setY(*yPtr);
        xPtr++;
        yPtr++;
    }
    // Update data
    setGraphData(graphIdx, points);
}

/*!
 * \brief IPCScope::setGraphData. Update the data for the last graph in the list.
 * \param x
 * \param y
 * \param len
 */
void IPCScope::setGraphData(double *x, double *y, int len)
{
    if(mGraphsList.isEmpty()){
        qDebug() << Q_FUNC_INFO << "Graph list is empty.";
        return;
    }
    setGraphData(mGraphsList.length()-1, x, y, len);
}

/*!
 * \brief IPCScope::setGraphData. Update graph data according to its name.
 * \param name
 * \param points
 */
void IPCScope::setGraphData(QString name, QVector<QPointF> points)
{
    QStringList nameList = this->graphsNameList();
    int graphIdx = nameList.indexOf(name);

    setGraphData(graphIdx, points);
}

/*!
 * \brief IPCScope::setGraphData. Update graph data according to its name. Method signature for C style convenience.
 * \param name
 * \param x
 * \param y
 * \param len
 */
void IPCScope::setGraphData(QString name, double *x, double *y, int len)
{
    QStringList nameList = this->graphsNameList();
    int graphIdx = nameList.indexOf(name);

    setGraphData(graphIdx, x, y, len);
}

/*!
 * \brief IPCScope::setZoomRange. Zoom into a range defined by its coordinates. It is noted that the points are given in
 * graph's coordinates. p1 is the top left point and p2 is the bottom right point.
 * \param xp1
 * \param yp1
 * \param xp2
 * \param yp2
 */
void IPCScope::setZoomRange(qreal xp1, qreal yp1, qreal xp2, qreal yp2)
{
    if(mAxesList.length() < 2)
    {
        qDebug() << Q_FUNC_INFO << "x and y axes must be attached to the scope before.";
        return;
    }
    mZoomRangeX.setMin(xp1);
    mZoomRangeX.setMax(xp2);
    mZoomRangeY.setMin(yp2);
    mZoomRangeY.setMax(yp1);
    QAbstractAxis *xAxis = mAxesList.at(0);
    QAbstractAxis *yAxis = mAxesList.at(1);
    /* Before setting the range, we need to update the tick interval first
     * to limit the excessive number of ticks
    */
    if(mScopeType == stpSemiLogX){
        QValueAxis *axis = static_cast<QValueAxis *>(yAxis);
        axis->setTickInterval(mZoomRangeY.max() - mZoomRangeY.min());
    } else if(mScopeType == stpSemiLogY){
        QValueAxis *axis = static_cast<QValueAxis *>(xAxis);
        axis->setTickInterval(mZoomRangeX.max() - mZoomRangeX.min());
    } else if (mScopeType == stpLinear){
        QValueAxis *xaxis = static_cast<QValueAxis *>(xAxis);
        QValueAxis *yaxis = static_cast<QValueAxis *>(yAxis);
        xaxis->setTickInterval(mZoomRangeX.max() - mZoomRangeX.min());
        yaxis->setTickInterval(mZoomRangeY.max() - mZoomRangeY.min());
    }

    xAxis->setRange(xp1, xp2);
    yAxis->setRange(yp2, yp1);

    /*
     * Reupdate the tick for cosmetic look
    */
    cosmeticTicksInterval();

    foreach(IPCMarker *marker, mMarkerList){
        marker->updatePosition();
    }
}

/*!
 * \brief IPCScope::setZoomRange. Zoom into a range defined by its top left point and its bottom right point.
 * It is noted that the points are given in graph's coordinates.
 * \param topLeft
 * \param bottomRight
 */
void IPCScope::setZoomRange(QPointF topLeft, QPointF bottomRight)
{
    if(mAxesList.length() < 2)
    {
        qDebug() << Q_FUNC_INFO << "x and y axes must be attached to the scope before.";
        return;
    }
    QAbstractAxis *xAxis = mAxesList.at(0);
    QAbstractAxis *yAxis = mAxesList.at(1);
    xAxis->setRange(topLeft.x(), bottomRight.x());
    yAxis->setRange(bottomRight.y(), topLeft.y());
    /*
     * Reupdate the tick for cosmetic look
    */
    cosmeticTicksInterval();

    foreach(IPCMarker *marker, mMarkerList){
        marker->updatePosition();
    }
}

/*!
 * \brief IPCScope::setZoomRange. Zoom into a range defined by the bounding rectangle.
 * \param boundingRect
 */
void IPCScope::setZoomRange(QRectF boundingRect)
{
    if(mAxesList.length() < 2)
    {
        qDebug() << Q_FUNC_INFO << "x and y axes must be attached to the scope before.";
        return;
    }
    QAbstractAxis *xAxis = mAxesList.at(0);
    QAbstractAxis *yAxis = mAxesList.at(1);
    xAxis->setRange(boundingRect.left(), boundingRect.right());
    yAxis->setRange(boundingRect.top(), boundingRect.bottom());
    /*
     * Reupdate the tick for cosmetic look
    */
    cosmeticTicksInterval();

    foreach(IPCMarker *marker, mMarkerList){
        marker->updatePosition();
    }
}

/*!
 * \brief boundingRectF. Find the bounding rect of a series of points. This is a rectangle which contains all the points in the series.
 * \param points
 * \return
 */
static QRectF boundingRectF(const QVector<QPointF> &points)
{
    bool first = true;
    qreal xmin = 0;
    qreal xmax = 0;
    qreal ymin = 0;
    qreal ymax = 0;

    if(points.isEmpty()){
        return QRectF(0,0,0,0);
    }
    QVectorIterator<QPointF> it(points);
    while (it.hasNext()){
        QPointF point = it.next();
        if(first){
            xmin = point.x();
            xmax = point.x();
            ymin = point.y();
            ymax = point.y();
            first = false;
        } else{
            // Update xmin
            if(xmin > point.x()){
                xmin = point.x();
            }
            if(xmax < point.x()){
                xmax = point.x();
            }
            if(ymin > point.y()){
                ymin = point.y();
            }
            if(ymax < point.y()){
                ymax = point.y();
            }
        }
    }
    return QRectF(QPointF(xmin,ymin), QPointF(xmax,ymax)).normalized();
}

/*!
 * \brief IPCScope::setZoomFit. Search for max and min value of the graphs, then zoom to fit the content.
 */
void IPCScope::setZoomFit()
{
    QRectF contentBoundingRect = QRectF(0,0,0,0);

    foreach(QAbstractSeries *s, mGraphsList){
        QVector<QPointF> points;
        QRectF rect;
        if((s->type() == QAbstractSeries::SeriesTypeLine)||(s->type() == QAbstractSeries::SeriesTypeScatter)){
            QXYSeries *series = static_cast<QXYSeries *>(s);
            points = series->pointsVector();
            rect = boundingRectF(points);
            contentBoundingRect = rect.united(contentBoundingRect);
        } else if(s->type() == QAbstractSeries::SeriesTypeArea){
            QAreaSeries *series = static_cast<QAreaSeries *>(s);
            QLineSeries *upperLineSeries = series->upperSeries();
            points = upperLineSeries->pointsVector();
            rect = boundingRectF(points);
            contentBoundingRect = rect.united(contentBoundingRect);
        }
    }
    // Zoom into the rect
    setZoomRange(contentBoundingRect);
}

/*!
 * \brief IPCScope::addMarker. Add a marker to the scope. Associate the active graph to it.
 */
void IPCScope::addMarker()
{
    IPCMarker *marker = new IPCMarker(mChart, 0);

    // Setup marker name
    QString markerName;
    markerName.append("M");
    markerName.append(QString::number(mMarkerList.length()+1));// Marker begins from M1
    marker->setName(markerName);
    marker->setFont(mMarkerFont);
    // Set X and Y log scale properties according to the X and Y axis style
    switch (mScopeType) {
    case stpLinear:
        marker->setLogScale(false, false);
        break;
    case stpSemiLogX:
        marker->setLogScale(true, false);
        break;
    case stpSemiLogY:
        marker->setLogScale(false, true);
        break;
    case stpLogLog:
        marker->setLogScale(true, true);
        break;
    }
    // Attatch the active graph to the marker
    if(mActiveGraphIdx >= 0){
        QAbstractSeries *graph = mGraphsList[mActiveGraphIdx];
        marker->setGraph(graph);
    }
    marker->setZValue(11);
    marker->show();
    // Add the marker into the internal list
    mMarkerList.append(marker);
    // Add marker into the marker table for value display
    mMarkerTable->addMarker(marker->name(), marker->pos());
    // Setup marker look according to the scope theme
    foreach(IPCMarker *marker, mMarkerList){
        marker->setColor(mMarkerColor);
    }
    mMarkerTable->setColor(mMarkerColor);
    updateGeometry();
}

/*!
 * \brief IPCScope::clearMarker. Remove and delete the marker at index markerIdx.
 * \param markerIdx
 */
void IPCScope::clearMarker(int markerIdx)
{
    if((markerIdx < 0) || (markerIdx > mMarkerList.length()-1)){
        qDebug() << Q_FUNC_INFO << "index out of range:" << markerIdx;
        return;
    }
    IPCMarker *marker = mMarkerList[markerIdx];
    // Remove the marker from the scene (the GraphicSene of the scope)
    mChart->scene()->removeItem(marker);
    // If the active marker index is equal to the one we remove, select the next one as the active marker
    if(mActiveMarkerIdx == markerIdx){
        int newMarkerListLen = mMarkerList.length()-1;
        if(mActiveMarkerIdx > newMarkerListLen-1){
            mActiveMarkerIdx = newMarkerListLen-1;
        }
    }
    // Remove the marker from the list
    mMarkerList.removeAt(markerIdx);
    // Remove the marker from the marker table
    mMarkerTable->clearMarker(markerIdx);
    // Delete the marker
    delete marker;
    updateGeometry();
}

/*!
 * \brief IPCScope::clearMarkers. Clear all the markers.
 */
void IPCScope::clearMarkers()
{
    while(!mMarkerList.isEmpty()){
        clearMarker(0);
    }
}

/*!
 * \brief IPCScope::setMarkerKeyValue. Set the marker's key value (x value) to move the marker.
 * \param val
 */
void IPCScope::setMarkerKeyValue(int markerIdx, double val)
{
    if((markerIdx < 0) || (markerIdx > mMarkerList.length()-1)){
        qDebug() << Q_FUNC_INFO << "index out of range:" << markerIdx;
        return;
    }
    mMarkerList.at(markerIdx)->setGraphKey(val);
    // Update the marker values in the marker table
    mMarkerTable->setMarkerPos(markerIdx, mMarkerList.at(markerIdx)->pos());
    updateGeometry();
}

/*!
 * \brief IPCScope::setMarkerKeyValue. Set the active marker's graph key value. Move the active marker to the x's value val.
 * \param val
 */
void IPCScope::setMarkerKeyValue(double val)
{
    if(mActiveMarkerIdx>=0){
        mMarkerList.at(mActiveMarkerIdx)->setGraphKey(val);
        // Update the marker values in the marker table
        mMarkerTable->setMarkerPos(mActiveMarkerIdx, mMarkerList.at(mActiveMarkerIdx)->pos());
        updateGeometry();
    }
}

/*!
 * \brief IPCScope::setMarkerColor. Change the color of a marker.
 * \param markerIdx
 * \param color
 */
void IPCScope::setMarkerColor(int markerIdx, const QColor &color)
{
    if((markerIdx < 0) || (markerIdx > mMarkerList.length()-1)){
        qDebug() << Q_FUNC_INFO << "index out of range:" << markerIdx;
        return;
    }
    mMarkerList.at(markerIdx)->setColor(color);
    // Also change the color in the marker table
    mMarkerTable->setColor(markerIdx, color);
}

/*!
 * \brief IPCScope::setMarkerColor. Change the color of the last marker in the list.
 * \param color
 */
void IPCScope::setMarkerColor(const QColor &color)
{
    if(mMarkerList.isEmpty()){
        qDebug() << Q_FUNC_INFO << "marker list is empty.";
        return;
    }
    setMarkerColor(mMarkerList.length()-1, color);
}

/*!
 * \brief IPCScope::setMarkersColor. Change the color of all the markers.
 * \param color
 */
void IPCScope::setMarkersColor(const QColor &color)
{
    for(int i = 0; i < mMarkerList.length(); i++){
        setMarkerColor(i, color);
    }
}

/*!
 * \brief IPCScope::setMarkerFont. Change the font of a marker.
 * \param markerIdx
 * \param font
 */
void IPCScope::setMarkerFont(int markerIdx, const QFont &font)
{
    if((markerIdx < 0) || (markerIdx > mMarkerList.length()-1)){
        qDebug() << Q_FUNC_INFO << "index out of range:" << markerIdx;
        return;
    }
    mMarkerList.at(markerIdx)->setFont(font);
    // Also change the font in the marker table
    mMarkerTable->setFont(markerIdx, font);
    updateGeometry();
}

/*!
 * \brief IPCScope::setMarkerFont. Change the font of the last marker in the list.
 * \param font
 */
void IPCScope::setMarkerFont(const QFont &font)
{
    if(mMarkerList.isEmpty()){
        qDebug() << Q_FUNC_INFO << "marker list is empty.";
        return;
    }
    setMarkerFont(mMarkerList.length()-1, font);
}

/*!
 * \brief IPCScope::setMarkersFont. Change the font of all the markers.
 * \param font
 */
void IPCScope::setMarkersFont(const QFont &font)
{
    for(int i = 0; i < mMarkerList.length(); i++){
        setMarkerFont(i, font);
    }
}

/*!
 * \brief IPCScope::setMarkerTablePosition. Change the position of the marker table.
 * \param pos
 */
void IPCScope::setMarkerTablePosition(MarkerTablePosition pos)
{
    mMarkerTablePos = pos;
    updateMarkerTablePosition();
}

/*!
 * \brief IPCScope::setLegendBorderPen. Set the pen used to draw the legend border.
 * \param pen
 */
void IPCScope::setLegendBorderPen(const QPen &pen)
{
    mChart->legend()->setPen(pen);
    mChart->legend()->update();
}

/*!
 * \brief IPCScope::setLegendBrush. Change the legend's background brush.
 * \param brush
 */
void IPCScope::setLegendBrush(const QBrush &brush)
{
    mChart->legend()->setBrush(brush);
    mChart->legend()->update();
}

/*!
 * \brief IPCScope::setLegendLabelColor. Change the legend's label color (text color).
 * \param color
 */
void IPCScope::setLegendLabelColor(const QColor &color)
{
    mChart->legend()->setLabelColor(color);
    mChart->legend()->update();
}

/*!
 * \brief IPCScope::setLegendFont. Change the legend's font.
 * \param font
 */
void IPCScope::setLegendFont(const QFont &font)
{
    mChart->legend()->setFont(font);
    mChart->legend()->update();
}

/*!
 * \brief IPCScope::setLegendPosition. Change the position of the legend on the scope.
 * \param pos
 */
void IPCScope::setLegendPosition(LegendPosition pos)
{
    mLegendPos = pos;
    updateGeometry();
}

/*!
 * \brief IPCScope::setScopeThemeDark. Format the scope for the dark theme.
 */
void IPCScope::setScopeThemeDark()
{
    /* Axes and grid */
    foreach(QAbstractAxis *axis, mAxesList){
        axis->setLabelsColor(QColor("#ffffff"));
        axis->setTitleBrush(QBrush(QColor("#ffffff")));
        axis->setLinePen(QPen(QColor("#7c7c7c"))); // Draw axis line and tick marks
        axis->setGridLinePen(QPen(QColor("#7c7c7c"))); // Draw the grid line
        QPen subGridPen;
        subGridPen.setColor(QColor("#7c7c7c"));
        subGridPen.setStyle(Qt::DotLine);
        axis->setMinorGridLinePen(subGridPen);        
    }
    /* Legend */
    setLegendBrush(QBrush(QColor("#000000")));
    setLegendLabelColor(QColor("#ffffff"));
    setLegendBorderPen(QPen(QColor("#50f100")));
    /* Marker */
    mMarkerColor = QColor(Qt::green);
    foreach(IPCMarker *marker, mMarkerList){
        marker->setColor(mMarkerColor);
    }
    mMarkerTable->setColor(mMarkerColor);
    /* Background */
    mChart->setBackgroundBrush(QBrush(QColor("#000000")));
}

/*!
 * \brief IPCScope::setScopeThemeLight. Format the scope for light theme.
 */
void IPCScope::setScopeThemeLight()
{
    /* Axes and grid */
    foreach(QAbstractAxis *axis, mAxesList){
        axis->setLabelsColor(QColor("#000000"));
        axis->setTitleBrush(QBrush(QColor("#000000")));
        axis->setLinePen(QPen(QColor("#7c7c7c"))); // Draw axis line and tick marks
        axis->setGridLinePen(QPen(QColor("#7c7c7c"))); // Draw the grid line
        QPen subGridPen;
        subGridPen.setColor(QColor("#7c7c7c"));
        subGridPen.setStyle(Qt::DotLine);
        axis->setMinorGridLinePen(subGridPen);
    }
    /* Legend */
    setLegendBrush(QBrush(QColor("#ffffff")));
    setLegendLabelColor(QColor("#000000"));
    setLegendBorderPen(QPen(QColor("#000000")));
    /* Marker */
    mMarkerColor = QColor(Qt::darkCyan);
    foreach(IPCMarker *marker, mMarkerList){
        marker->setColor(mMarkerColor);
    }
    mMarkerTable->setColor(mMarkerColor);
    /* Background */
    mChart->setBackgroundBrush(QBrush(QColor("#ffffff")));
}

/*!
 * \brief IPCScope::setScopeTheme. Change the scope theme.
 * \param theme
 */
void IPCScope::setScopeTheme(ScopeTheme theme)
{
    mScopeTheme = theme;
    switch (theme) {
    case stLight:
        setScopeThemeLight();
        break;
    case stDark:
        setScopeThemeDark();
        break;
    }
}

/*!
 * \brief tableToString. Extract string fom a table. Prepare for print.
 * \param table
 * \return
 */
static QString tableToString(QTableWidget *table, bool headerVisible, QString name, double scale)
{
    QString strStream;
    QTextStream out(&strStream);

    int rowCount = table->rowCount();
    int columnCount = table->columnCount();
    if(headerVisible){
        out <<  "<html>\n"
            "<head>\n"
            "<meta Content=\"Text/html; charset=Windows-1251\">\n"
            <<  QString("<title>%1</title>\n").arg(name)
            <<  "</head>\n"
            "<body bgcolor=#ffffff link=#5000A0>\n"
            "<table border=0 cellspacing=5 cellpadding=2>\n";

        // Headers
        out << "<thead><tr bgcolor=#f0f0f0>";
        for (int column = 0; column < columnCount; column++)
            if (!table->isColumnHidden(column))
                out << QString("<th>%1</th>").arg(table->model()->headerData(column, Qt::Horizontal).toString());
        out << "</tr></thead>\n";
    }
    // Table data
    for (int row = 0; row < rowCount; row++){
        out << "<tr>";
        for (int column = 0; column < columnCount; column++){
            if (!table->isColumnHidden(column)) {
                QString data = table->item(row, column)->text();
                QColor color = table->item(row, column)->textColor();
                QFont font = table->item(row, column)->font();
                QFontInfo info(font);
                QString family = info.family();
                bool bold = info.bold();
                int size = (int)round((double)info.pixelSize() * scale);
                if(bold)
                    out << QString("<td style=""color:%1;font-family:""%2"";font-size:%3px;font-weight:bold;"">%4 </td>").arg(color.name()).arg(family).arg(QString::number(size)).arg((!data.isEmpty()) ? data : QString("&nbsp;"));
                else
                    out << QString("<td style=""color:%1;font-family:""%2"";font-size:%3px;"">%4 </td>").arg(color.name()).arg(family).arg(QString::number(size)).arg((!data.isEmpty()) ? data : QString("&nbsp;"));
            }
        }
        out << "</tr>\n";
    }
    out <<  "</table>\n"
        "</body>\n"
        "</html>\n";

    return strStream;
}

/*!
 * \brief IPCScope::toPixmap. Render the scope into a pixmap.
 * \param width
 * \param height
 * \param scale
 * \return
 */
QPixmap IPCScope::toPixmap(int width, int height, double scale)
{
    int newWidth, newHeight;
    if (width == 0 || height == 0){
      newWidth = this->width();
      newHeight = this->height();
    } else{
      newWidth = width;
      newHeight = height;
    }
    int scaledWidth = qRound(scale*newWidth);
    int scaledHeight = qRound(scale*newHeight);

    QPixmap result(scaledWidth, scaledHeight);
    //result.fill(mBackgroundBrush.style() == Qt::SolidPattern ? mBackgroundBrush.color() : Qt::transparent); // if using non-solid pattern, make transparent now and draw brush pattern later
    result.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&result);
    if (painter.isActive()){
        painter.scale(scale, scale);
        this->render(&painter);

        /* Print the marker table */
        if(mMarkerTable->isVisible()){
            QString markertableText = tableToString(mMarkerTable, false, mScopeName, scale);
            QTextDocument *document = new QTextDocument();
            document->setHtml(markertableText);
            QPoint tablePos = mMarkerTable->pos();
            tablePos.setX((int)round((double)tablePos.x()*scale));
            tablePos.setY((int)round((double)tablePos.y()*scale));
            painter.translate(tablePos);
            document->drawContents(&painter);

            delete document;
        }
        painter.end();
    } else // might happen if pixmap has width or height zero
    {
        qDebug() << Q_FUNC_INFO << "Couldn't activate painter on pixmap";
        return QPixmap();
    }
    return result;
}

/*!
 * \brief IPCScope::savePdf. Save the scope into a pdf file.
 * \param fileName
 * \param width
 * \param height
 * \param pdfCreator
 * \param pdfTitle
 */
void IPCScope::savePdf(const QString &fileName, int width, int height, const QString &pdfCreator, const QString &pdfTitle)
{
    int newWidth, newHeight;
    if (width == 0 || height == 0){
      newWidth = this->width();
      newHeight = this->height();
    } else{
      newWidth = width;
      newHeight = height;
    }
    QPrinter printer(QPrinter::ScreenResolution);
    printer.setOutputFileName(fileName);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setColorMode(QPrinter::Color);
    printer.printEngine()->setProperty(QPrintEngine::PPK_Creator, pdfCreator);
    printer.printEngine()->setProperty(QPrintEngine::PPK_DocumentName, pdfTitle);

    QPainter painter(&printer);

    double xscale = printer.pageRect().width() / double(newWidth);
    double yscale = printer.pageRect().height() / double(newHeight);
    double scale = qMin(xscale, yscale);
    painter.translate(printer.paperRect().center());
    painter.scale(scale, scale);
    painter.translate(-newWidth/ 2, -newHeight/ 2);
    this->render(&painter);

    /* Print the marker table */
    if(mMarkerTable->isVisible()){
        QString markertableText = tableToString(mMarkerTable, false, mScopeName, scale);
        QTextDocument *document = new QTextDocument();
        document->setHtml(markertableText);

        QPoint tablePos = mMarkerTable->pos();
        tablePos.setX((int)round((double)tablePos.x()*scale));
        tablePos.setY((int)round((double)tablePos.y()*scale));
        painter.translate(tablePos);
        document->drawContents(&painter);

        delete document;
    }
}

/*!
 * \brief IPCScope::savePng. Save the scope to a png file.
 * \param fileName
 * \param width
 * \param height
 * \param scale
 * \param quality
 * \param resolution
 */
bool IPCScope::savePng(const QString &fileName, int width, int height, double scale, int quality, int dotPerInch)
{
    QImage buffer = toPixmap(width, height, scale).toImage();

    int dotsPerMeter = 0;
    dotsPerMeter = dotPerInch/0.0254;

    buffer.setDotsPerMeterX(dotsPerMeter);
    buffer.setDotsPerMeterY(dotsPerMeter);
    if (!buffer.isNull())
      return buffer.save(fileName, "PNG", quality);
    else
      return false;
}

/*!
 * \brief IPCScope::graphsNameList. Return a list of graph's name.
 * \return
 */
QStringList IPCScope::graphsNameList() const
{
    QStringList ret;
    foreach(QAbstractSeries *graph, mGraphsList){
        ret.append(graph->name());
    }
    return ret;
}
