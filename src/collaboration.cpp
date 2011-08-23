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

#include <QSvgRenderer>

#include "collaboration.h"
#include "scene.h"

#include <QtWebKit>

static QNetworkAccessManager *networkAccessManager = NULL;
static ServerLoginDialog *serverLoginDialog;

ServerLoginDialog::ServerLoginDialog(QWidget *parent)
    : QDialog(parent), m_userName("")
{
    logMessage("ServerLoginDialog::ServerLoginDialog()");

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
    logMessage("ServerLoginDialog::~ServerLoginDialog()");

    QSettings settings;
    settings.setValue("ServerLoginDialog/Geometry", saveGeometry());
}

int ServerLoginDialog::showDialog()
{
    lblCaption->setText("");
    logMessage("ServerLoginDialog::showDialog()");

    return exec();
}

void ServerLoginDialog::createControls()
{
    logMessage("ServerLoginDialog::createControls()");

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

    networkReply = networkAccessManager->post(QNetworkRequest(QUrl(Util::config()->collaborationServerURL + "/login_xml.php")), postData);

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
    logMessage("ServerLoginDialog::doAccept()");

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
    logMessage("ServerLoginDialog::doReject()");

    reject();
}

// ***********************************************************************************

ServerDownloadDialog::ServerDownloadDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("ServerDownloadDialog::ServerDownloadDialog()");

    setWindowIcon(icon("collaboration"));
    setWindowTitle(tr("Download from server"));

    createControls();

    setMinimumSize(600, 350);

    QSettings settings;
    restoreGeometry(settings.value("ServerDownloadDialog/Geometry", saveGeometry()).toByteArray());

    load(Util::config()->collaborationServerURL + "problems.php");
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

    webView->reload();
    return exec();
}

void ServerDownloadDialog::createControls()
{
    logMessage("ServerDownloadDialog::createControls()");

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
    logMessage("ServerDownloadDialog::readFromServerXML()");

    networkReply = networkAccessManager->get(QNetworkRequest(QUrl(QString(Util::config()->collaborationServerURL + "/problem_download.php?type=xml&id=%1&version=%2").
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

void ServerDownloadDialog::doLogin()
{
    serverLoginDialog->showDialog();
    webView->reload();
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

    setMinimumSize(sizeHint().width() * 1.7, sizeHint().height());
    setMaximumSize(sizeHint().width() * 1.7, sizeHint().height());

    QSettings settings;
    restoreGeometry(settings.value("ServerUploadDialog/Geometry", saveGeometry()).toByteArray());
}

ServerUploadDialog::~ServerUploadDialog()
{
    logMessage("ServerUploadDialog::~ServerUploadDialog()");

    QSettings settings;
    settings.setValue("ServerUploadDialog/Geometry", saveGeometry());
}

int ServerUploadDialog::showDialog()
{
    logMessage("ServerUploadDialog::showDialog()");

    if (serverLoginDialog->showDialog() == QDialog::Accepted)
    {
        readFromServerContent();
        return exec();
    }
}

void ServerUploadDialog::createControls()
{
    logMessage("ServerUploadDialog::createControls()");

    cmbName = new QComboBox(this);
    cmbName->setVisible(false);
    txtName = new QLineEdit(this);

    lblInformation = new QLabel(this);
    QPalette palette = lblInformation->palette();
    palette.setColor(QPalette::WindowText, Qt::blue);
    lblInformation->setPalette(palette);

    radDocumentNew = new QRadioButton(tr("New"), this);
    radDocumentNew->setChecked(true);
    radDocumentExisting = new QRadioButton(tr("Existing"), this);
    connect(radDocumentNew, SIGNAL(clicked()), this, SLOT(doDocumentChanged()));
    connect(radDocumentExisting, SIGNAL(clicked()), this, SLOT(doDocumentChanged()));

    QGridLayout *layoutDialog = new QGridLayout();
    layoutDialog->addWidget(new QLabel(tr("Document:")), 0, 0);
    layoutDialog->addWidget(radDocumentNew, 0, 1);
    layoutDialog->addWidget(radDocumentExisting, 1, 1);
    layoutDialog->addWidget(new QLabel(tr("Name:")), 2, 0);
    layoutDialog->addWidget(cmbName, 2, 1, 1, 2);
    layoutDialog->addWidget(txtName, 2, 1, 1, 2);
    layoutDialog->addWidget(new QLabel(tr("Physic field:")), 3, 0);
    layoutDialog->addWidget(new QLabel(physicFieldString(Util::scene()->problemInfo()->physicField())), 3, 1);
    layoutDialog->addWidget(lblInformation, 5, 1);

    // dialog buttons
    btnUpload = new QPushButton(tr("Upload"));
    connect(btnUpload, SIGNAL(clicked()), this, SLOT(doUpload()));
    QPushButton *btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));

    QHBoxLayout *layoutButtonViewport = new QHBoxLayout();
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
    txtName->setVisible(false);

    if (radDocumentExisting->isChecked())
    {
        if (cmbName->findText(Util::scene()->problemInfo()->name, Qt::MatchStartsWith) != -1)
            cmbName->setCurrentIndex(cmbName->findText(Util::scene()->problemInfo()->name, Qt::MatchStartsWith));
        else if (cmbName->count() > 0)
            cmbName->setCurrentIndex(0);

        cmbName->setVisible(true);

        lblInformation->setText("New version will be uploaded to the server");
    }
    else
    {
        txtName->setVisible(true);
        txtName->setText(Util::scene()->problemInfo()->name);
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

    QByteArray postData;
    postData.append("physicfield=" + physicFieldToStringKey(Util::scene()->problemInfo()->physicField()));

    networkReply = networkAccessManager->post(QNetworkRequest(QUrl(Util::config()->collaborationServerURL + "problems_xml.php")), postData);
    connect(networkReply, SIGNAL(finished()), this, SLOT(httpContentFinished()));
}

void ServerUploadDialog::httpContentFinished()
{
    logMessage("ServerUploadDialog::httpContentFinished()");

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

    if (cmbName->findText(Util::scene()->problemInfo()->name, Qt::MatchStartsWith) != -1)
    {
        radDocumentExisting->setChecked(true);
        doDocumentChanged();
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
    postData.append("physicfield=" + physicFieldToStringKey(Util::scene()->problemInfo()->physicField()) + "&");
    postData.append("content=" + text);

    networkReply = networkAccessManager->post(QNetworkRequest(QUrl(Util::config()->collaborationServerURL + "problem_upload.php")), postData);
    connect(networkReply, SIGNAL(finished()), this, SLOT(httpFileFinished()));
}

void ServerUploadDialog::httpFileFinished()
{
    logMessage("ServerUploadDialog::httpFileFinished()");

    QString content = networkReply->readAll();

    if (content.startsWith("Message: "))
    {
        QMessageBox::information(this, tr("Upload to server"), tr("Problem '%1' was uploaded to the server.").arg(Util::scene()->problemInfo()->name));
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
    logMessage("ServerUploadDialog::doUpload()");

    uploadToServer();
}

void ServerUploadDialog::doClose()
{
    logMessage("ServerUploadDialog::doClose()");

    reject();
}
