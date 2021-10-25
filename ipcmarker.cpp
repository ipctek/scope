#include "ipcmarker.h"

IPCMarker::IPCMarker(QChart *parentChart, QXYSeries *targetGraph) :
    QGraphicsItem(parentChart),    
    mSize(8),
    mStyle(msSquare),
    mGraphKey(0),
    mInterpolating(true),
    mXLog(false),
    mYLog(false)
{
    mParentChart = parentChart;
    mTargetGraph = targetGraph;

    mBrush = QBrush(Qt::NoBrush);
    mPen = QPen(mBrush, 1.5);
    setColor(QColor("blue"));
    mPos.setX(0);
    mPos.setY(0);
    setFont(QFont());
    mNamePos = mnpTop;
    mNameVisible = true;
    mNamePad = 2;
    setName("M");
}

/*!
 * \brief IPCMarker::setName. The marker name is displayed along with the marker point if the marker name is visible.
 * \param name
 */
void IPCMarker::setName(const QString &name)
{
    mName = name;

    // Calculate the name's bounding rectangle according to the font metrics
    QFontMetrics fontMetrics(mFont);
    mNameRect = fontMetrics.boundingRect(mName);
    qreal h = mNameRect.height();
    qreal w = mNameRect.width();
    // Translate the rectangle to above or below according to the name position property
    if(mNamePos == mnpTop){
        mNameRect.translate(-w/2, - mSize - mNamePad);
    } else if(mNamePos == mnpBottom){
        mNameRect.translate(-w/2, h + mSize + mNamePad);
    } else if(mNamePos == mnpLeft){
        mNameRect.translate(-w - mNamePad - mSize, h/2);
    } else{
        mNameRect.translate(mSize + mNamePad, h/2);
    }
}

/*!
 * \brief IPCMarker::setFont. Update the marker name's font.
 * \param font
 */
void IPCMarker::setFont(const QFont &font)
{
    mFont = font;
    // Reset the name in order to update the name bounding rect
    setName(mName);
}

/*!
 * \brief IPCMarker::setGraph. Attach the marker to a graph.
 * \param graph
 */
void IPCMarker::setGraph(QAbstractSeries *graph)
{
    if (graph){
        if (graph->chart() == mParentChart){
            mTargetGraph = graph;
            updatePosition();
        } else{
            qDebug() << Q_FUNC_INFO << "graph isn't in same QChart instance as this marker";
        }
    } else{
        mTargetGraph = 0;
    }
}

/*!
 * \brief IPCMarker::setGraphKey. Move the marker to the key value (abcissa value)
 * \param key
 */
void IPCMarker::setGraphKey(double key)
{
    mGraphKey = key;
    updatePosition();
}

/*!
 * \brief IPCMarker::updatePosition. Update the marker position. This method can be called each time the graph data has changed
 * in order to update the marker position on the graph.
 */
void IPCMarker::updatePosition()
{    
    if(mTargetGraph){
        if(mParentChart){
            QXYSeries *series = 0;
            if((mTargetGraph->type() == QAbstractSeries::SeriesTypeLine)|| (mTargetGraph->type() == QAbstractSeries::SeriesTypeScatter)){
                series = static_cast<QXYSeries *>(mTargetGraph);
            } else if (mTargetGraph->type() == QAbstractSeries::SeriesTypeArea){
                QAreaSeries *s = static_cast<QAreaSeries *>(mTargetGraph);
                series = s->upperSeries();
            } else{
                qDebug() << Q_FUNC_INFO << " series is neither Area nor Line nor Scatter.";
                return;
            }
            if (series->pointsVector().size() > 1){
                QVector<QPointF>::const_iterator first = series->pointsVector().constBegin();
                QVector<QPointF>::const_iterator last = series->pointsVector().constEnd()-1;
                if (mGraphKey <= first->x()){
                    mPos.setX(first->x());
                    mPos.setY(first->y());
                } else if (mGraphKey >= last->x()){
                    mPos.setX(last->x());
                    mPos.setY(last->y());
                } else{
                    /* Find the lower bound */
                    QVector<QPointF> points = series->pointsVector();
                    QPointF keyPoint(mGraphKey, 0);

                    QVector<QPointF>::const_iterator it = std::lower_bound(points.constBegin(), points.constEnd(), keyPoint, QPointFLessThan);

                    if(it != points.constBegin())
                    {
                        it--;
                    }
                    QVector<QPointF>::const_iterator prevIt = it; // Previous iteration
                    ++it; // Won't pass the constEnd because we handled that case (mGraphKey >= last->x()) before

                    // Now, prevIt is the iterator pointing to the point before the mGraphKey, and it is the iterator just after
                    // We calculate now the marker position according to interpolating option
                    // Note that the marker position is in the graph's coordinate, NOT the Chart's pixel coordinate.
                    if (mInterpolating)
                    {
                        // interpolate between the two iterators around mGraphKey:
                        double slope = 0;
                        double x1 = prevIt->x();
                        double x2 = it->x();
                        double y1 = prevIt->y();
                        double y2 = it->y();
                        double x = mGraphKey;
                        if(mXLog){
                            if(x1 > 0){
                                x1 = log10(x1);
                            }
                            if(x2 > 0){
                                x2 = log10(x2);
                            }
                            if(x > 0){
                                x = log10(x);
                            }
                        }
                        if(mYLog){
                            if(y1 > 0){
                                y1 = log10(y1);
                            }
                            if(y2 > 0){
                                y2 = log10(y2);
                            }
                        }
                        if (!qFuzzyCompare(x1, x2)){
                            slope = (y2 - y1)/(x2 - x1);
                        }
                        double y = y1 + (x - x1)*slope;
                        mPos.setX(mGraphKey);
                        if(mYLog){
                            mPos.setY(pow(10, y));
                        }else {
                            mPos.setY(y);
                        }
                    } else{
                        // Find the iterator with key closest to mGraphKey:
                        if (mGraphKey < (prevIt->x()+it->x())*0.5){
                            mPos.setX(prevIt->x());
                            mPos.setY(prevIt->y());
                        } else{
                            mPos.setX(it->x());
                            mPos.setY(it->y());
                        }
                    }
                }
            } else if (series->pointsVector().size() == 1){
                QVector<QPointF>::const_iterator it = series->pointsVector().constBegin();
                mPos.setX(it->x());
                mPos.setY(it->y());
            }
        }
        prepareGeometryChange();
        // Move the marker to the position in parent chart's coordinates
        setPos(mParentChart->mapToPosition(mPos));
    }
}

/*!
 * \brief IPCMarker::boundingRect. Bounding rect of the marker.
 * \return
 */
QRectF IPCMarker::boundingRect() const
{
    if(mParentChart){
        QPointF center = mapFromParent(mParentChart->mapToPosition(mPos));
        double w = mSize/2.0;

        QRectF rect = QRectF(center-QPointF(w, w), center+QPointF(w, w));
        // Unite with the text rectangle
        QRectF textRect = mNameRect;
        textRect.translate(center);

        rect = rect.united(textRect);
        // Compensate for the penwidth
        qreal penWidth = mPen.widthF();
        rect.adjust(-penWidth/2-5, -penWidth/2-5, penWidth/2+5, penWidth/2+5);
        //rect.adjust(-penWidth, -penWidth, penWidth, penWidth);
        return rect;
    }
    return QRectF(0,0,0,0);
}

/*!
 * \brief IPCMarker::paint. Draw the marker according to the marker style.
 * \param painter
 * \param option
 * \param widget
 */
void IPCMarker::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (mStyle == msNone)
      return;

    if(mParentChart){
        painter->setFont(mFont);
        painter->setPen(mPen);
        painter->setBrush(mBrush);
        QPointF center = mapFromParent(mParentChart->mapToPosition(mPos));
        double w = mSize/2.0;
        QRectF clipParent = mParentChart->plotArea();
        QRectF clip = QRectF(mapFromParent(clipParent.topLeft()), mapFromParent(clipParent.bottomRight()));
        switch (mStyle){
            case msNone: return;
            case msPlus:
            {
                if (clip.intersects(QRectF(center-QPointF(w, w), center+QPointF(w, w)))){
                    painter->drawLine(QLineF(center+QPointF(-w, 0), center+QPointF(w, 0)));
                    painter->drawLine(QLineF(center+QPointF(0, -w), center+QPointF(0, w)));
                }
                break;
            }
            case msCrosshair:
            {
                if (center.y() > clip.top() && center.y() < clip.bottom())
                    painter->drawLine(QLineF(clip.left(), center.y(), clip.right(), center.y()));
                if (center.x() > clip.left() && center.x() < clip.right())
                    painter->drawLine(QLineF(center.x(), clip.top(), center.x(), clip.bottom()));
                break;
            }
            case msCircle:
            {
                if (clip.intersects(QRectF(center-QPointF(w, w), center+QPointF(w, w))))
                    painter->drawEllipse(center, w, w);
                break;
            }
            case msSquare:
            {
                if (clip.intersects(QRectF(center-QPointF(w, w), center+QPointF(w, w))))
                    painter->drawRect(QRectF(center-QPointF(w, w), center+QPointF(w, w)));
                break;
            }
        }
        if(mNameVisible){            
            // Draw the Marker name
            QRectF textRect = mNameRect;
            textRect.translate(center);
            textRect.adjust(-5,-5,5,5);
            painter->drawText(textRect, mName);
        }
    }
}
