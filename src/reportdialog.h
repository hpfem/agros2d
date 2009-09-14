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

    void generateIndex();
    QString replaceTemplates(const QString &source);

private slots:
    void doAccept();
    void doOpen();
};

#endif // REPORTDIALOG_H
