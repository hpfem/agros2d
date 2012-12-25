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

#include <QSvgRenderer>
#include <QtWebKit>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWebKitWidgets>
#endif

#include "util/global.h"

#include "scene.h"
#include "hermes2d/module.h"
#include "hermes2d/module_agros.h"
#include "hermes2d/problem.h"


static QNetworkAccessManager *networkAccessManager = NULL;
static ServerLoginDialog *serverLoginDialog;

ServerLoginDialog::ServerLoginDialog(QWidget *parent)
    : QDialog(parent), m_userName("")
{
    setWindowIcon(icon("collaboration"));
    setWindowTitle(tr("Login"));

    createControls();

    setMinimumSize(sizeHint().width() * 1.5, sizeHint().height());
    setMaximumSize(sizeHint().width() * 1.5, sizeHint().height());

    QSettings settings;
    restoreGeometry(settings.value("ServerLoginDialog/Geometry", saveGeometry()).toByteArray());

    // remembered values
    txtUsername->setText(settings.value("ServerLoginDialog/Username", "").toString());
    chkRememberPassword->setChecked(settings.value("ServerLoginDialog/RememberPassword", false).toBool());
    txtPassword->setText(settings.value("ServerLoginDialog/Password", "").toString());
}

ServerLoginDialog::~ServerLoginDialog()
{
    QSettings settings;
    settings.setValue("ServerLoginDialog/Geometry", saveGeometry());
}

int ServerLoginDialog::showDialog()
{
    lblCaption->setText("");
    return exec();
}

void ServerLoginDialog::createControls()
{
    txtUsername = new QLineEdit(this);
    txtPassword = new QLineEdit(this);
    txtPassword->setEchoMode(QLineEdit::Password);
    lblCaption = new QLabel(this);

    chkRememberPassword = new QCheckBox(tr("Remember password"), this);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow(tr("Username:"), txtUsername);
    formLayout->addRow(tr("Password:"), txtPassword);
    formLayout->addRow("", chkRememberPassword);
    formLayout->addRow("", lblCaption);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(formLayout, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void ServerLoginDialog::login(const QString &username, const QString &password)
{
    m_userName = tr("Anonymous user");

    QByteArray postData;
    postData.append("login_username=" + username + "&");
    postData.append("login_password=" + password);

    networkReply = networkAccessManager->post(QNetworkRequest(QUrl(Agros2D::configComputer()->collaborationServerURL + "/login_xml.php")), postData);

    connect(networkReply, SIGNAL(finished()), this, SLOT(httpContentFinished()));
}

void ServerLoginDialog::httpContentFinished()
{
    QString content = networkReply->readAll();
    if (content.startsWith("User: "))
    {
        QSettings settings;
        settings.setValue("ServerLoginDialog/Username", txtUsername->text());
        settings.setValue("ServerLoginDialog/RememberPassword", chkRememberPassword->isChecked());
        if (chkRememberPassword->isChecked())
        {
            settings.setValue("ServerLoginDialog/Password", txtPassword->text());
        }
        else
        {
            settings.setValue("ServerLoginDialog/Password", "");
            txtPassword->setText("");
        }

        m_userName = content.right(content.length() - 6);

        accept();
    }
    else
    {
        lblCaption->setText(content);
    }
}

void ServerLoginDialog::doAccept()
{
    if (txtUsername->text().isEmpty())
    {
        QMessageBox::critical(this, "", "Username is empty.");
        return;
    }
    if (txtPassword->text().isEmpty())
    {
        QMessageBox::critical(this, "", "Password is empty.");
        return;
    }

    login(txtUsername->text(), txtPassword->text());
}

void ServerLoginDialog::doReject()
{
    reject();
}

// ***********************************************************************************

ServerDownloadDialog::ServerDownloadDialog(QWidget *parent) : QDialog(parent)
{
    setWindowIcon(icon("collaboration"));
    setWindowTitle(tr("Download from server"));

    createControls();

    setMinimumSize(600, 350);

    QSettings settings;
    restoreGeometry(settings.value("ServerDownloadDialog/Geometry", saveGeometry()).toByteArray());

    load(Agros2D::configComputer()->collaborationServerURL + "problems.php");
}

ServerDownloadDialog::~ServerDownloadDialog()
{
    QSettings settings;
    settings.setValue("ServerDownloadDialog/Geometry", saveGeometry());
}

int ServerDownloadDialog::showDialog()
{
    webView->reload();
    return exec();
}

void ServerDownloadDialog::createControls()
{
    if (!networkAccessManager)
        networkAccessManager = new QNetworkAccessManager();
    networkAccessManager->setCookieJar(new QNetworkCookieJar());

    webView = new QWebView(this);
    webView->page()->setNetworkAccessManager(networkAccessManager);
    webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(webView->page(), SIGNAL(linkClicked(QUrl)), this, SLOT(linkClicked(QUrl)));

    if (!serverLoginDialog)
        serverLoginDialog = new ServerLoginDialog(this);

    // dialog buttons
    QPushButton *btnLogin = new QPushButton(tr("Login"));
    connect(btnLogin, SIGNAL(clicked()), this, SLOT(doLogin()));
    QPushButton *btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));

    QHBoxLayout *layoutButtonViewport = new QHBoxLayout();
    layoutButtonViewport->addStretch();
    layoutButtonViewport->addWidget(btnLogin);
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

    webView->load(QUrl(url));
}

void ServerDownloadDialog::readFromServerXML(int ID, int version)
{
    networkReply = networkAccessManager->get(QNetworkRequest(QUrl(QString(Agros2D::configComputer()->collaborationServerURL + "/problem_download.php?type=xml&id=%1&version=%2").
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

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
void ServerDownloadDialog::linkClicked(const QUrlQuery &url)
#else
void ServerDownloadDialog::linkClicked(const QUrl &url)
#endif
{
    if (url.toString().startsWith(Agros2D::configComputer()->collaborationServerURL))
    {
        if (url.toString().contains("problem_download.php?type=xml&id="))
            readFromServerXML(url.queryItemValue("id").toInt(),
                              url.queryItemValue("version").toInt());
        else
            load(url.toString());
    }
}

void ServerDownloadDialog::doLogin()
{
    serverLoginDialog->showDialog();
    webView->reload();
}

void ServerDownloadDialog::doClose()
{
    reject();
}

// *************************************************************************************************************

ServerUploadDialog::ServerUploadDialog(QWidget *parent) : QDialog(parent)
{
    setWindowIcon(icon(""));
    setWindowTitle(tr("Upload to the server"));

    createControls();

    setMinimumSize(sizeHint().width() * 1.7, sizeHint().height());
    setMaximumSize(sizeHint().width() * 1.7, sizeHint().height());

    QSettings settings;
    restoreGeometry(settings.value("ServerUploadDialog/Geometry", saveGeometry()).toByteArray());
}

ServerUploadDialog::~ServerUploadDialog()
{
    QSettings settings;
    settings.setValue("ServerUploadDialog/Geometry", saveGeometry());
}

int ServerUploadDialog::showDialog()
{
    if (serverLoginDialog->showDialog() == QDialog::Accepted)
    {
        readFromServerContent();
        return exec();
    }
}

void ServerUploadDialog::createControls()
{
    assert(0); //TODO
    //    cmbName = new QComboBox(this);
    //    cmbName->setVisible(false);
    //    txtName = new QLineEdit(this);

    //    lblInformation = new QLabel(this);
    //    QPalette palette = lblInformation->palette();
    //    palette.setColor(QPalette::WindowText, Qt::blue);
    //    lblInformation->setPalette(palette);

    //    radDocumentNew = new QRadioButton(tr("New"), this);
    //    radDocumentNew->setChecked(true);
    //    radDocumentExisting = new QRadioButton(tr("Existing"), this);
    //    connect(radDocumentNew, SIGNAL(clicked()), this, SLOT(doDocumentChanged()));
    //    connect(radDocumentExisting, SIGNAL(clicked()), this, SLOT(doDocumentChanged()));

    //    QGridLayout *layoutDialog = new QGridLayout();
    //    layoutDialog->addWidget(new QLabel(tr("Document:")), 0, 0);
    //    layoutDialog->addWidget(radDocumentNew, 0, 1);
    //    layoutDialog->addWidget(radDocumentExisting, 1, 1);
    //    layoutDialog->addWidget(new QLabel(tr("Name:")), 2, 0);
    //    layoutDialog->addWidget(cmbName, 2, 1, 1, 2);
    //    layoutDialog->addWidget(txtName, 2, 1, 1, 2);
    //    layoutDialog->addWidget(new QLabel(tr("Physic field:")), 3, 0);
    //    layoutDialog->addWidget(new QLabel(QString::fromStdString(Agros2D::problem()->config()->fieldId())), 3, 1);
    //    layoutDialog->addWidget(lblInformation, 5, 1);

    //    // dialog buttons
    //    btnUpload = new QPushButton(tr("Upload"));
    //    connect(btnUpload, SIGNAL(clicked()), this, SLOT(doUpload()));
    //    QPushButton *btnClose = new QPushButton(tr("Close"));
    //    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));

    //    QHBoxLayout *layoutButtonViewport = new QHBoxLayout();
    //    layoutButtonViewport->addStretch();
    //    layoutButtonViewport->addWidget(btnClose);
    //    layoutButtonViewport->addWidget(btnUpload);

    //    QVBoxLayout *layout = new QVBoxLayout();
    //    layout->addLayout(layoutDialog);
    //    layout->addStretch();
    //    layout->addLayout(layoutButtonViewport);

    //    doDocumentChanged();
    //    setLayout(layout);
}

void ServerUploadDialog::doDocumentChanged()
{
    cmbName->setVisible(false);
    txtName->setVisible(false);

    if (radDocumentExisting->isChecked())
    {
        if (cmbName->findText(Agros2D::problem()->config()->name(), Qt::MatchStartsWith) != -1)
            cmbName->setCurrentIndex(cmbName->findText(Agros2D::problem()->config()->name(), Qt::MatchStartsWith));
        else if (cmbName->count() > 0)
            cmbName->setCurrentIndex(0);

        cmbName->setVisible(true);

        lblInformation->setText("New version will be uploaded to the server");
    }
    else
    {
        txtName->setVisible(true);
        txtName->setText(Agros2D::problem()->config()->name());
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
    assert(0); //TODO
    //    QByteArray postData;
    //    postData.append("physicfield=" + QString::fromStdString(Agros2D::problem()->config()->fieldId()));

    //    networkReply = networkAccessManager->post(QNetworkRequest(QUrl(Agros2D::config()->collaborationServerURL + "problems_xml.php")), postData);
    //    connect(networkReply, SIGNAL(finished()), this, SLOT(httpContentFinished()));
}

void ServerUploadDialog::httpContentFinished()
{
    QString content = networkReply->readAll();
    if (content.isEmpty())
    {
        QMessageBox::critical(this, "", tr("Colaboration server could not be connected"));
        qDebug() << content;
        close();
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

        cmbName->addItem(QString("%1 (%2)").
                         arg(element.toElement().attribute("name")).
                         arg(element.toElement().attribute("author")),
                         element.toElement().attribute("id"));

        n = n.nextSibling();
    }

    if (cmbName->findText(Agros2D::problem()->config()->name(), Qt::MatchStartsWith) != -1)
    {
        radDocumentExisting->setChecked(true);
        doDocumentChanged();
    }
}

void ServerUploadDialog::uploadToServer()
{
    assert(0); //TODO
    //    QByteArray text = readFileContentByteArray(Agros2D::problem()->config()->fileName);

    //    int id_problem = 0;
    //    QString name = Agros2D::problem()->config()->name;

    //    if (radDocumentExisting->isChecked())
    //    {
    //        id_problem = cmbName->itemData(cmbName->currentIndex()).toInt();
    //        name = cmbName->itemText(cmbName->currentIndex());
    //    }

    //    QByteArray postData;
    //    postData.append("id_problem=" + QString::number(id_problem) + "&");
    //    postData.append("name=" + name + "&");
    //    postData.append("physicfield=" + QString::fromStdString(Agros2D::problem()->config()->fieldId()) + "&");
    //    postData.append("content=" + text);

    //    networkReply = networkAccessManager->post(QNetworkRequest(QUrl(Agros2D::config()->collaborationServerURL + "problem_upload.php")), postData);
    //    connect(networkReply, SIGNAL(finished()), this, SLOT(httpFileFinished()));
}

void ServerUploadDialog::httpFileFinished()
{
    QString content = networkReply->readAll();

    if (content.startsWith("Message: "))
    {
        QMessageBox::information(this, tr("Upload to server"), tr("Problem '%1' was uploaded to the server.").arg(Agros2D::problem()->config()->name()));
        accept();
    }
    else
    {
        lblInformation->setText(content);
        qDebug() << content;
    }
}

void ServerUploadDialog::doUpload()
{
    uploadToServer();
}

void ServerUploadDialog::doClose()
{
    reject();
}
