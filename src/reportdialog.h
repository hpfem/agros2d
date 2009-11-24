#ifndef REPORTDIALOG_H
#define REPORTDIALOG_H

#include "util.h"
#include "sceneview.h"

#include <QWebView>

class ReportDialog : public QDialog
{
    Q_OBJECT

public:
    ReportDialog(SceneView *sceneView, QWidget *parent = 0);
    ~ReportDialog();

    void showDialog();

private:
    SceneView *m_sceneView;
    QWebView *view;

    void createControls();

    QPushButton *btnClose;
    QPushButton *btnOpenInExternalBrowser;
    QPushButton *btnPrint;

    void generateFigures();
    void generateIndex();
    QString replaceTemplates(const QString &source);

    QString htmlMaterials();
    QString htmlBoundaries();
    QString htmlGeometryNodes();
    QString htmlGeometryEdges();
    QString htmlGeometryLabels();

    QString htmlFigure(const QString &fileName, const QString &caption);

private slots:
    void doClose();
    void doOpenInExternalBrowser();
    void doPrint();
};

#endif // REPORTDIALOG_H
