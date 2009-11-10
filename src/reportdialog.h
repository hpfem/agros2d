#ifndef REPORTDIALOG_H
#define REPORTDIALOG_H

#include "util.h"

#include <QWebView>

class ReportDialog : public QDialog
{
    Q_OBJECT

public:
    ReportDialog(QWidget *parent = 0);
    ~ReportDialog();

    void showDialog();

private:
    QWebView *view;

    void createControls();

    QPushButton *btnClose;
    QPushButton *btnOpenInExternalBrowser;
    QPushButton *btnPrint;

    void generateIndex();
    QString replaceTemplates(const QString &source);

    QString htmlMaterials();
    QString htmlBoundaries();
    QString htmlGeometryNodes();
    QString htmlGeometryEdges();
    QString htmlGeometryLabels();

private slots:
    void doClose();
    void doOpenInExternalBrowser();
    void doPrint();
};

#endif // REPORTDIALOG_H
