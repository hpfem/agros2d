#ifndef CHARTDIALOG_H
#define CHARTDIALOG_H

#include "scene.h"
#include "util.h"
#include "gui.h"
#include "localvalueview.h"

class Chart;

class ChartDialog : public QDialog
{
    Q_OBJECT

signals:
    void setChartLine(const Point &start, const Point &end);

public slots:
    void doPlot();

public:
    ChartDialog(QWidget *parent = 0);
    ~ChartDialog();

    void showDialog();

protected:
    void hideEvent(QHideEvent *event);

private:
    QTabWidget* tabOutput;
    QTabWidget* tabAnalysisType;

    QwtPlotPicker *picker;

    // geometry
    QComboBox *cmbTimeStep;

    QLabel *lblStartX;
    QLabel *lblStartY;
    QLabel *lblEndX;
    QLabel *lblEndY;

    SLineEditDouble *txtStartX;
    SLineEditDouble *txtStartY;
    SLineEditDouble *txtEndX;
    SLineEditDouble *txtEndY;

    QRadioButton *radAxisLength;
    QRadioButton *radAxisX;
    QRadioButton *radAxisY;

    QSpinBox *txtAxisPoints;

    // time
    QLabel *lblPointX;
    QLabel *lblPointY;
    SLineEditDouble *txtPointX;
    SLineEditDouble *txtPointY;

    QComboBox *cmbFieldVariable;
    QComboBox *cmbFieldVariableComp;

    QWidget *widGeometry;
    QWidget *widTime;

    Chart *chart;
    QTableWidget *trvTable;

    void createControls();

    void plotGeometry();
    void plotTime();

private slots:
    void doFieldVariable(int index);
    void doFieldVariableComp(int index);
    void doSaveImage();
    void doExportData();
    void doMoved(const QPoint &);
    void doChartLine();
    void doTimeStepChanged(int index);
};

#endif // CHARTDIALOG_H
