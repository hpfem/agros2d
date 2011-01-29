// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include "util.h"

class CentralWidget;
class TopicChooser;

class HelpDialog : public QDialog
{
    Q_OBJECT

public:
    HelpDialog(QWidget *parent = 0);
    ~HelpDialog();

    void showPage(const QString &str);

protected:
    void createControls();

private:
    QHelpEngine *helpEngine;
    QSplitter *splitter;
    CentralWidget *centralWidget;
    TopicChooser *topicChooser;

private slots:
    void doExpandTOC();
};

// ***********************************************************************************************************

class HelpViewer : public QWebView
{
    Q_OBJECT

public:
    HelpViewer(QHelpEngine *helpEngine, CentralWidget *central, QWidget *parent);
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
    inline bool hasLoadFinished() const
    { return loadFinished; }

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
    void keyPressEvent(QKeyEvent *e);

    void wheelEvent(QWheelEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *event);

private Q_SLOTS:
    void actionChanged();
    void setLoadFinished(bool ok);

private:
    QHelpEngine *helpEngine;
    CentralWidget* parentWidget;
    QUrl homeUrl;
    bool multiTabsAllowed;
    bool loadFinished;
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
    void tabNew();
    void tabClose();
    void tabClose(int index);
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
    QTabWidget* tabWidget;
    QHelpEngine *helpEngine;
    QPrinter *printer;

    QToolButton *btnTabGoBack;
    QToolButton *btnTabGoForward;
};

// ************************************************************************************************

class TopicChooser : public QDialog
{
    Q_OBJECT

public:
    TopicChooser(QWidget *parent = 0);

signals:
    void linkActivated(const QUrl &link);

public slots:
    void doLinksActivated(const QMap<QString, QUrl> &links, const QString &keyword);

protected:
    void createControls();

private slots:
    void doAccept();
    void doReject();    

private:
    QListWidget *lstView;
    QMap<QString, QUrl> m_links;
};

#endif // HELPDIALOG_H
