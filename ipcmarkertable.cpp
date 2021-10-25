#include "ipcmarkertable.h"

IPCMarkerTable::IPCMarkerTable(QWidget *parent) :
    QTableWidget(0,5,parent),
    mKeyDisplayType(kdNone),
    mPrecision(2),
    mYText(""),
    mColor(QColor(Qt::black)),
    mFont(QFont())
{
    viewSetup();
}

/*!
 * \brief IPCMarkerTable::setColor. Change the color of the corresponding marker.
 * \param markerIdx
 * \param color
 */
void IPCMarkerTable::setColor(int markerIdx, const QColor &color)
{
    if((markerIdx < 0)||(markerIdx > this->rowCount()-1)){
        qDebug() << Q_FUNC_INFO << "index out of range.";
    }
    mColor = color;
    for(int j = 0; j < this->columnCount(); j++){
        this->item(markerIdx, j)->setTextColor(color);
    }
}

/*!
 * \brief IPCMarkerTable::setColor. Set color for all the items in the table.
 * \param color
 */
void IPCMarkerTable::setColor(const QColor &color)
{
    mColor = color;
    for(int i = 0; i < this->rowCount(); i++){
        for(int j = 0; j < this->columnCount(); j++){
            this->item(i, j)->setTextColor(mColor);
        }
    }
}

/*!
 * \brief IPCMarkerTable::setFont. Change the font of the corresponding marker.
 * \param markerIdx
 * \param font
 */
void IPCMarkerTable::setFont(int markerIdx, const QFont &font)
{
    if((markerIdx < 0)||(markerIdx > this->rowCount()-1)){
        qDebug() << Q_FUNC_INFO << "index out of range.";
    }
    mFont = font;
    for(int j = 0; j < this->columnCount(); j++){
        this->item(markerIdx, j)->setFont(font);
    }
    resizeToContents();
}

/*!
 * \brief IPCMarkerTable::setFont. Change the font of all items in the table.
 * \param font
 */
void IPCMarkerTable::setFont(const QFont &font)
{
    mFont = font;
    for(int i = 0; i < this->rowCount(); i++){
        for(int j = 0; j < this->columnCount(); j++){
            this->item(i, j)->setFont(font);
        }
    }
    resizeToContents();
}

/*!
 * \brief IPCMarkerTable::setMarkerPos. Update display the marker position.
 * \param markerIdx
 * \param pos
 */
void IPCMarkerTable::setMarkerPos(int markerIdx, QPointF pos)
{
    if((markerIdx < 0)||(markerIdx > this->rowCount()-1)){
        qDebug() << Q_FUNC_INFO << "index out of range.";
    }
    /* Format for display texts */
    QString xString, xUnitString;
    markerTextReformat(pos.x(), mPrecision, 0, xString, xUnitString);

    this->item(markerIdx, 1)->setText(xString);
    this->item(markerIdx, 2)->setText(xUnitString);
    this->item(markerIdx, 3)->setText(QString::number(pos.y(),'f', mPrecision));
    resizeToContents();
}

/*!
 * \brief IPCMarkerTable::viewSetup. Setup the table display.
 */
void IPCMarkerTable::viewSetup()
{
    // Do not show the headers
    this->horizontalHeader()->setVisible(false);
    this->verticalHeader()->setVisible(false);
    // Do not show grid
    this->setShowGrid(false);
    // Do not show the border
    this->setFrameStyle(QFrame::NoFrame);
    // Transparent background
    this->setStyleSheet("background-color: transparent;");
    // Stretch
    this->setFixedSize(2, 2);
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    this->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    // Hide the scroll bar
    this->verticalScrollBar()->setVisible(false);
    this->horizontalScrollBar()->setVisible(false);
}

/*!
 * \brief IPCMarkerTable::freqTexFormat. Frequency-type reformat x value string and x unit string for one marker table entry.
 * \param x
 * \param precision
 * \param len
 * \param xString
 * \param xUnitString
 */
void IPCMarkerTable::freqTextFormat(double x, int precision, int len, QString &xString, QString &xUnitString)
{
    double newVal;
    QString xUnit;

    if(qAbs(x) > 1e6-0.00001){ // MHz
        xUnit = QString("MHz");
        newVal = x / 1e6;
    }else if(qAbs(x) > 1e3-0.00001){ // KHz
        xUnit = QString("KHz");
        newVal = x / 1e3;
    }else if(qAbs(x) > 1-0.00001){ // Hz
        xUnit = QString("Hz");
        newVal = x;
    }else{ // mHz
        xUnit = QString("mHz");
        newVal = x * 1e3;
    }

    xString = QString::number(newVal, 'f', precision);

    while(xString.length() < len){
        xString.append(" ");
    }

    xUnitString = xUnit;
}

/*!
 * \brief IPCMarkerTable::timeTexFormat. Time-type reformat x value string and x unit string for one marker table entry.
 * \param x
 * \param precision
 * \param len
 * \param xString
 * \param xUnitString
 */
void IPCMarkerTable::timeTextFormat(double x, int precision, int len, QString &xString, QString &xUnitString)
{
    double newVal;
    QString xUnit;

    if(x > 1-0.00001){ // Seconds
        xUnit = QString("s");
        newVal = x;
    }else{ // ms
        xUnit = QString("ms");
        newVal = x * 1e3;
    }

    xString = QString::number(newVal, 'f', precision);

    while(xString.length() < len){
        xString.append(" ");
    }

    xUnitString = xUnit;
}

/*!
 * \brief IPCMarkerTable::textNoFormat. No-type reformat x value string and x unit string for one marker table entry.
 * \param x
 * \param precision
 * \param len
 * \param xString
 * \param xUnitString
 */
void IPCMarkerTable::textNoFormat(double x, int precision, int len,  QString &xString, QString &xUnitString)
{
    xString = QString::number(x, 'f', precision);

    while(xString.length() < len)
    {
        xString.append(" ");
    }

    xUnitString = "";
}

void IPCMarkerTable::markerTextReformat(double x, int precision, int len,  QString &xString, QString &xUnitString)
{
    QString xUnit;
    QString xValText, yValtext;

    switch (mKeyDisplayType) {
    case kdFrequency:
        freqTextFormat(x, precision, len, xString, xUnitString);
        break;
    case kdTime:
        timeTextFormat(x, precision, len, xString, xUnitString);
        break;
    case kdNone:
        textNoFormat(x, precision, len, xString, xUnitString);
        break;
    }
}

/*!
 * \brief IPCMarkerTable::addMarker. Insert a marker into the table.
 * \param name
 * \param pos
 */
void IPCMarkerTable::addMarker(QString name, QPointF pos)
{
    /* Format for display texts */
    QString xString, xUnitString;
    markerTextReformat(pos.x(), mPrecision, 0, xString, xUnitString);
    /* Insert the marker into the marker table */
    this->insertRow(this->rowCount());

    this->setItem( this->rowCount()-1, 0, new QTableWidgetItem(name));
    this->setItem( this->rowCount()-1, 1, new QTableWidgetItem(xString));
    this->setItem( this->rowCount()-1, 2, new QTableWidgetItem(xUnitString));
    this->setItem( this->rowCount()-1, 3, new QTableWidgetItem(QString::number(pos.y(),'f',2)));
    this->setItem( this->rowCount()-1, 4, new QTableWidgetItem(mYText));

    for(int i = 0; i < this->columnCount(); i++)
    {
        this->item(this->rowCount()-1, i)->setTextColor(mColor);
        this->item(this->rowCount()-1, i)->setFont(mFont);
        this->item(this->rowCount()-1, i)->setFlags(this->item(this->rowCount()-1, i)->flags() & ~Qt::ItemIsSelectable);
    }
    resizeToContents();
}

/*!
 * \brief IPCMarkerTable::clearMarker. Remove a marker from the table.
 * \param markerIdx
 */
void IPCMarkerTable::clearMarker(int markerIdx)
{
    if((markerIdx < 0) || (markerIdx > this->rowCount()-1)){
        qDebug() << Q_FUNC_INFO << "index out of range.";
        return;
    }
    for(int i = 0; i < this->columnCount(); i++){
        QTableWidgetItem *item = this->item(markerIdx,i);
        delete item;
    }
    this->removeRow(markerIdx);
    resizeToContents();
}

/*!
 * \brief IPCMarkerTable::clearMarkers. Clear all the markers in the table.
 */
void IPCMarkerTable::clearMarkers()
{
    while(this->rowCount()>0){
        clearMarker(0);
    }
}

/*!
 * \brief IPCMarkerTable::resizeToContents. Resize columns' width to fit the contents.
 */
void IPCMarkerTable::resizeToContents()
{
    int nbCol = this->columnCount();
    int nbRow = this->rowCount();
    int tableWidth = 0;
    int tableHeight = 0;
    // Resize for each column
    for(int i = 0; i < nbCol; i++){
        int maxWidth = 0;
        int height = 0;
        for(int j = 0; j < nbRow; j++){
            QTableWidgetItem *item = this->item(j,i);
            QString itemText = item->text();
            // Calculate the number of pixel for the item text
            QFontMetrics fontMetrics(this->font());
            QRect textRect = fontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip|Qt::AlignLeft, itemText);
            int recWidth = textRect.width();
            if(maxWidth < recWidth){
                maxWidth = recWidth;
            }
            if(i == 0){ // For the first column, calculate then resize the row height
                height = textRect.height()+ this->contentsMargins().top() + this->contentsMargins().bottom();
                this->verticalHeader()->resizeSection(j, height+5);
                tableHeight += this->rowHeight(j);
            }
        }
        // Resize the column, add padding
        int finalWidth =  maxWidth + this->contentsMargins().left() + this->contentsMargins().right() + 12;
        this->horizontalHeader()->resizeSection(i, finalWidth);
        tableWidth += this->columnWidth(i);
    }
    this->setFixedSize(tableWidth+7, tableHeight+7);
    return;
}

/*!
 * \brief IPCMarkerTable::mousePressEvent. Register the clicked point in order to move the widget later in MouseMoveEvent() method.
 * \param event
 */
void IPCMarkerTable::mousePressEvent(QMouseEvent *event)
{
    lastMousePos = event->globalPos();
    mOrigin = this->pos();
}

/*!
 * \brief IPCMarkerTable::mouseMoveEvent. Move the table around with the mouse.
 * \param event
 */
void IPCMarkerTable::mouseMoveEvent(QMouseEvent *event)
{
    QPoint delta = event->globalPos() - lastMousePos;
    this->move(mOrigin + delta);
    QWidget::mouseMoveEvent(event);
}
