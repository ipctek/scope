#ifndef IPCMARKER_H
#define IPCMARKER_H

#include <QtCharts>

using namespace QtCharts;

inline bool QPointFLessThan(const QPointF &a, const QPointF &b) { return a.x() < b.x(); }

class IPCMarker : public QGraphicsItem
{
public:
    explicit IPCMarker(QChart *parentChart, QXYSeries *targetGraph = nullptr);

    enum MarkerStyle {  msNone       /// The marker is not visible
                       ,msPlus       /// A plus shaped crosshair with limited size
                       ,msCrosshair  /// A plus shaped crosshair which spans the complete axis rect
                       ,msCircle     /// A circle
                       ,msSquare     /// A square
                     };
    Q_ENUMS(MarkerStyle)

    enum MarkerNamePosition {  mnpTop       /// The marker name is displayed above the marker point
                              ,mnpBottom    /// The marker name is displayed under the marker point
                              ,mnpLeft      /// The marker name is displayed at the left of the marker point
                              ,mnpRight     /// The marker name is displayed at the right of the marker point
                            };
    Q_ENUMS(MarkerNamePosition)

    // Setters
    void setName(const QString &name);
    void setNameVisible(bool visible){mNameVisible = visible;}
    void setNamePosition(MarkerNamePosition pos){mNamePos = pos;}
    void setNamePadding(int pad){mNamePad = pad;}
    void setPen(const QPen &pen){mPen = pen;}
    void setBrush(const QBrush &brush){mBrush = brush; mPen.setBrush(mBrush);}
    void setColor(const QColor &color){mColor = color; mPen.setColor(mColor);}
    void setFont(const QFont &font);
    void setSize(double size){mSize = size;}
    void setStyle(MarkerStyle style){mStyle = style;}
    void setGraph(QAbstractSeries *graph);
    void setGraphKey(double key);
    void setInterpolating(bool enabled){mInterpolating = enabled;}
    void setLogScale(bool xLog, bool yLog){mXLog = xLog; mYLog = yLog;}

    // Getters
    QString name() const {return mName;}
    bool nameVisible(){return mNameVisible;}
    MarkerNamePosition namePosition(){return mNamePos;}
    int namePadding(){return mNamePad;}
    QPen pen() const {return mPen;}
    QBrush brush() const {return mBrush;}
    QColor color() const {return mColor;}
    QFont font() const {return mFont;}
    double size() const {return mSize;}
    MarkerStyle style() const {return mStyle;}
    QAbstractSeries *graph(){return mTargetGraph;}
    double graphKey() const {return mGraphKey;}
    bool interpolating() const {return mInterpolating;}
    QPointF pos() const {return mPos;}

    // Update the position when the graph data changes
    void updatePosition();

    // Implement the boundingRect method for hit test
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    // Implement the paint method of the QGraphicsItem class
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;

private:
    // property members
    QChart *mParentChart;
    QAbstractSeries *mTargetGraph;

    QString mName;
    QRectF mNameRect;
    bool mNameVisible;
    MarkerNamePosition mNamePos;
    int mNamePad;
    QPen mPen;
    QBrush mBrush;
    QColor mColor;
    QFont mFont;
    double mSize;
    MarkerStyle mStyle;
    double mGraphKey;
    bool mInterpolating;
    bool mXLog; // Indicate the x axis is log scale, used to correctly interpolate
    bool mYLog; // Indicate that the y axis is log scale
    // Marker position
    QPointF mPos;

};

#endif // IPCMARKER_H
