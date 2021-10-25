#include <QCoreApplication>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include "ipcscope.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    IPCScope *scope = new IPCScope(nullptr, ScopeType::stpSemiLogX);

    scope->addGraph("Spurious", IPCScope::lsLine);
    scope->addGraph("Noise Spectrum", IPCScope::lsLine);
    scope->addGraph("Residual", IPCScope::lsArea);

    scope->markerTable()->setKeyDisplayType(IPCMarkerTable::kdFrequency);
    scope->markerTable()->setYText("dBc/Hz");

    QVector<QPointF> points;
    QVector<QPointF> points2;
    QVector<QPointF> points3;

    points << QPointF(1,-110) << QPointF(10,-140) << QPointF(100,-170) << QPointF(1000,-180) << QPointF(10000,-180) << QPointF(100000,-180) << QPointF(1000000,-180);
    points2 << QPointF(1,-130) << QPointF(10,-160) << QPointF(100,-190) << QPointF(1000,-200) << QPointF(10000,-200) << QPointF(100000,-200) << QPointF(1000000,-200);
    points3 << QPointF(1,-110) << QPointF(10,-140) << QPointF(100,-160) << QPointF(1000,-180) << QPointF(10000,-180) << QPointF(100000,-180) << QPointF(1000000,-180);

    scope->setGraphData(0,points3);
    scope->setGraphData(1,points);
    scope->setGraphData(2,points2);
    scope->addMarker();
    scope->addMarker();
    scope->addMarker();
    scope->addMarker();
    scope->setActiveGraphIdx(1);

    scope->show();

    scope->setMarkerKeyValue(0, 1.1);
    scope->setMarkerKeyValue(1, 1.5);
    scope->setMarkerKeyValue(2, 15);
    scope->setMarkerKeyValue(3, 1000000);
    scope->setZoomFit();
    scope->setGraphColor(0, QColor(Qt::magenta));
    scope->setGraphColor(1, QColor(Qt::yellow));
    scope->setGraphColor(2, QColor(204,204,204,80));

    scope->setZoomRange(1, -70, 1000000, -210);

    scope->setScopeTheme(IPCScope::stDark);

    scope->xAxis()->setTitleText("Offset Hz");
    scope->yAxis()->setTitleText("L(fm) dBc/Hz");
    scope->setGraphVisible(0, false);
    scope->setGraphVisible(0, true);

    scope->resize(1024,760);
    scope->setLegendVisible(true);

    return a.exec();
}
