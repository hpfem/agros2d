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

ServerDownloadDialog::ServerDownloadDialog(QWidget *parent) : QDialog(parent)
{
    setWindowIcon(icon("collaboration"));
    setWindowTitle(tr("Download from server"));

    createControls();

    setMinimumSize(600, 350);

    QSettings settings;
    restoreGeometry(settings.value("ServerDownloadDialog/Geometry", saveGeometry()).toByteArray());
    txtFind->setText(settings.value("ServerDownloadDialog/Find", "").toString());

    readFromServerContent();
}

ServerDownloadDialog::~ServerDownloadDialog()
{
    QSettings settings;
    settings.setValue("ServerDownloadDialog/Geometry", saveGeometry());
    settings.setValue("ServerDownloadDialog/Find", txtFind->text());
}

int ServerDownloadDialog::showDialog()
{
    return exec();
}

void ServerDownloadDialog::createControls()
{
    logMessage("ServerDownloadDialog::createControls()");

    trvProject = new QTreeWidget(this);
    trvProject->setHeaderHidden(false);
    trvProject->setMouseTracking(true);
    trvProject->setColumnCount(4);

    connect(trvProject, SIGNAL(itemPressed(QTreeWidgetItem *, int)), this, SLOT(doItemSelected(QTreeWidgetItem *, int)));
    connect(trvProject, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(doItemSelected(QTreeWidgetItem *, int)));
    connect(trvProject, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doItemSelected(QTreeWidgetItem *, QTreeWidgetItem *)));

    QStringList labels;
    labels << "" << tr("Date") << tr("Name") << tr("Author");
    trvProject->setHeaderLabels(labels);

    txtFind = new QLineEdit(this);
    btnFind = new QPushButton(tr("Find"));
    connect(btnFind, SIGNAL(clicked()), this, SLOT(doFind()));

    cmbVersion = new QComboBox(this);
    connect(cmbVersion, SIGNAL(currentIndexChanged(int)), this, SLOT(doVersionChanged(int)));

    svgImage = new QSvgWidget(this);
    lblName = new QLabel();
    lblDate = new QLabel();
    lblAuthor = new QLabel();
    lblAffiliation = new QLabel();

    QFormLayout *layoutDetailTable = new QFormLayout();
    layoutDetailTable->addRow(tr("Version:"), cmbVersion);
    layoutDetailTable->addRow(tr("Date:"), lblDate);
    layoutDetailTable->addRow(tr("Name:"), lblName);
    layoutDetailTable->addRow(tr("Author:"), lblAuthor);
    layoutDetailTable->addRow(tr("Affiliation:"), lblAffiliation);

    QVBoxLayout *layoutDetail = new QVBoxLayout();
    layoutDetail->addLayout(layoutDetailTable);
    layoutDetail->addWidget(svgImage);

    QHBoxLayout *layoutFind = new QHBoxLayout();
    layoutFind->addWidget(new QLabel("Text:"));
    layoutFind->addWidget(txtFind);
    layoutFind->addWidget(btnFind);

    QGridLayout *layoutProject = new QGridLayout();
    layoutProject->addLayout(layoutFind, 0, 0);
    layoutProject->addWidget(trvProject, 1, 0);
    layoutProject->addLayout(layoutDetail, 0, 1, 2, 1);

    QWidget *project = new QWidget();
    project->setLayout(layoutProject);

    // dialog buttons
    btnDownload = new QPushButton(tr("Download"));
    btnDownload->setEnabled(false);
    connect(btnDownload, SIGNAL(clicked()), this, SLOT(doDownload()));
    QPushButton *btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));

    QHBoxLayout *layoutButtonViewport = new QHBoxLayout();
    layoutButtonViewport->addStretch();
    layoutButtonViewport->addWidget(btnClose);
    layoutButtonViewport->addWidget(btnDownload);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(project, 1);
    layout->addStretch();
    layout->addLayout(layoutButtonViewport);

    setLayout(layout);
}

void ServerDownloadDialog::readFromServerContent()
{
    QByteArray postData;
    postData.append("text=" + txtFind->text());

    networkReply = networkAccessManager.post(QNetworkRequest(QUrl("http://agros2d.org/collaboration/problem_list_xml.php")), postData);
    connect(networkReply, SIGNAL(finished()), this, SLOT(httpContentFinished()));
}

void ServerDownloadDialog::httpContentFinished()
{
    trvProject->blockSignals(true);
    trvProject->clear();

    QString content = networkReply->readAll();
    QDomDocument doc;
    doc.setContent(content);

    // problems
    QDomElement eleDoc = doc.documentElement();

    // problems
    QDomNode n = eleDoc.toElement().elementsByTagName("item").at(0);
    while(!n.isNull())
    {
        QDomElement element = n.toElement();

        QString id = element.toElement().attribute("id");
        QString id_problem_version = element.toElement().attribute("id_problem_version");
        QString type = element.toElement().attribute("type");
        QString version = element.toElement().attribute("version");
        QString name = element.toElement().attribute("name");
        QString date = element.toElement().attribute("date");
        QString author = element.toElement().attribute("author");
        QString affiliation = element.toElement().attribute("affiliation");

        QTreeWidgetItem *item = new QTreeWidgetItem(trvProject);
        item->setData(0, Qt::UserRole, id);
        item->setIcon(0, icon("agros2d"));
        item->setText(1, QDateTime::fromString(date, "yyyy-MM-dd HH:mm:ss").toString("HH:mm:ss"));
        item->setText(2, name);
        item->setData(2, Qt::UserRole, version);
        item->setText(3, author);

        n = n.nextSibling();
    }

    trvProject->resizeColumnToContents(0);
    trvProject->resizeColumnToContents(1);
    trvProject->resizeColumnToContents(2);
    trvProject->resizeColumnToContents(3);

    trvProject->blockSignals(false);

    if (!trvProject->topLevelItemCount() > 0)
    {
        trvProject->setCurrentItem(trvProject->topLevelItem(0));
        doItemSelected(trvProject->currentItem(), Qt::UserRole);
    }
}

void ServerDownloadDialog::readFromServerXML(int ID, int version)
{
    networkReply = networkAccessManager.get(QNetworkRequest(QUrl(QString("http://agros2d.org/collaboration/problem_download.php?type=xml&id=%1&version=%2").
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

void ServerDownloadDialog::doItemSelected(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    doItemSelected(current, Qt::UserRole);
}

void ServerDownloadDialog::doItemSelected(QTreeWidgetItem *item, int role)
{
    btnDownload->setEnabled(false);

    if (item != NULL)
    {
        btnDownload->setEnabled(true);

        // version
        int maxVersion = trvProject->currentItem()->data(2, Qt::UserRole).toInt();
        cmbVersion->blockSignals(true);
        cmbVersion->clear();
        for (int i = 1; i <= maxVersion; i++)
            cmbVersion->addItem(QString::number(i));
        cmbVersion->setCurrentIndex(-1);

        cmbVersion->blockSignals(false);
        cmbVersion->setCurrentIndex(cmbVersion->count() - 1);
    }
}

void ServerDownloadDialog::doVersionChanged(int index)
{
    // detail
    int ID = trvProject->currentItem()->data(0, Qt::UserRole).toInt();

    QByteArray postData;
    postData.append(QString("id=%1&").arg(ID));
    postData.append(QString("version=%1").arg(cmbVersion->currentIndex() + 1));

    networkReply = networkAccessManager.post(QNetworkRequest(QUrl("http://agros2d.org/collaboration/problem_detail_xml.php")), postData);
    connect(networkReply, SIGNAL(finished()), this, SLOT(httpDetailFinished()));
}

void ServerDownloadDialog::httpDetailFinished()
{
    QString content = networkReply->readAll();
    QDomDocument doc;
    doc.setContent(content);

    // problems
    QDomElement eleDoc = doc.documentElement();

    // problems
    QDomNode n = eleDoc.toElement().elementsByTagName("item").at(0);
    if (!n.isNull())
    {
        QDomElement element = n.toElement();

        lblName->setText(element.toElement().attribute("name"));
        lblDate->setText(QDateTime::fromString(element.toElement().attribute("date"), "yyyy-MM-dd HH:mm:ss").toString("HH:mm:ss dd.MM.yyyy"));
        lblAuthor->setText(element.toElement().attribute("author"));
        lblAffiliation->setText(element.toElement().attribute("affiliation"));
    }

    // svg
    int ID = trvProject->currentItem()->data(0, Qt::UserRole).toInt();
    networkReply = networkAccessManager.get(QNetworkRequest(QUrl(QString("http://agros2d.org/collaboration/problem_download.php?id=%1&version=%2").
                                                                 arg(ID).
                                                                 arg(cmbVersion->currentIndex() + 1))));
    connect(networkReply, SIGNAL(finished()), this, SLOT(httpDetailSvgFinished()));
}

void ServerDownloadDialog::httpDetailSvgFinished()
{
    QByteArray svg = networkReply->readAll();

    QString fileName = tempProblemDir() + "/geometry.svg";

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        stream << svg;

        file.waitForBytesWritten(0);
        file.close();

        svgImage->load(fileName);
    }
}

void ServerDownloadDialog::doFind()
{
    readFromServerContent();
}

void ServerDownloadDialog::doDownload()
{
    if (trvProject->currentItem())
        readFromServerXML(trvProject->currentItem()->data(0, Qt::UserRole).toInt(),
                          cmbVersion->currentIndex() + 1);
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

    readFromServerContent();

    QSettings settings;
    restoreGeometry(settings.value("ServerUploadDialog/Geometry", saveGeometry()).toByteArray());
}

ServerUploadDialog::~ServerUploadDialog()
{
    QSettings settings;
    settings.setValue("ServerUploadDialog/Geometry", saveGeometry());
    settings.setValue("ServerUploadDialog/Author", txtAuthor->text());
    settings.setValue("ServerUploadDialog/Affiliation", txtAffiliation->text());
}

int ServerUploadDialog::showDialog()
{
    return exec();
}

void ServerUploadDialog::createControls()
{
    logMessage("ServerUploadDialog::createControls()");

    lblName = new QLabel(this);
    lblName->setVisible(false);
    cmbName = new QComboBox(this);
    cmbName->setVisible(false);
    lblWarning = new QLabel(this);
    QPalette palette = lblWarning->palette();
    palette.setColor(QPalette::WindowText, Qt::blue);
    lblWarning->setPalette(palette);
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
    doDocumentChanged();

    QGridLayout *layoutDialog = new QGridLayout();
    layoutDialog->addWidget(new QLabel(tr("Document:")), 0, 0);
    layoutDialog->addWidget(radDocumentNew, 0, 1);
    layoutDialog->addWidget(radDocumentExisting, 0, 2);
    layoutDialog->addWidget(new QLabel(tr("Name:")), 1, 0);
    layoutDialog->addWidget(lblName, 1, 1, 1, 2);
    layoutDialog->addWidget(cmbName, 1, 1, 1, 2);
    layoutDialog->addWidget(lblWarning, 2, 1, 1, 2);
    layoutDialog->addWidget(new QLabel(tr("Author:")), 3, 0);
    layoutDialog->addWidget(txtAuthor, 3, 1, 1, 2);
    layoutDialog->addWidget(new QLabel(tr("Affiliation:")), 4, 0);
    layoutDialog->addWidget(txtAffiliation, 4, 1, 1, 2);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutDialog);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void ServerUploadDialog::doDocumentChanged()
{
    cmbName->setVisible(false);
    lblName->setVisible(false);

    if (radDocumentExisting->isChecked())
    {
        cmbName->setCurrentIndex(cmbName->findText(Util::scene()->problemInfo()->name));
        if (cmbName->currentIndex() == 0 && cmbName->count() > 0)
            cmbName->setCurrentIndex(0);

        cmbName->setVisible(true);

        lblWarning->setText("Warning: New version will be uploaded to the server.");
    }
    else
    {
        lblName->setVisible(true);
        lblName->setText(Util::scene()->problemInfo()->name);
        lblWarning->setText("Information: You can set name in problem dialog.");
    }
}

void ServerUploadDialog::readFromServerContent()
{
    networkReply = networkAccessManager.get(QNetworkRequest(QUrl("http://agros2d.org/collaboration/problem_list_xml.php")));
    connect(networkReply, SIGNAL(finished()), this, SLOT(httpContentFinished()));
}

void ServerUploadDialog::httpContentFinished()
{
    QString content = networkReply->readAll();
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

    networkReply = networkAccessManager.post(QNetworkRequest(QUrl("http://agros2d.org/collaboration/problem_upload.php")), postData);
    connect(networkReply, SIGNAL(finished()), this, SLOT(httpFileFinished()));
}

void ServerUploadDialog::httpFileFinished()
{
    QMessageBox::information(this, tr("Upload to server"), tr("Problem '%1' was uploaded to the server.").arg(Util::scene()->problemInfo()->name));
    accept();
}

void ServerUploadDialog::doAccept()
{
    uploadToServer();
}

void ServerUploadDialog::doReject()
{
    reject();
}
