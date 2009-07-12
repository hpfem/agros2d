#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include "util.h"

#include <QtGui/QTextBrowser>
#include <QtGui/QSplitter>
#include <QtGui/QHBoxLayout>
#include <QtGui/QTabWidget>
#include <QAction>
#include <QWebView>

class CentralWidget;

class HelpDialog : public QDialog
{
    Q_OBJECT

public:
    HelpDialog(QWidget *parent = 0);
    ~HelpDialog();

    void setSource(const QUrl &url);

protected:
    void createControls();

private:
    QSplitter *splitter;
    CentralWidget *centralWidget;
};

// ***********************************************************************************************************

class HelpViewer : public QWebView
{
    Q_OBJECT

public:
    HelpViewer(QHelpEngine *helpEngine, CentralWidget *parent);
    void setSource(const QUrl &url);

    inline QUrl source() const
    { return url(); }

    inline QString documentTitle() const
    { return title(); }

    inline bool hasSelection() const
    { return !selectedText().isEmpty(); } // ### this is suboptimal

    void zoomIn(int range = 1);
    void zoomOut(int range = 1);

    void resetZoom();
    int zoom() const;
    void setZoom(int zoom) { zoomIn(zoom); }

    inline void copy()
    { return triggerPageAction(QWebPage::Copy); }

    inline bool isForwardAvailable() const
    { return pageAction(QWebPage::Forward)->isEnabled(); }
    inline bool isBackwardAvailable() const
    { return pageAction(QWebPage::Back)->isEnabled(); }

public Q_SLOTS:
    void home();
    void backward() { back(); }

Q_SIGNALS:
    void copyAvailable(bool enabled);
    void forwardAvailable(bool enabled);
    void backwardAvailable(bool enabled);
    void highlighted(const QString &);
    void sourceChanged(const QUrl &);

protected:
    virtual void wheelEvent(QWheelEvent *);
    void mouseReleaseEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *event);

private Q_SLOTS:
    void actionChanged();

private:
    QHelpEngine *helpEngine;
    CentralWidget* parentWidget;
    QUrl homeUrl;
    bool multiTabsAllowed;
};

// ***********************************************************************************************************

class CentralWidget : public QWidget
{
    Q_OBJECT

public:
    CentralWidget(QHelpEngine *engine, QWidget *parent = 0);
    ~CentralWidget();

    bool hasSelection() const;
    QUrl currentSource() const;
    QString currentTitle() const;
    bool isHomeAvailable() const;
    bool isForwardAvailable() const;
    bool isBackwardAvailable() const;
    QList<QAction*> globalActions() const;
    void setGlobalActions(const QList<QAction*> &actions);
    HelpViewer *currentHelpViewer() const;
    void activateTab(bool onlyHelpViewer = false);
    bool find(const QString &txt, QTextDocument::FindFlags findFlags, bool incremental);
    void setLastShownPages();
    HelpViewer *helpViewerAtIndex(int index) const;

    static CentralWidget *instance();

public slots:
    void zoomIn();
    void zoomOut();
    void nextPage();
    void resetZoom();
    void previousPage();
    void copySelection();
    void print();
    void pageSetup();
    void printPreview();
    void setSource(const QUrl &url);
    void setSourceInNewTab(const QUrl &url, int zoom = 0);
    HelpViewer *newEmptyTab();
    void home();
    void forward();
    void backward();
    void copy();

protected:
    void focusInEvent(QFocusEvent *event);

signals:
    void currentViewerChanged();
    void copyAvailable(bool yes);
    void sourceChanged(const QUrl &url);
    void highlighted(const QString &link);
    void forwardAvailable(bool available);
    void backwardAvailable(bool available);
    void addNewBookmark(const QString &title, const QString &url);

private slots:
    void newTab();
    void closeTab();
    void closeTab(int index);
    void setTabTitle(const QUrl& url);
    void currentPageChanged(int index);
    void showTabBarContextMenu(const QPoint &point);
    void printPreview(QPrinter *printer);

private:
    void connectSignals();
    bool eventFilter(QObject *object, QEvent *e);
    void initPrinter();
    QString quoteTabTitle(const QString &title) const;

private:
    int lastTabPage;
    QString collectionFile;
    QList<QAction*> globalActionList;

    QWidget *findBar;
    QTabWidget *tabWidget;
    QHelpEngine *helpEngine;
    QPrinter *printer;
};

#endif // HELPDIALOG_H
