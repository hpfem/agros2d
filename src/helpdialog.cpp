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

#include "helpdialog.h"

#include <QtHelp/QHelpEngine>
#include <QtHelp/QHelpContentWidget>
#include <QtHelp/QHelpIndexWidget>

const QString path = "Agros2D.org.Agros2D.1/doc/";

HelpDialog::HelpDialog(QWidget *parent) : QDialog(parent)
{
    setWindowIcon(icon("help-contents"));
    setWindowTitle(tr("Help dialog"));
    setWindowFlags(Qt::Window);

    topicChooser = new TopicChooser(this);

    createControls();

    resize(900, 600);

    QSettings settings;
    restoreGeometry(settings.value("HelpDialog/Geometry", saveGeometry()).toByteArray());
    splitter->restoreGeometry(settings.value("HelpDialog/SplitterGeometry", splitter->saveGeometry()).toByteArray());
    splitter->restoreState(settings.value("HelpDialog/SplitterState", splitter->saveState()).toByteArray());  
}

HelpDialog::~HelpDialog()
{
    QSettings settings;
    settings.setValue("HelpDialog/Geometry", saveGeometry());
    settings.setValue("HelpDialog/SplitterGeometry", splitter->saveGeometry());
    settings.setValue("HelpDialog/SplitterState", splitter->saveState());
}

void HelpDialog::createControls()
{
    helpEngine = new QHelpEngine(datadir() + "/doc/help/Agros2D.qhc", this);
    helpEngine->setupData();
    helpEngine->setCustomValue("HomePage", "qthelp://" + path + "index.html");

    splitter = new QSplitter(Qt::Horizontal);
    centralWidget = new CentralWidget(helpEngine, this);

    // index
    QLineEdit *txtIndexFilter = new QLineEdit("", this);

    QHBoxLayout *layoutFilter = new QHBoxLayout();
    layoutFilter->addWidget(new QLabel(tr("Filter:")));
    layoutFilter->addWidget(txtIndexFilter);

    QVBoxLayout *layoutIndex = new QVBoxLayout();
    layoutIndex->addLayout(layoutFilter);
    layoutIndex->addWidget(helpEngine->indexWidget());

    QWidget *widIndex = new QWidget();
    widIndex->setLayout(layoutIndex);

    connect(txtIndexFilter, SIGNAL(textChanged(QString)), helpEngine->indexWidget(), SLOT(filterIndices(QString)));

    // tab
    QTabWidget *tabLeft = new QTabWidget(this);
    tabLeft->setMinimumWidth(250);
    tabLeft->addTab(helpEngine->contentWidget(), icon(""), tr("Content"));
    tabLeft->addTab(widIndex, icon(""), tr("Index"));
    tabLeft->setMinimumWidth(150);
    tabLeft->setMaximumWidth(220);

    splitter->insertWidget(0, tabLeft);
    splitter->insertWidget(1, centralWidget);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(splitter);

    connect(helpEngine->contentWidget(), SIGNAL(linkActivated(const QUrl &)), centralWidget, SLOT(setSource(const QUrl &)));
    connect(helpEngine->indexWidget(), SIGNAL(linkActivated(const QUrl &, const QString &)), centralWidget, SLOT(setSource(const QUrl &)));
    connect(helpEngine->indexWidget(), SIGNAL(linksActivated(const QMap<QString, QUrl> &, const QString &)), topicChooser, SLOT(doLinksActivated(QMap<QString, QUrl>, QString)));
    connect(topicChooser, SIGNAL(linkActivated(const QUrl &)), centralWidget, SLOT(setSource(const QUrl &)));

    QHelpContentModel *contentModel = qobject_cast<QHelpContentModel*>(helpEngine->contentWidget()->model());
    connect(contentModel, SIGNAL(contentsCreated()), this, SLOT(doExpandTOC()));

    showPage("glossary.html");
}

void HelpDialog::showPage(const QString &str)
{
    centralWidget->setSource(QUrl("qthelp://" + path + str));
}

void HelpDialog::doExpandTOC()
{
    helpEngine->contentWidget()->expandToDepth(1);
}

// ***********************************************************************************************************

class HelpNetworkReply : public QNetworkReply
{
public:
    HelpNetworkReply(const QNetworkRequest &request, const QByteArray &fileData,
                     const QString &mimeType);

    virtual void abort();

    virtual qint64 bytesAvailable() const
    { return data.length() + QNetworkReply::bytesAvailable(); }

protected:
    virtual qint64 readData(char *data, qint64 maxlen);

private:
    QByteArray data;
    qint64 origLen;
};

HelpNetworkReply::HelpNetworkReply(const QNetworkRequest &request,
                                   const QByteArray &fileData, const QString &mimeType)
                                       : data(fileData), origLen(fileData.length())
{
    setRequest(request);
    setOpenMode(QIODevice::ReadOnly);

    setHeader(QNetworkRequest::ContentTypeHeader, mimeType);
    setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(origLen));
    QTimer::singleShot(0, this, SIGNAL(metaDataChanged()));
    QTimer::singleShot(0, this, SIGNAL(readyRead()));
}

void HelpNetworkReply::abort()
{
    // nothing to do
}

qint64 HelpNetworkReply::readData(char *buffer, qint64 maxlen)
{
    qint64 len = qMin(qint64(data.length()), maxlen);
    if (len)
    {
        qMemCopy(buffer, data.constData(), len);
        data.remove(0, len);
    }
    if (!data.length())
        QTimer::singleShot(0, this, SIGNAL(finished()));
    return len;
}

class HelpNetworkAccessManager : public QNetworkAccessManager
{
public:
    HelpNetworkAccessManager(QHelpEngine *engine, QObject *parent);

protected:
    virtual QNetworkReply *createRequest(Operation op,
                                         const QNetworkRequest &request, QIODevice *outgoingData = 0);

private:
    QHelpEngine *helpEngine;
};

HelpNetworkAccessManager::HelpNetworkAccessManager(QHelpEngine *engine,
                                                   QObject *parent)
                                                       : QNetworkAccessManager(parent), helpEngine(engine)
{
}

QNetworkReply *HelpNetworkAccessManager::createRequest(Operation op,
                                                       const QNetworkRequest &request, QIODevice *outgoingData)
{
    const QString& scheme = request.url().scheme();
    if (scheme == QLatin1String("qthelp") || scheme == QLatin1String("about"))
    {
        const QUrl& url = request.url();
        QString mimeType = url.toString();
        if (mimeType.endsWith(QLatin1String(".svg")) || mimeType.endsWith(QLatin1String(".svgz")))
        {
            mimeType = QLatin1String("image/svg+xml");
        }
        else if (mimeType.endsWith(QLatin1String(".css")))
        {
            mimeType = QLatin1String("text/css");
        }
        else if (mimeType.endsWith(QLatin1String(".js")))
        {
            mimeType = QLatin1String("text/javascript");
        }
        else
        {
            mimeType = QLatin1String("text/html");
        }
        return new HelpNetworkReply(request, helpEngine->fileData(url), mimeType);
    }
    return QNetworkAccessManager::createRequest(op, request, outgoingData);
}

class HelpPage : public QWebPage
{
    friend class HelpViewer;
public:
    HelpPage(CentralWidget *central, QHelpEngine *engine, QObject *parent);

protected:
    virtual QWebPage *createWindow(QWebPage::WebWindowType);

    virtual bool acceptNavigationRequest(QWebFrame *frame,
                                         const QNetworkRequest &request, NavigationType type);

private:
    CentralWidget *centralWidget;
    QHelpEngine *helpEngine;
    Qt::MouseButtons m_pressedButtons;
    Qt::KeyboardModifiers m_keyboardModifiers;
};

HelpPage::HelpPage(CentralWidget *central, QHelpEngine *engine, QObject *parent)
    : QWebPage(parent)
    , centralWidget(central)
    , helpEngine(engine)
    , m_pressedButtons(Qt::NoButton)
    , m_keyboardModifiers(Qt::NoModifier)
{
}

QWebPage *HelpPage::createWindow(QWebPage::WebWindowType)
{
    return centralWidget->newEmptyTab()->page();
}

static bool isLocalUrl(const QUrl &url)
{
    const QString scheme = url.scheme();
    if (scheme.isEmpty()
        || scheme == QLatin1String("file")
        || scheme == QLatin1String("qrc")
        || scheme == QLatin1String("data")
        || scheme == QLatin1String("qthelp")
        || scheme == QLatin1String("about"))
        return true;
    return false;
}

bool HelpPage::acceptNavigationRequest(QWebFrame *, const QNetworkRequest &request, QWebPage::NavigationType type)
{
    const QUrl &url = request.url();
    if (isLocalUrl(url))
    {
        const QString& path = url.path();
        if (path.endsWith(QLatin1String(".pdf")))
        {
            const int lastDash = path.lastIndexOf(QChar('/'));
            QString fileName = QDir::tempPath() + QDir::separator();
            if (lastDash < 0)
                fileName += path;
            else
                fileName += path.mid(lastDash + 1, path.length());

            QFile tmpFile(QDir::cleanPath(fileName));
            if (tmpFile.open(QIODevice::ReadWrite))
            {
                tmpFile.write(helpEngine->fileData(url));
                tmpFile.close();
            }
            QDesktopServices::openUrl(QUrl(tmpFile.fileName()));
            return false;
        }

        if (type == QWebPage::NavigationTypeLinkClicked
            && (m_keyboardModifiers & Qt::ControlModifier || m_pressedButtons == Qt::MidButton))
        {
            HelpViewer* viewer = centralWidget->newEmptyTab();
            if (viewer)
                centralWidget->setSource(url);
            m_pressedButtons = Qt::NoButton;
            m_keyboardModifiers = Qt::NoModifier;
            return false;
        }
        return true;
    }

    QDesktopServices::openUrl(url);
    return false;
}

HelpViewer::HelpViewer(QHelpEngine *engine, CentralWidget *central, QWidget *parent)
    : QWebView(parent)
    , helpEngine(engine)
    , parentWidget(central)
    , multiTabsAllowed(true)
    , loadFinished(false)
{
    setPage(new HelpPage(central, helpEngine, this));
    settings()->setAttribute(QWebSettings::PluginsEnabled, false);
    settings()->setAttribute(QWebSettings::JavaEnabled, false);

    page()->setNetworkAccessManager(new HelpNetworkAccessManager(engine, this));

    QAction* action = pageAction(QWebPage::OpenLinkInNewWindow);
    action->setText(tr("Open Link in New Tab"));
    if (!central)
    {
        multiTabsAllowed = false;
        action->setVisible(false);
    }

    pageAction(QWebPage::DownloadLinkToDisk)->setVisible(false);
    pageAction(QWebPage::DownloadImageToDisk)->setVisible(false);
    pageAction(QWebPage::OpenImageInNewWindow)->setVisible(false);

    connect(pageAction(QWebPage::Copy), SIGNAL(changed()), this, SLOT(actionChanged()));
    connect(pageAction(QWebPage::Back), SIGNAL(changed()), this, SLOT(actionChanged()));
    connect(pageAction(QWebPage::Forward), SIGNAL(changed()), this, SLOT(actionChanged()));
    connect(page(), SIGNAL(linkHovered(QString, QString, QString)), this, SIGNAL(highlighted(QString)));
    connect(this, SIGNAL(urlChanged(QUrl)), this, SIGNAL(sourceChanged(QUrl)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(setLoadFinished(bool)));

    setAcceptDrops(false);
}

void HelpViewer::setSource(const QUrl &url)
{
    if ((!url.isEmpty()) && (this->url().toString() == url.toString()))
        return;

    loadFinished = false;
    if (!homeUrl.isValid())
        homeUrl = url;
    load(url);
}

void HelpViewer::resetZoom()
{
    setTextSizeMultiplier(1.0);
}

void HelpViewer::zoomIn(int range)
{
    setTextSizeMultiplier(qMin(2.0, textSizeMultiplier() + range / 10.0));
}

void HelpViewer::zoomOut(int range)
{
    setTextSizeMultiplier(qMax(0.5, textSizeMultiplier() - range / 10.0));
}

int HelpViewer::zoom() const
{
    qreal zoom = textSizeMultiplier() * 10.0;
    return (zoom < 10.0 ? zoom * -1.0 : zoom - 10.0);
}

void HelpViewer::home()
{
    QString homepage = helpEngine->customValue(QLatin1String("HomePage"),
                                               QLatin1String("")).toString();

    if (homepage.isEmpty()) {
        homepage = helpEngine->customValue(QLatin1String("DefaultHomePage"),
                                           QLatin1String("about:blank")).toString();
    }

    setSource(homepage);
}

void HelpViewer::keyPressEvent(QKeyEvent *e)
{
    if ((e->key() == Qt::Key_C) && (e->modifiers() & Qt::ControlModifier))
    {
        if (hasSelection())
            copy();
    }

    QWebView::keyPressEvent(e);
}

void HelpViewer::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier)
    {
        const int delta = e->delta();
        if (delta > 0)
            zoomIn(delta / 120);
        else if (delta < 0)
            zoomOut(-delta / 120);
        e->accept();
        return;
    }
    QWebView::wheelEvent(e);
}

void HelpViewer::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::XButton1)
    {
        triggerPageAction(QWebPage::Back);
        return;
    }

    if (e->button() == Qt::XButton2)
    {
        triggerPageAction(QWebPage::Forward);
        return;
    }

    QWebView::mouseReleaseEvent(e);
}

void HelpViewer::actionChanged()
{
    QAction *a = qobject_cast<QAction *>(sender());
    if (a == pageAction(QWebPage::Copy))
        emit copyAvailable(a->isEnabled());
    else if (a == pageAction(QWebPage::Back))
        emit backwardAvailable(a->isEnabled());
    else if (a == pageAction(QWebPage::Forward))
        emit forwardAvailable(a->isEnabled());
}

void HelpViewer::mousePressEvent(QMouseEvent *event)
{
    HelpPage *currentPage = static_cast<HelpPage*>(page());
    if ((currentPage != 0) && multiTabsAllowed)
    {
        currentPage->m_pressedButtons = event->buttons();
        currentPage->m_keyboardModifiers = event->modifiers();
    }
    QWebView::mousePressEvent(event);
}

void HelpViewer::setLoadFinished(bool ok)
{
    loadFinished = ok;
    emit sourceChanged(url());
}

// ***********************************************************************************************************

HelpViewer* helpViewerFromTabPosition(const QTabWidget *widget,
                                      const QPoint &point)
{
    QTabBar *tabBar = qFindChild<QTabBar*>(widget);
    for (int i = 0; i < tabBar->count(); ++i)
    {
        if (tabBar->tabRect(i).contains(point))
            return qobject_cast<HelpViewer*>(widget->widget(i));
    }
    return 0;
}
CentralWidget *staticCentralWidget = 0;

CentralWidget::CentralWidget(QHelpEngine *engine, QWidget *parent)
    : QWidget(parent)
    , findBar(0)
    , tabWidget(0)
    , helpEngine(engine)
    , printer(0)
{
    lastTabPage = 0;
    globalActionList.clear();
    collectionFile = helpEngine->collectionFile();

    tabWidget = new QTabWidget;
    tabWidget->setDocumentMode(true);
    tabWidget->setMovable(true);
    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(tabClose(int)));
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentPageChanged(int)));

    QToolButton *btnTabNew = new QToolButton(this);
    btnTabNew->setAutoRaise(true);
    btnTabNew->setToolTip(tr("Add new page"));
    btnTabNew->setIcon(icon("tabadd"));
    connect(btnTabNew, SIGNAL(clicked()), this, SLOT(tabNew()));

    btnTabGoBack = new QToolButton(this);
    btnTabGoBack->setAutoRaise(true);
    btnTabGoBack->setToolTip(tr("Go back"));
    btnTabGoBack->setIcon(icon("tabback"));
    btnTabGoBack->setEnabled(false);
    connect(btnTabGoBack, SIGNAL(clicked()), this, SLOT(backward()));
    connect(this, SIGNAL(backwardAvailable(bool)), btnTabGoBack, SLOT(setEnabled(bool)));

    btnTabGoForward = new QToolButton(this);
    btnTabGoForward->setAutoRaise(true);
    btnTabGoForward->setToolTip(tr("Go forward"));
    btnTabGoForward->setIcon(icon("tabforward"));
    btnTabGoForward->setEnabled(false);
    connect(btnTabGoForward, SIGNAL(clicked()), this, SLOT(forward()));
    connect(this, SIGNAL(forwardAvailable(bool)), btnTabGoForward, SLOT(setEnabled(bool)));

    QToolButton *btnTabGoHome = new QToolButton(this);
    btnTabGoHome->setAutoRaise(true);
    btnTabGoHome->setToolTip(tr("Go home"));
    btnTabGoHome->setIcon(icon("tabhome"));
    connect(btnTabGoHome, SIGNAL(clicked()), this, SLOT(home()));

    // controls
    QToolBar *tlbControls = new QToolBar();
    tlbControls->addWidget(btnTabNew);
    tlbControls->addWidget(btnTabGoHome);
    tlbControls->addWidget(btnTabGoBack);
    tlbControls->addWidget(btnTabGoForward);

    tabWidget->setCornerWidget(tlbControls, Qt::TopLeftCorner);

    QVBoxLayout *vboxLayout = new QVBoxLayout(this);
    vboxLayout->setMargin(0);
    vboxLayout->addWidget(tabWidget);

    QTabBar *tabBar = qFindChild<QTabBar*>(tabWidget);
    if (tabBar)
    {
        tabBar->installEventFilter(this);
        tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tabBar, SIGNAL(customContextMenuRequested(QPoint)), this,
                SLOT(showTabBarContextMenu(QPoint)));
    }

    staticCentralWidget = this;
}

CentralWidget::~CentralWidget()
{
#ifndef QT_NO_PRINTER
    delete printer;
#endif


    QHelpEngineCore engine(collectionFile, 0);
    if (!engine.setupData())
        return;

    QString zoomCount;
    QString currentPages;
    for (int i = 0; i < tabWidget->count(); ++i)
    {
        HelpViewer *viewer = qobject_cast<HelpViewer*>(tabWidget->widget(i));
        if (viewer && viewer->source().isValid())
        {
            currentPages += (viewer->source().toString() + QLatin1Char('|'));
            zoomCount += QString::number(viewer->zoom()) + QLatin1Char('|');
        }
    }
    engine.setCustomValue(QLatin1String("LastTabPage"), lastTabPage);
    engine.setCustomValue(QLatin1String("LastShownPages"), currentPages);
    engine.setCustomValue(QLatin1String("LastShownPagesZoom"), zoomCount);
}

CentralWidget *CentralWidget::instance()
{
    return staticCentralWidget;
}

void CentralWidget::tabNew()
{
    HelpViewer *viewer = currentHelpViewer();
    if (viewer)
        setSourceInNewTab(viewer->source());
}

void CentralWidget::zoomIn()
{
    HelpViewer* viewer = currentHelpViewer();
    if (viewer)
        viewer->zoomIn();
}

void CentralWidget::zoomOut()
{
    HelpViewer* viewer = currentHelpViewer();
    if (viewer)
        viewer->zoomOut();
}

void CentralWidget::nextPage()
{
    int index = tabWidget->currentIndex() + 1;
    if (index >= tabWidget->count())
        index = 0;
    tabWidget->setCurrentIndex(index);
}

void CentralWidget::resetZoom()
{
    HelpViewer* viewer = currentHelpViewer();
    if (viewer)
        viewer->resetZoom();
}

void CentralWidget::previousPage()
{
    int index = tabWidget->currentIndex() -1;
    if (index < 0)
        index = tabWidget->count() -1;
    tabWidget->setCurrentIndex(index);
}

void CentralWidget::tabClose()
{
    tabClose(tabWidget->currentIndex());
}

void CentralWidget::tabClose(int index)
{
    HelpViewer* viewer = helpViewerAtIndex(index);
    if (!viewer || tabWidget->count() == 1)
        return;

    tabWidget->removeTab(index);
    QTimer::singleShot(0, viewer, SLOT(deleteLater()));
}

void CentralWidget::setSource(const QUrl &url)
{
    HelpViewer *viewer = currentHelpViewer();
    HelpViewer *lastViewer = qobject_cast<HelpViewer*>(tabWidget->widget(lastTabPage));

    if (!viewer && !lastViewer)
    {
        viewer = new HelpViewer(helpEngine, this, this);
        viewer->installEventFilter(this);
        lastTabPage = tabWidget->addTab(viewer, QString());
        tabWidget->setCurrentIndex(lastTabPage);
        connectSignals();
    }
    else
    {
        viewer = lastViewer;
    }

    viewer->setSource(url);
    currentPageChanged(lastTabPage);
    viewer->setFocus(Qt::OtherFocusReason);
    tabWidget->setCurrentIndex(lastTabPage);
    tabWidget->setTabText(lastTabPage, quoteTabTitle(viewer->documentTitle()));
}

void CentralWidget::setLastShownPages()
{
    QString value = helpEngine->customValue(QLatin1String("LastShownPages"), QString()).toString();
    const QStringList lastShownPageList = value.split(QLatin1Char('|'), QString::SkipEmptyParts);
    const int pageCount = lastShownPageList.count();

    QString homePage = helpEngine->customValue(QLatin1String("DefaultHomePage"), QLatin1String("about:blank")).toString();

    int option = helpEngine->customValue(QLatin1String("StartOption"), 2).toInt();
    if (option == 0 || option == 1 || pageCount <= 0)
    {
        if (option == 0)
        {
            homePage = helpEngine->customValue(QLatin1String("HomePage"),
                                               homePage).toString();
        }
        else if (option == 1)
        {
            homePage = QLatin1String("about:blank");
        }
        setSource(homePage);
        return;
    }

    value = helpEngine->customValue(QLatin1String("LastShownPagesZoom"), QString()).toString();
    QVector<QString> zoomVector = value.split(QLatin1Char('|'), QString::SkipEmptyParts).toVector();

    const int zoomCount = zoomVector.count();
    zoomVector.insert(zoomCount, pageCount - zoomCount, QLatin1String("0"));

    QVector<QString>::const_iterator zIt = zoomVector.constBegin();
    QStringList::const_iterator it = lastShownPageList.constBegin();
    for (; it != lastShownPageList.constEnd(); ++it, ++zIt)
        setSourceInNewTab((*it), (*zIt).toInt());

    int tab = helpEngine->customValue(QLatin1String("LastTabPage"), 0).toInt();
    tabWidget->setCurrentIndex(tab);
}

bool CentralWidget::hasSelection() const
{
    const HelpViewer* viewer = currentHelpViewer();
    return viewer ? viewer->hasSelection() : false;
}

QUrl CentralWidget::currentSource() const
{
    const HelpViewer* viewer = currentHelpViewer();
    if (viewer)
        return viewer->source();

    return QUrl();
}

QString CentralWidget::currentTitle() const
{
    const HelpViewer* viewer = currentHelpViewer();
    if (viewer)
        return viewer->documentTitle();

    return QString();
}

void CentralWidget::copySelection()
{
    HelpViewer* viewer = currentHelpViewer();
    if (viewer)
        viewer->copy();
}

void CentralWidget::initPrinter()
{
#ifndef QT_NO_PRINTER
    if (!printer)
        printer = new QPrinter(QPrinter::HighResolution);
#endif
}

void CentralWidget::print()
{
#ifndef QT_NO_PRINTER
    HelpViewer* viewer = currentHelpViewer();
    if (!viewer)
        return;

    initPrinter();

    QPrintDialog *dlg = new QPrintDialog(printer, this);
    dlg->addEnabledOption(QAbstractPrintDialog::PrintPageRange);
    dlg->addEnabledOption(QAbstractPrintDialog::PrintCollateCopies);
    dlg->setWindowTitle(tr("Print Document"));
    if (dlg->exec() == QDialog::Accepted)
    {
        viewer->print(printer);
    }
    delete dlg;
#endif
}

void CentralWidget::printPreview()
{
#ifndef QT_NO_PRINTER
    initPrinter();
    QPrintPreviewDialog preview(printer, this);
    connect(&preview, SIGNAL(paintRequested(QPrinter*)),
            SLOT(printPreview(QPrinter*)));
    preview.exec();
#endif
}

void CentralWidget::printPreview(QPrinter *p)
{
#ifndef QT_NO_PRINTER
    HelpViewer *viewer = currentHelpViewer();
    if (viewer)
        viewer->print(p);
#else
    Q_UNUSED(p)
#endif
        }

void CentralWidget::pageSetup()
{
#ifndef QT_NO_PRINTER
    initPrinter();
    QPageSetupDialog dlg(printer);
    dlg.exec();
#endif
}

bool CentralWidget::isHomeAvailable() const
{
    return currentHelpViewer() ? true : false;
}

void CentralWidget::home()
{
    HelpViewer* viewer = currentHelpViewer();
    if (viewer)
        viewer->home();
}

bool CentralWidget::isForwardAvailable() const
{
    const HelpViewer* viewer = currentHelpViewer();
    if (viewer)
        return viewer->isForwardAvailable();

    return false;
}

void CentralWidget::forward()
{
    HelpViewer* viewer = currentHelpViewer();
    if (viewer)
        viewer->forward();
}

bool CentralWidget::isBackwardAvailable() const
{
    const HelpViewer* viewer = currentHelpViewer();
    if (viewer)
        return viewer->isBackwardAvailable();

    return false;
}

void CentralWidget::backward()
{
    HelpViewer* viewer = currentHelpViewer();
    if (viewer)
        viewer->backward();
}


QList<QAction*> CentralWidget::globalActions() const
{
    return globalActionList;
}

void CentralWidget::setGlobalActions(const QList<QAction*> &actions)
{
    globalActionList = actions;
}

void CentralWidget::setSourceInNewTab(const QUrl &url, int zoom)
{
    HelpViewer *viewer = new HelpViewer(helpEngine, this, this);
    viewer->installEventFilter(this);
    viewer->setZoom(zoom);
    viewer->setSource(url);
    viewer->setFocus(Qt::OtherFocusReason);

    tabWidget->setCurrentIndex(tabWidget->addTab(viewer, quoteTabTitle(viewer->documentTitle())));

    connectSignals();
}

HelpViewer *CentralWidget::newEmptyTab()
{
    HelpViewer* viewer = new HelpViewer(helpEngine, this, this);
    viewer->installEventFilter(this);
    viewer->setFocus(Qt::OtherFocusReason);
    tabWidget->setCurrentIndex(tabWidget->addTab(viewer, tr("unknown")));

    connectSignals();
    return viewer;
}

void CentralWidget::connectSignals()
{
    const HelpViewer* viewer = currentHelpViewer();
    if (viewer)
    {
        connect(viewer, SIGNAL(copyAvailable(bool)), this, SIGNAL(copyAvailable(bool)));
        connect(viewer, SIGNAL(forwardAvailable(bool)), this, SIGNAL(forwardAvailable(bool)));
        connect(viewer, SIGNAL(backwardAvailable(bool)), this, SIGNAL(backwardAvailable(bool)));
        connect(viewer, SIGNAL(sourceChanged(QUrl)), this, SIGNAL(sourceChanged(QUrl)));
        connect(viewer, SIGNAL(highlighted(QString)), this, SIGNAL(highlighted(QString)));
        connect(viewer, SIGNAL(sourceChanged(QUrl)), this, SLOT(setTabTitle(QUrl)));
    }
}

HelpViewer *CentralWidget::helpViewerAtIndex(int index) const
{
    return qobject_cast<HelpViewer*>(tabWidget->widget(index));
}

HelpViewer *CentralWidget::currentHelpViewer() const
{
    return qobject_cast<HelpViewer*>(tabWidget->currentWidget());
}

void CentralWidget::activateTab(bool onlyHelpViewer)
{
    if (currentHelpViewer())
    {
        currentHelpViewer()->setFocus();
    }
    else
    {
        int idx = 0;
        if (onlyHelpViewer)
            idx = lastTabPage;
        tabWidget->setCurrentIndex(idx);
        tabWidget->currentWidget()->setFocus();
    }
}

void CentralWidget::setTabTitle(const QUrl& url)
{
    Q_UNUSED(url)
    QTabBar *tabBar = qFindChild<QTabBar*>(tabWidget);

    for (int tab = 0; tab < tabBar->count(); ++tab) {
        HelpViewer *viewer = qobject_cast<HelpViewer*>(tabWidget->widget(tab));
        if (viewer)
        {
            tabWidget->setTabText(tab, quoteTabTitle(viewer->documentTitle().trimmed()));
        }
    }
}

void CentralWidget::currentPageChanged(int index)
{
    const HelpViewer *viewer = currentHelpViewer();

    if (viewer)
        lastTabPage = index;

    tabWidget->setTabsClosable(tabWidget->count() > 1);
    QWidget *widget = tabWidget->cornerWidget(Qt::TopRightCorner);
    if (widget)
    {
        widget->setEnabled(viewer && (tabWidget->count() > 1));

        widget = tabWidget->cornerWidget(Qt::TopLeftCorner);
        widget->setEnabled(viewer ? true : false);
    }
    emit currentViewerChanged();
}

void CentralWidget::showTabBarContextMenu(const QPoint &point)
{
    HelpViewer* viewer = helpViewerFromTabPosition(tabWidget, point);
    if (!viewer)
        return;

    QTabBar *tabBar = qFindChild<QTabBar*>(tabWidget);

    QMenu menu(QLatin1String(""), tabBar);
    QAction *newPage = menu.addAction(tr("Add New Page"));

    bool enableAction = tabBar->count() > 1;
    QAction *closePage = menu.addAction(tr("Close This Page"));
    closePage->setEnabled(enableAction);

    QAction *closePages = menu.addAction(tr("Close Other Pages"));
    closePages->setEnabled(enableAction);

    menu.addSeparator();

    QAction *newBookmark = menu.addAction(tr("Add Bookmark for this Page..."));
    const QString &url = viewer->source().toString();
    if (url.isEmpty() || url == QLatin1String("about:blank"))
        newBookmark->setEnabled(false);

    QAction *pickedAction = menu.exec(tabBar->mapToGlobal(point));
    if (pickedAction == newPage)
        setSourceInNewTab(viewer->source());

    if (pickedAction == closePage)
    {
        tabWidget->removeTab(tabWidget->indexOf(viewer));
        QTimer::singleShot(0, viewer, SLOT(deleteLater()));
    }

    if (pickedAction == closePages)
    {
        int currentPage = tabWidget->indexOf(viewer);
        for (int i = tabBar->count() -1; i >= 0; --i)
        {
            viewer = qobject_cast<HelpViewer*>(tabWidget->widget(i));
            if (i != currentPage && viewer)
            {
                tabWidget->removeTab(i);
                QTimer::singleShot(0, viewer, SLOT(deleteLater()));

                if (i < currentPage)
                    --currentPage;
            }
        }
    }

    if (pickedAction == newBookmark)
        emit addNewBookmark(viewer->documentTitle(), url);
}

// If we have a current help viewer then this is the 'focus proxy', otherwise
// it's the tab widget itself. This is needed, so an embedding program can just
// set the focus to the central widget and it does TheRightThing(TM)
void CentralWidget::focusInEvent(QFocusEvent * /* event */)
{
    QObject *receiver = tabWidget;
    if (currentHelpViewer())
        receiver = currentHelpViewer();

    QTimer::singleShot(1, receiver, SLOT(setFocus()));
}

bool CentralWidget::eventFilter(QObject *object, QEvent *e)
{
    if (e->type() == QEvent::KeyPress)
    {
        if ((static_cast<QKeyEvent*>(e))->key() == Qt::Key_Backspace)
        {
            HelpViewer *viewer = currentHelpViewer();
            if (viewer == object) {
                if (viewer->isBackwardAvailable())
                {
                    // this helps in case there is an html <input> field
                    if (!viewer->hasFocus())
                        viewer->backward();
                }
                return true;
            }
        }
    }

    if (qobject_cast<QTabBar*>(object)) {
        bool dblClick = e->type() == QEvent::MouseButtonDblClick;
        if ((e->type() == QEvent::MouseButtonRelease) || dblClick)
        {
            if (tabWidget->count() <= 1)
                return QWidget::eventFilter(object, e);

            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
            HelpViewer *viewer = helpViewerFromTabPosition(tabWidget,
                                                           mouseEvent->pos());

            if (viewer)
            {
                if ((mouseEvent->button() == Qt::MidButton) || dblClick)
                {
                    tabWidget->removeTab(tabWidget->indexOf(viewer));
                    QTimer::singleShot(0, viewer, SLOT(deleteLater()));
                    currentPageChanged(tabWidget->currentIndex());
                    return true;
                }
            }
        }
    }
    return QWidget::eventFilter(object, e);
}

bool CentralWidget::find(const QString &txt, QTextDocument::FindFlags findFlags,
                         bool incremental)
{
    HelpViewer* viewer = currentHelpViewer();

    Q_UNUSED(incremental)
    if (viewer)
    {
        QWebPage::FindFlags options = QWebPage::FindWrapsAroundDocument;
        if (findFlags & QTextDocument::FindBackward)
            options |= QWebPage::FindBackward;
        if (findFlags & QTextDocument::FindCaseSensitively)
            options |= QWebPage::FindCaseSensitively;

        return viewer->findText(txt, options);
    }
    return false;
}

void CentralWidget::copy()
{
    HelpViewer* viewer = currentHelpViewer();
    if (viewer)
        viewer->copy();
}

QString CentralWidget::quoteTabTitle(const QString &title) const
{
    QString s = title;
    return s.replace(QLatin1Char('&'), QLatin1String("&&"));
}

// *******************************************************************************************

TopicChooser::TopicChooser(QWidget *parent) : QDialog(parent)
{
    setWindowIcon(icon("help-browser"));
    setWindowTitle(tr("Topic browser"));

    createControls();

    resize(350, 200);
    setMinimumSize(size());
    setMaximumSize(size());
}

TopicChooser::~TopicChooser()
{
    delete lstView;
}

void TopicChooser::createControls()
{
    QVBoxLayout *layout = new QVBoxLayout();

    QLabel *lblLabel = new QLabel(tr("Display"));

    lstView = new QListWidget(this);
    connect(lstView, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(doAccept()));

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    layout->addWidget(lblLabel);
    layout->addWidget(lstView);
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void TopicChooser::doLinksActivated(const QMap<QString, QUrl> &links, const QString &keyword)
{
    m_links = links;

    lstView->clear();

    QMapIterator<QString, QUrl> i(m_links);
    while (i.hasNext())
    {
        i.next();
        lstView->addItem(i.key());
    }

    exec();
}

void TopicChooser::doAccept()
{
    emit linkActivated(m_links[lstView->currentItem()->text()]);
    accept();
}

void TopicChooser::doReject()
{
    reject();
}
