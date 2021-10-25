#ifndef IPCSCOPE_H
#define IPCSCOPE_H

#include <QObject>
#include <QtCharts>
#include <QList>
#include <QPalette>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QtPrintSupport>
#include "ipcrange.h"
#include "ipcmarker.h"
#include "ipcmarkertable.h"

using namespace QtCharts;

enum ScopeType { stpLinear     /// Both X and Y axes are linear
                ,stpSemiLogX   /// X axis is log type
                ,stpSemiLogY   /// Y axis is log type
                ,stpLogLog     /// Both X and Y axes are log type
               };

class IPCScope : public QGraphicsView
{
    Q_OBJECT
public:
    explicit IPCScope(QWidget *parent = nullptr, ScopeType scopeType = stpLinear);
    virtual ~IPCScope();

    enum TraceMode {
         ClearWrite=1   /// Clear write mode
        ,MaxHold=2      /// Max hold mode
        ,MinHold=3      /// Min hold mode
        ,Average=4      /// Average mode
    };
    Q_ENUMS(TraceMode)

    enum ScopeTheme { stLight     /// Light theme
                     ,stDark      /// Dark theme
                   };
    Q_ENUMS(ScopeTheme)

    enum LineStyle { lsScatter    /// Scatter style, no line between points
                    ,lsLine       /// Line style
                    ,lsArea       /// Area style, with a brush covering all the field under the graph

                   };
    Q_ENUMS(LineStyle)

    enum ZoomDirection{  zdNone             /// No zoom
                        ,zdVertical         /// Zoom vertical
                        ,zdHorizontal       /// Zoom horizontal
                        ,zdBothDirections = zdVertical | zdHorizontal /// Zoom both directions
                       };
    Q_ENUMS(ZoomDirection)

    enum LegendPosition{ lpTopLeft      /// Top left of the scope
                        ,lpTopRight     /// Top right of the scope
                        ,lpBottomLeft   /// Bottom left of the scope
                        ,lpBottomRight  /// Bottom right of the scope
                       };
    Q_ENUMS(LegendPosition)

    enum MarkerTablePosition{ mpTopLeft      /// Top left of the scope
                             ,mpTopMidle     /// Top midle of the scope
                             ,mpTopRight     /// Top right of the scope
                       };
    Q_ENUMS(MarkerTablePosition)

    // A scope has a chart and a chartview
    QChart *mChart;

    // Setters
    void setName(const QString &name){mScopeName = name;}
    void setOpenGLEnabled(bool enable);

    // Methods concerning the graphs
    void setActiveGraphIdx(int graphIdx);
    void addGraph(QString graphName = "No name", LineStyle lineStyle = lsLine);
    QAbstractSeries * const &graph(int graphIdx) const;
    QAbstractSeries * const &graph() const;
    void clearGraph(int graphIdx);
    int clearGraphs();
    void setGraphVisible(int graphIdx, bool visible);
    void setGraphVisible(bool visible);
    void setGraphName(int graphIdx, QString name);
    void setGraphName(QString name);
    void setGraphColor(int graphIdx, const QColor &color);
    void setGraphColor(const QColor &color);
    // Graph data update
    void setGraphData(int graphIdx, QVector<QPointF> points);
    void setGraphData(QVector<QPointF> points);
    void setGraphData(int graphIdx, double *x, double *y, int len);
    void setGraphData(double *x, double *y, int len);
    void setGraphData(QString name, QVector<QPointF> points);
    void setGraphData(QString name, double *x, double *y, int len);

    // Methods concerning zoom
    void setZoomDirection(const ZoomDirection &dir){mZoomDirection = dir;}
    void setZoomWeight(double weight){mZoomWeight = weight;}
    void setZoomRange(qreal xp1, qreal yp1, qreal xp2, qreal yp2);
    void setZoomRange(QPointF topLeft, QPointF bottomRight);
    void setZoomRange(QRectF boundingRect);
    void setZoomFit();

    // Save and load
    bool saveGraph(int graphIdx, const QString &fileName);
//    virtual void loadGraph() = 0;

    // Graph markers
    void addMarker();
    void clearMarker(int markerIdx);
    void clearMarkers();
    void setActiveMarkerIdx(int markerIdx){mActiveMarkerIdx = markerIdx;}
    void setMarkerKeyValue(int markerIdx, double val);
    void setMarkerKeyValue(double val);
    void setMarkerColor(int markerIdx, const QColor &color);
    void setMarkerColor(const QColor &color);
    void setMarkersColor(const QColor &color);
    void setMarkerFont(int markerIdx, const QFont &font);
    void setMarkerFont(const QFont &font);
    void setMarkersFont(const QFont &font);
    void setMarkerTableVisible(bool visible){mMarkerTableVisible = visible; mMarkerTable->setVisible(visible);}
    void setMarkerTablePosition(MarkerTablePosition pos);

    // Legend
    void setLegendVisible(bool visible){mLegendVisible = visible; mChart->legend()->setVisible(visible);}
    void setLegendBorderPen(const QPen &pen);
    void setLegendBrush(const QBrush &brush);
    void setLegendLabelColor(const QColor &color);
    void setLegendFont(const QFont &font);
    void setLegendPosition(LegendPosition pos);

    // Theme
    void setScopeThemeDark();
    void setScopeThemeLight();
    void setScopeTheme(ScopeTheme theme);

    // Printing to pdf file, image, etc.
    QPixmap toPixmap(int width, int height, double scale);
    void savePdf(const QString &fileName, int width, int height, const QString &pdfCreator, const QString &pdfTitle);
    bool savePng(const QString &fileName, int width=0, int height=0, double scale=1.0, int quality=-1, int dotPerInch=96);

    // Getters
    QString name() const {return mScopeName;}
    bool openGLEnabled() const {return mOpenGLEnabled;}
    int activeGraphIdx() const {return mActiveGraphIdx;}
    QStringList graphsNameList() const;
    int graphCount() const {return mGraphsList.length();}
    int activeMarkerIdx() const {return mActiveMarkerIdx;}
    QList<QAbstractAxis *> const & axes() const {return mAxesList;}
    QAbstractAxis * const &xAxis(){return mAxesList.at(0);}
    QAbstractAxis * const &yAxis(){return mAxesList.at(1);}
    QList<IPCMarker *> const & markers() const {return mMarkerList;}
    IPCMarker * const & marker (int markerIdx) const {return mMarkerList.at(markerIdx);}
    int markerCount(){return mMarkerList.length();}
    ZoomDirection zoomDirection() const{return mZoomDirection;}
    double zoomWeight() const{return mZoomWeight;}
    IPCMarkerTable * const & markerTable() const {return mMarkerTable;}
    QLegend * legend(){return mChart->legend();}    

signals:

protected:
    int getMinorTicks(double tickInterval);
    double getMantissa(double input, double *magnitude) const;
    double cleanMantissa(double input) const;
    void updateLegendPosition();
    void updateMarkerTablePosition();
    void cosmeticTicksInterval();
    void updateGeometry();
    void resizeEvent(QResizeEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
private:
    // Name
    QString mScopeName;
    // Default font used throughout the scope
    QFont mBaseFont;
    QFont mMarkerFont;
    // Scope type
    ScopeType mScopeType;
    // Scope theme
    ScopeTheme mScopeTheme;
    // Hardware acceleration
    bool mOpenGLEnabled;
    // Active graph
    int mActiveGraphIdx;
    // Active marker
    int mActiveMarkerIdx;
    // A scope has a list of graphs. A graph is a QAbstractSeries
    QList<QAbstractSeries *> mGraphsList;
    // A scope has a list of markers
    QList<IPCMarker *> mMarkerList;
    // A scope has a marker table
    IPCMarkerTable *mMarkerTable;
    MarkerTablePosition mMarkerTablePos;
    bool mMarkerTableVisible;
    QColor mMarkerColor;
    // A scope has a list of axes
    QList<QAbstractAxis *> mAxesList;
    // List of predefined colors for graphs
    QStringList mGraphColors;
    // Direction of zoom
    ZoomDirection mZoomDirection;
    double mZoomWeight;
    // Zoom range
    IPCRange mZoomRangeX;
    IPCRange mZoomRangeY;
    // Zoom rubber band
    QRubberBand *mRubberBand;
    QPoint mRubberBandOrigin;
    // Legend
    LegendPosition mLegendPos;
    bool mLegendVisible;

};

#endif // IPCSCOPE_H
