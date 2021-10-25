#ifndef IPCMARKERTABLE_H
#define IPCMARKERTABLE_H

#include <QTableWidget>
#include <QHeaderView>
#include <QScrollBar>
#include <QMouseEvent>
#include <QDebug>

class IPCMarkerTable : public QTableWidget
{
    Q_OBJECT
public:
    IPCMarkerTable(QWidget *parent = nullptr);

    enum KeyDisplayType { kdFrequency     /// Reformat the key value to display frequency value
                         ,kdTime          /// Reformat the key value to display time value
                         ,kdNone          /// No format
                        };
    Q_ENUMS(KeyDisplayType)

    // Resize to contents
    void resizeToContents();
    // Add one marker entry
    void addMarker(QString name = "", QPointF pos = QPointF(0,0));
    void clearMarker(int markerIdx);
    void clearMarkers();
    // Setters
    void setKeyDisplayType(const KeyDisplayType &type){mKeyDisplayType = type;}
    void setYText(const QString &text){mYText = text;}
    void setPrecision(int precision){mPrecision = precision;}
    void setColor(int markerIdx, const QColor &color);
    void setColor(const QColor &color);
    void setFont(int markerIdx, const QFont &font);
    void setFont(const QFont &font);
    void setMarkerPos(int markerIdx, QPointF pos);
    // Getters
    KeyDisplayType keyDisplayType() const {return mKeyDisplayType;}
    QString yText() const{return mYText;}
    int precision() const{return mPrecision;}
    QColor color() const{return mColor;}
    QFont font() const{return mFont;}
protected:
    void viewSetup();
    void freqTextFormat(double x, int precision, int len, QString &xString, QString &xUnitString);
    void timeTextFormat(double x, int precision, int len, QString &xString, QString &xUnitString);
    void textNoFormat(double x, int precision, int len,  QString &xString, QString &xUnitString);
    void markerTextReformat(double x, int precision, int len,  QString &xString, QString &xUnitString);
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:
    /* Used to move the table upon mouse move */
    QPoint mOrigin;
    QPoint lastMousePos;
    // Key display type
    KeyDisplayType mKeyDisplayType;
    // Key display precision
    int mPrecision;
    // y coordinate display text
    QString mYText;
    // Color
    QColor mColor;
    // Font
    QFont mFont;
};

#endif // IPCMARKERTABLE_H
