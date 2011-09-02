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

#include "collaboration.h"
#include "scene.h"

#include <QtWebKit>

ServerDownloadDialog::ServerDownloadDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("ServerDownloadDialog::ServerDownloadDialog()");

    setWindowIcon(icon("collaboration"));
    setWindowTitle(tr("Download from server"));

    createControls();

    setMinimumSize(600, 350);

    QSettings settings;
    restoreGeometry(settings.value("ServerDownloadDialog/Geometry", saveGeometry()).toByteArray());
}

ServerDownloadDialog::~ServerDownloadDialog()
{
    logMessage("ServerDownloadDialog::~ServerDownloadDialog()");

    QSettings settings;
    settings.setValue("ServerDownloadDialog/Geometry", saveGeometry());
}

int ServerDownloadDialog::showDialog()
{
    logMessage("ServerDownloadDialog::showDialog()");

    return exec();
}

void ServerDownloadDialog::createControls()
{
    logMessage("ServerDownloadDialog::createControls()");

    networkAccessManager.setCookieJar(new QNetworkCookieJar());

    webView = new QWebView(this);
    webView->page()->setNetworkAccessManager(&networkAccessManager);
    webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(webView->page(), SIGNAL(linkClicked(QUrl)), this, SLOT(linkClicked(QUrl)));

    QByteArray postData;
    postData.append("login_username=karban");
    postData.append("login_password=agros");

    // QNetworkRequest *request = new QNetworkRequest(QUrl(Util::config()->collaborationServerURL + "/login.php"));
    // request->setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    // networkAccessManager.post(*request, postData);

    // networkReply->setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    networkReply = networkAccessManager.post(QNetworkRequest(QUrl(Util::config()->collaborationServerURL + "/login.php")), postData);

    // connect(networkReply, SIGNAL(finished()), this, SLOT(httpContentFinished()));
    connect(networkReply, SIGNAL(finished()), this, SLOT(httpContentFinished()));
    // webView->setUrl(QUrl(Util::config()->collaborationServerURL + "/problems.php"));
    // webView->load(networkReply->request(), QNetworkAccessManager::PostOperation);

    // dialog buttons
    QPushButton *btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));

    QHBoxLayout *layoutButtonViewport = new QHBoxLayout();
    layoutButtonViewport->addStretch();
    layoutButtonViewport->addWidget(btnClose);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(webView, 1);
    layout->addStretch();
    layout->addLayout(layoutButtonViewport);

    setLayout(layout);
}

void ServerDownloadDialog::load(const QString &str)
{
    QString url = str;
    if (url.contains(".php?"))
        url = url.replace(".php?", ".php?no_header=1&");
    else
        url = url.replace(".php", ".php?no_header=1");

    // qDebug() << "load: " << url;
    webView->load(QUrl(url));
}

void ServerDownloadDialog::httpContentFinished()
{
    load(Util::config()->collaborationServerURL + "problems.php");
}

void ServerDownloadDialog::readFromServerXML(int ID, int version)
{
    logMessage("ServerDownloadDialog::readFromServerXML()");

    networkReply = networkAccessManager.get(QNetworkRequest(QUrl(QString(Util::config()->collaborationServerURL + "/problem_download.php?type=xml&id=%1&version=%2").
                                                                 arg(QString::number(ID)).
                                                                 arg(QString::number(version)))));
    connect(networkReply, SIGNAL(finished()), this, SLOT(httpFileFinished()));
}

void ServerDownloadDialog::httpFileFinished()
{
    QByteArray text = networkReply->readAll();
    QString *data = new QString(text);

    m_fileName = tempProblemDir() + "/collaboration.a2d";
    writeStringContent(m_fileName, data);

    delete data;

    accept();
}

void ServerDownloadDialog::linkClicked(const QUrl &url)
{
    if (url.toString().startsWith(Util::config()->collaborationServerURL))
    {
        if (url.toString().contains("problem_download.php?type=xml&id="))
            readFromServerXML(url.queryItemValue("id").toInt(),
                              url.queryItemValue("version").toInt());
        else
            load(url.toString());
    }
}

void ServerDownloadDialog::doClose()
{
    logMessage("ServerDownloadDialog::doClose()");

    reject();
}

// *************************************************************************************************************

ServerUploadDialog::ServerUploadDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("ServerUploadDialog::ServerUploadDialog()");

    setWindowIcon(icon(""));
    setWindowTitle(tr("Upload to the server"));

    createControls();

    QSettings settings;
    restoreGeometry(settings.value("ServerUploadDialog/Geometry", saveGeometry()).toByteArray());

    readFromServerContent();
}

ServerUploadDialog::~ServerUploadDialog()
{
    logMessage("ServerUploadDialog::~ServerUploadDialog()");

    QSettings settings;
    settings.setValue("ServerUploadDialog/Geometry", saveGeometry());
    settings.setValue("ServerUploadDialog/Author", txtAuthor->text());
    settings.setValue("ServerUploadDialog/Affiliation", txtAffiliation->text());
}

int ServerUploadDialog::showDialog()
{
    logMessage("ServerUploadDialog::showDialog()");

    return exec();
}

void ServerUploadDialog::createControls()
{
    logMessage("ServerUploadDialog::createControls()");

    lblName = new QLabel(this);
    lblName->setVisible(false);
    cmbName = new QComboBox(this);
    cmbName->setVisible(false);
    connect(cmbName, SIGNAL(highlighted(int)), this, SLOT(doExistingProblemSelected(int)));

    lblInformation = new QLabel(this);
    QPalette palette = lblInformation->palette();
    palette.setColor(QPalette::WindowText, Qt::blue);
    lblInformation->setPalette(palette);
    txtAuthor = new QLineEdit(this);
    txtAffiliation = new QLineEdit(this);

    QSettings settings;
    txtAuthor->setText(settings.value("ServerUploadDialog/Author", "").toString());
    txtAffiliation->setText(settings.value("ServerUploadDialog/Affiliation", "").toString());

    radDocumentNew = new QRadioButton(tr("New"), this);
    radDocumentNew->setChecked(true);
    radDocumentExisting = new QRadioButton(tr("Existing"), this);
    connect(radDocumentNew, SIGNAL(clicked()), this, SLOT(doDocumentChanged()));
    connect(radDocumentExisting, SIGNAL(clicked()), this, SLOT(doDocumentChanged()));

    QGridLayout *layoutDialog = new QGridLayout();
    layoutDialog->addWidget(new QLabel(tr("Document:")), 0, 0);
    layoutDialog->addWidget(radDocumentNew, 0, 1);
    layoutDialog->addWidget(radDocumentExisting, 0, 2);
    layoutDialog->addWidget(new QLabel(tr("Name:")), 1, 0);
    layoutDialog->addWidget(lblName, 1, 1, 1, 2);
    layoutDialog->addWidget(cmbName, 1, 1, 1, 2);
    layoutDialog->addWidget(lblInformation, 2, 1);
    layoutDialog->addWidget(new QLabel(tr("Author:")), 3, 0);
    layoutDialog->addWidget(txtAuthor, 3, 1, 1, 2);
    layoutDialog->addWidget(new QLabel(tr("Affiliation:")), 4, 0);
    layoutDialog->addWidget(txtAffiliation, 4, 1, 1, 2);

    // dialog buttons
    btnUpload = new QPushButton(tr("Upload"));
    connect(btnUpload, SIGNAL(clicked()), this, SLOT(doUpload()));
    QPushButton *btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));
    lblNotification = new QLabel(this);

    QHBoxLayout *layoutButtonViewport = new QHBoxLayout();
    layoutButtonViewport->addWidget(lblNotification);
    layoutButtonViewport->addStretch();
    layoutButtonViewport->addWidget(btnClose);
    layoutButtonViewport->addWidget(btnUpload);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutDialog);
    layout->addStretch();
    layout->addLayout(layoutButtonViewport);

    doDocumentChanged();
    setLayout(layout);
}

void ServerUploadDialog::doDocumentChanged()
{
    logMessage("ServerUploadDialog::doDocumentChanged()");

    cmbName->setVisible(false);
    lblName->setVisible(false);

    if (radDocumentExisting->isChecked())
    {
        cmbName->setCurrentIndex(cmbName->findText(Util::scene()->problemInfo()->name));
        if (cmbName->currentIndex() == 0 && cmbName->count() > 0)
            cmbName->setCurrentIndex(0);

        cmbName->setVisible(true);

        lblInformation->setText("New version will be uploaded to the server");
    }
    else
    {
        lblName->setVisible(true);
        lblName->setText(Util::scene()->problemInfo()->name);

        if (Util::scene()->problemInfo()->name == QString(tr("unnamed")))
        {
            lblInformation->setText("Please, set name of the problem in problem dialog");
            btnUpload->setDisabled(true);
        }
        else
            lblInformation->clear();
    }
}

void ServerUploadDialog::doExistingProblemSelected(int index)
{
    if (index != 0)
        btnUpload->setEnabled(true);
    else
        btnUpload->setDisabled(true);
}

void ServerUploadDialog::readFromServerContent()
{
    logMessage("ServerUploadDialog::readFromServerContent()");

    networkReply = networkAccessManager.get(QNetworkRequest(QUrl(Util::config()->collaborationServerURL + "problem_list_xml.php")));
    connect(networkReply, SIGNAL(finished()), this, SLOT(httpContentFinished()));
}

void ServerUploadDialog::httpContentFinished()
{
    logMessage("ServerUploadDialog::httpContentFinished()");

    QString content = networkReply->readAll();
    if (content.isEmpty())
    {
        lblNotification->setText(tr("Colaboration server could not be connected"));
        btnUpload->setDisabled(true);

        return;
    }

    QDomDocument doc;
    doc.setContent(content);

    // problems
    QDomElement eleDoc = doc.documentElement();

    // problems
    QDomNode n = eleDoc.toElement().elementsByTagName("item").at(0);
    while(!n.isNull())
    {
        QDomElement element = n.toElement();

        cmbName->addItem(QString("%1").arg(element.toElement().attribute("name")),
                         element.toElement().attribute("id"));

        n = n.nextSibling();
    }
}

void ServerUploadDialog::uploadToServer()
{
    logMessage("ServerUploadDialog::uploadToServer()");

    QByteArray text = readFileContentByteArray(Util::scene()->problemInfo()->fileName);

    int id_problem = 0;
    QString name = Util::scene()->problemInfo()->name;

    if (radDocumentExisting->isChecked())
    {
        id_problem = cmbName->itemData(cmbName->currentIndex()).toInt();
        name = cmbName->itemText(cmbName->currentIndex());
    }

    QByteArray postData;
    postData.append("id_problem=" + QString::number(id_problem) + "&");
    postData.append("name=" + name + "&");
    postData.append("author=" + txtAuthor->text() + "&");
    postData.append("affiliation=" + txtAffiliation->text() + "&");
    postData.append("content=" + text);

    networkReply = networkAccessManager.post(QNetworkRequest(QUrl(Util::config()->collaborationServerURL + "problem_upload.php")), postData);
    connect(networkReply, SIGNAL(finished()), this, SLOT(httpFileFinished()));
}

void ServerUploadDialog::httpFileFinished()
{
    logMessage("ServerUploadDialog::httpFileFinished()");

    QMessageBox::information(this, tr("Upload to server"), tr("Problem '%1' was uploaded to the server.").arg(Util::scene()->problemInfo()->name));
    accept();
}

void ServerUploadDialog::doUpload()
{
    logMessage("ServerUploadDialog::doUpload()");

    uploadToServer();
}

void ServerUploadDialog::doClose()
{
    logMessage("ServerUploadDialog::doClose()");

    reject();
}
