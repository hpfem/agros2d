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

#include "colaboration.h"
#include "scene.h"

CloudDialogContent::CloudDialogContent(QWidget *parent) : QDialog(parent)
{
    setWindowIcon(icon(""));
    setWindowTitle(tr("Download from server"));

    createControls();

    QSettings settings;
    restoreGeometry(settings.value("CloudDialogContent/Geometry", saveGeometry()).toByteArray());

    readFromCloudContent();
}

CloudDialogContent::~CloudDialogContent()
{
    QSettings settings;
    settings.setValue("CloudDialogContent/Geometry", saveGeometry());
}

int CloudDialogContent::showDialog()
{
    return exec();
}

void CloudDialogContent::createControls()
{
    logMessage("CloudDialogContent::createControls()");

    trvProject = new QTreeWidget(this);
    trvProject->setHeaderHidden(false);
    trvProject->setMouseTracking(true);
    trvProject->setColumnCount(6);

    QStringList labels;
    labels << "" << tr("Date") << tr("Name") << tr("Version") << tr("Author") << tr("Affiliation");
    trvProject->setHeaderLabels(labels);

    QGridLayout *layoutProject = new QGridLayout();
    layoutProject->addWidget(trvProject);

    QWidget *project = new QWidget();
    project->setLayout(layoutProject);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(project, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void CloudDialogContent::readFromCloudContent()
{
    QByteArray postData;

    networkReply = networkAccessManager.get(QNetworkRequest(QUrl("http://agros2d.org/agros_problem_list_xml.php")));
    connect(networkReply, SIGNAL(finished()), this, SLOT(httpContentFinished()));
}

void CloudDialogContent::httpContentFinished()
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

        QString id = element.toElement().attribute("id");
        QString id_problem_version = element.toElement().attribute("id_problem_version");
        QString type = element.toElement().attribute("type");
        QString version = element.toElement().attribute("version");
        QString name = element.toElement().attribute("name");
        QDateTime date = QDateTime::fromString(element.toElement().attribute("date"), "yyyy-MM-dd HH:mm:ss");
        QString author = element.toElement().attribute("author");
        QString affiliation = element.toElement().attribute("affiliation");

        QTreeWidgetItem *item = new QTreeWidgetItem(trvProject);
        item->setData(0, Qt::UserRole, id_problem_version);
        item->setIcon(0, icon("agros2d"));
        item->setText(1, date.toString("HH:mm:ss dd.MM.yyyy"));
        item->setText(2, name);
        item->setText(3, version);
        item->setText(4, author);
        item->setText(5, affiliation);

        n = n.nextSibling();
    }

    trvProject->resizeColumnToContents(0);
    trvProject->resizeColumnToContents(1);
    trvProject->resizeColumnToContents(2);
    trvProject->resizeColumnToContents(3);
    trvProject->resizeColumnToContents(4);
}

void CloudDialogContent::readFromCloud(int ID)
{
    QByteArray postData;
    postData.append("id_version=" + QString::number(ID));

    networkReply = networkAccessManager.post(QNetworkRequest(QUrl("http://agros2d.org/agros_problem_download.php")), postData);
    connect(networkReply, SIGNAL(finished()), this, SLOT(httpFileFinished()));
}

void CloudDialogContent::httpFileFinished()
{
    QByteArray text = networkReply->readAll();
    QString *data = new QString(text);

    m_fileName = tempProblemDir() + "/colaboration.a2d";
    writeStringContent(m_fileName, data);

    delete data;

    accept();
}

void CloudDialogContent::doAccept()
{
    if (trvProject->currentItem())
        readFromCloud(trvProject->currentItem()->data(0, Qt::UserRole).toInt());
}

void CloudDialogContent::doReject()
{
    reject();
}

// *************************************************************************************

CloudDialogSave::CloudDialogSave(QWidget *parent) : QDialog(parent)
{
    setWindowIcon(icon(""));
    setWindowTitle(tr("Write to cloud"));

    createControls();

    readFromCloudContent();

    QSettings settings;
    restoreGeometry(settings.value("CloudDialogSave/Geometry", saveGeometry()).toByteArray());
}

CloudDialogSave::~CloudDialogSave()
{
    QSettings settings;
    settings.setValue("CloudDialogSave/Geometry", saveGeometry());
    settings.setValue("CloudDialogSave/Author", txtAuthor->text());
    settings.setValue("CloudDialogSave/Affiliation", txtAffiliation->text());
}

int CloudDialogSave::showDialog()
{
    return exec();
}

void CloudDialogSave::createControls()
{
    logMessage("CloudDialogSave::createControls()");

    cmbName = new QComboBox(this);
    cmbName->setVisible(false);
    txtName = new QLineEdit(this);
    txtAuthor = new QLineEdit(this);
    txtAffiliation = new QLineEdit(this);

    txtName->setText(Util::scene()->problemInfo()->name);
    QSettings settings;
    txtAuthor->setText(settings.value("CloudDialogSave/Author", "").toString());
    txtAffiliation->setText(settings.value("CloudDialogSave/Affiliation", "").toString());

    cmbDocument = new QComboBox(this);
    cmbDocument->addItem("New", "new");
    cmbDocument->addItem("Existing", "existing");
    connect(cmbDocument, SIGNAL(currentIndexChanged(int)), this, SLOT(doDocumentChanged(int)));
    doDocumentChanged(0);

    QGridLayout *layoutCloud = new QGridLayout();
    layoutCloud->addWidget(new QLabel(tr("Document:")), 0, 0);
    layoutCloud->addWidget(cmbDocument, 0, 1);
    layoutCloud->addWidget(new QLabel(tr("Name:")), 1, 0);
    layoutCloud->addWidget(txtName, 1, 1);
    layoutCloud->addWidget(cmbName, 1, 1);
    layoutCloud->addWidget(new QLabel(tr("Author:")), 2, 0);
    layoutCloud->addWidget(txtAuthor, 2, 1);
    layoutCloud->addWidget(new QLabel(tr("Affiliation:")), 3, 0);
    layoutCloud->addWidget(txtAffiliation, 3, 1);

    QWidget *project = new QWidget();
    project->setLayout(layoutCloud);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(project, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void CloudDialogSave::doDocumentChanged(int index)
{
    txtName->setVisible(false);
    cmbName->setVisible(false);
    if (cmbDocument->itemData(index).toString() == "new")
        txtName->setVisible(true);
    else
        cmbName->setVisible(true);
}

void CloudDialogSave::readFromCloudContent()
{
    QByteArray postData;

    networkReply = networkAccessManager.get(QNetworkRequest(QUrl("http://agros2d.org/agros_problem_list.php")));
    connect(networkReply, SIGNAL(finished()), this, SLOT(httpContentFinished()));
}

void CloudDialogSave::httpContentFinished()
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

        cmbName->addItem(QString("%1 (%2)").
                         arg(element.toElement().attribute("name")).
                         arg(element.toElement().attribute("author")),
                         element.toElement().attribute("id"));

        n = n.nextSibling();
    }
}

void CloudDialogSave::saveToCloud()
{
    QByteArray text = readFileContentByteArray(Util::scene()->problemInfo()->fileName);

    int id_problem = 0;
    if (cmbDocument->itemData(cmbDocument->currentIndex()).toString() == "existing")
        id_problem = cmbName->itemData(cmbName->currentIndex()).toInt();

    QByteArray postData;
    postData.append("id_problem=" + QString::number(id_problem) + "&");
    postData.append("name=" + txtName->text() + "&");
    postData.append("author=" + txtAuthor->text() + "&");
    postData.append("affiliation=" + txtAffiliation->text() + "&");
    postData.append("content=" + text);

    networkReply = networkAccessManager.post(QNetworkRequest(QUrl("http://agros2d.org/agros_problem_upload.php")), postData);
    connect(networkReply, SIGNAL(finished()), this, SLOT(httpFileFinished()));
}

void CloudDialogSave::httpFileFinished()
{
    accept();
}

void CloudDialogSave::doAccept()
{
    saveToCloud();
}

void CloudDialogSave::doReject()
{
    reject();
}
