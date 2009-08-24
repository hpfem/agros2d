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
    QTabWidget* tabWidget;

    QwtPlotPicker *picker;

    QLabel *lblStartX;
    QLabel *lblStartY;
    QLabel *lblEndX;
    QLabel *lblEndY;

    SLineEdit *txtStartX;
    SLineEdit *txtStartY;
    SLineEdit *txtEndX;
    SLineEdit *txtEndY;

    QRadioButton *radAxisLength;
    QRadioButton *radAxisX;
    QRadioButton *radAxisY;
    QSpinBox *txtAxisPoints;

    QComboBox *cmbFieldVariable;
    QComboBox *cmbFieldVariableComp;

    Chart *chart;
    QTableWidget *trvTable;

    void createControls();

private slots:
    void doFieldVariable(int index);
    void doPrint();
    void doSaveImage();
    void doExportData();
    void doMoved(const QPoint &);
    void doChartLine();
};

#endif // CHARTDIALOG_H
