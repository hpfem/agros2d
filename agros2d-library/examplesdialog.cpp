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

#include "examplesdialog.h"

#include "util/constants.h"
#include "util/global.h"

#include "scene.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "moduledialog.h"

#include "gui/lineeditdouble.h"
#include "gui/latexviewer.h"
#include "gui/common.h"

#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

#include "hermes2d/module.h"

#include "ctemplate/template.h"

#include "../resources_source/classes/problem_a2d_31_xml.h"

ExamplesDialog::ExamplesDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Examples"));
    setModal(true);

    m_selectedFilename = "";
    m_selectedFormFilename = "";

    // problem information
    webView = new QWebView();
    webView->page()->setNetworkAccessManager(networkAccessManager());
    webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    connect(webView->page(), SIGNAL(linkClicked(QUrl)), this, SLOT(linkClicked(QUrl)));

    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", htmlFontFamily().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(htmlFontSize()).toStdString()));

    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/style_common.css").toStdString(), ctemplate::DO_NOT_STRIP, &stylesheet, &style);
    m_cascadeStyleSheet = QString::fromStdString(style);

    lstProblems = new QTreeWidget(this);
    lstProblems->setMouseTracking(true);
    lstProblems->setColumnCount(1);
    lstProblems->setIndentation(15);
    lstProblems->setIconSize(QSize(24, 24));
    lstProblems->setHeaderHidden(true);
    lstProblems->setMinimumWidth(320);

    connect(lstProblems, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doItemDoubleClicked(QTreeWidgetItem *, int)));
    connect(lstProblems, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    QHBoxLayout *layoutSurface = new QHBoxLayout();
    layoutSurface->addWidget(lstProblems);
    layoutSurface->addWidget(webView, 1);

    QWidget *widget = new QWidget();
    widget->setLayout(layoutSurface);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(widget, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    readProblems();

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    /*
    if (lstProblems->count() > 0)
    {
        lstProblems->setCurrentRow(0);
        doItemSelected(lstProblems->currentItem());
    }
    */

    QSettings settings;
    restoreGeometry(settings.value("ExamplesDialog/Geometry", saveGeometry()).toByteArray());
}

ExamplesDialog::~ExamplesDialog()
{
    QSettings settings;
    settings.setValue("ExamplesDialog/Geometry", saveGeometry());
}

void ExamplesDialog::doAccept()
{
    accept();
}

void ExamplesDialog::doReject()
{
    reject();
}

int ExamplesDialog::showDialog()
{
    return exec();
}

void ExamplesDialog::doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    webView->setHtml("");

    if (current)
    {
        m_selectedFilename = current->data(0, Qt::UserRole).toString();
        if (!m_selectedFilename.isEmpty())
        {
            problemInfo(m_selectedFilename);
            buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        }
    }
}

void ExamplesDialog::doItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (lstProblems->currentItem())
    {
        if (!lstProblems->currentItem()->data(0, Qt::UserRole).toString().isEmpty())
            accept();
    }
}

void ExamplesDialog::linkClicked(const QUrl &url)
{
    QString search = "/open?";
    if (url.toString().contains(search))
    {
#if QT_VERSION < 0x050000
        QString fileName = url.queryItemValue("filename");
        QString form = url.queryItemValue("form");
#else
        QString fileName = QUrlQuery(url).queryItemValue("filename");
        QString form = QUrlQuery(url).queryItemValue("form");
#endif

        m_selectedFilename = QUrl(fileName).toLocalFile();
        m_selectedFormFilename = QUrl(form).toLocalFile();

        accept();
    }
}

void ExamplesDialog::readProblems()
{
    // clear listview
    lstProblems->clear();

    QDir dir(QString("%1/resources/examples").arg(datadir()));
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks);

    readProblems(dir, lstProblems->invisibleRootItem());
}

int ExamplesDialog::readProblems(QDir dir, QTreeWidgetItem *parentItem)
{
    int count = 0;

    QFileInfoList listExamples = dir.entryInfoList();
    for (int i = 0; i < listExamples.size(); ++i)
    {
        QFileInfo fileInfo = listExamples.at(i);
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        if (fileInfo.isDir())
        {
            QFont fnt = lstProblems->font();
            fnt.setBold(true);

            QTreeWidgetItem *dirItem = new QTreeWidgetItem(parentItem);
            dirItem->setText(0, fileInfo.fileName());
            dirItem->setFont(0, fnt);
            dirItem->setExpanded(true);

            // recursive read
            int numberOfProblems = readProblems(fileInfo.absoluteFilePath(), dirItem);

            if (numberOfProblems == 0)
            {
                // remove dir from tree
                parentItem->removeChild(dirItem);
            }

            // increase counter
            count += numberOfProblems;
        }
        else if (fileInfo.suffix() == "a2d")
        {
            QList<QIcon> icons = problemIcons(fileInfo.absoluteFilePath());

            QTreeWidgetItem *exampleProblemItem = new QTreeWidgetItem(parentItem);
            if (icons.count() == 1)
                exampleProblemItem->setIcon(0, icons.at(0));
            else
                exampleProblemItem->setIcon(0, icon("fields/empty"));
            exampleProblemItem->setText(0, fileInfo.baseName());
            exampleProblemItem->setData(0, Qt::UserRole, fileInfo.absoluteFilePath());

            // increase counter
            count++;
        }
        else if (fileInfo.suffix() == "py")
        {
            // skip ui python
            if (!QFile::exists(fileInfo.absoluteFilePath().left(fileInfo.absoluteFilePath().length() - 3) + ".ui"))
            {
                QTreeWidgetItem *examplePythonItem = new QTreeWidgetItem(parentItem);
                examplePythonItem->setIcon(0, icon("script-python"));
                examplePythonItem->setText(0, fileInfo.baseName());
                examplePythonItem->setData(0, Qt::UserRole, fileInfo.absoluteFilePath());

                // increase counter
                count++;
            }
        }
        else if (fileInfo.suffix() == "ui")
        {
            QTreeWidgetItem *exampleFormItem = new QTreeWidgetItem(parentItem);
            exampleFormItem->setIcon(0, icon("options-main"));
            exampleFormItem->setText(0, fileInfo.baseName());
            exampleFormItem->setData(0, Qt::UserRole, fileInfo.absoluteFilePath());

            // increase counter
            count++;
        }
    }

    return count;
}

void ExamplesDialog::problemInfo(const QString &fileName)
{
    if (QFile::exists(fileName))
    {
        QFileInfo fileInfo(fileName);

        // template
        std::string info;
        ctemplate::TemplateDictionary problemInfo("info");

        // problem info
        problemInfo.SetValue("AGROS2D", "file:///" + compatibleFilename(QDir(datadir() + TEMPLATEROOT + "/panels/agros2d_logo.png").absolutePath()).toStdString());

        problemInfo.SetValue("STYLESHEET", m_cascadeStyleSheet.toStdString());
        problemInfo.SetValue("PANELS_DIRECTORY", QUrl::fromLocalFile(QString("%1%2").arg(QDir(datadir()).absolutePath()).arg(TEMPLATEROOT + "/panels")).toString().toStdString());
        problemInfo.SetValue("BASIC_INFORMATION_LABEL", tr("Basic informations").toStdString());

        problemInfo.SetValue("NAME_LABEL", tr("Name:").toStdString());
        problemInfo.SetValue("NAME", fileInfo.baseName().toStdString());

        QString templateName;
        if (fileInfo.suffix() == "a2d")
        {
            templateName = "example_problem.tpl";

            try
            {
                std::auto_ptr<XMLProblem::document> document_xsd = XMLProblem::document_(compatibleFilename(fileName).toStdString(), xml_schema::flags::dont_validate);
                XMLProblem::document *doc = document_xsd.get();

                problemInfo.SetValue("COORDINATE_TYPE_LABEL", tr("Coordinate type:").toStdString());
                problemInfo.SetValue("COORDINATE_TYPE", coordinateTypeString(coordinateTypeFromStringKey(QString::fromStdString(doc->problem().coordinate_type()))).toStdString());

                // nodes
                QList<SceneNode *> nodes;
                // nodes
                for (unsigned int i = 0; i < doc->geometry().nodes().node().size(); i++)
                {
                    XMLProblem::node node = doc->geometry().nodes().node().at(i);

                    Point point = Point(node.x(),
                                        node.y());

                    nodes.append(new SceneNode(point));
                }

                // edges
                QList<SceneEdge *> edges;
                for (unsigned int i = 0; i < doc->geometry().edges().edge().size(); i++)
                {
                    XMLProblem::edge edge = doc->geometry().edges().edge().at(i);

                    SceneNode *nodeFrom = nodes.at(edge.start());
                    SceneNode *nodeTo = nodes.at(edge.end());

                    edges.append(new SceneEdge(nodeFrom, nodeTo, edge.angle()));
                }

                // geometry
                QString geometry = generateSvgGeometry(edges);

                // cleanup
                foreach (SceneNode *node, nodes)
                    delete node;
                nodes.clear();
                foreach (SceneEdge *edge, edges)
                    delete edge;
                edges.clear();

                problemInfo.SetValue("GEOMETRY_LABEL", tr("Geometry").toStdString());
                problemInfo.SetValue("GEOMETRY_NODES_LABEL", tr("Nodes:").toStdString());
                problemInfo.SetValue("GEOMETRY_NODES", QString::number(doc->geometry().nodes().node().size()).toStdString());
                problemInfo.SetValue("GEOMETRY_EDGES_LABEL", tr("Edges:").toStdString());
                problemInfo.SetValue("GEOMETRY_EDGES", QString::number(doc->geometry().edges().edge().size()).toStdString());
                problemInfo.SetValue("GEOMETRY_LABELS_LABEL", tr("Labels:").toStdString());
                problemInfo.SetValue("GEOMETRY_LABELS", QString::number(doc->geometry().labels().label().size()).toStdString());
                problemInfo.SetValue("GEOMETRY_SVG", geometry.toStdString());

                problemInfo.SetValue("PHYSICAL_FIELD_MAIN_LABEL", tr("Physical fields").toStdString());

                // fields
                for (unsigned int i = 0; i < doc->problem().fields().field().size(); i++)
                {
                    XMLProblem::field field = doc->problem().fields().field().at(i);

                    ctemplate::TemplateDictionary *fieldInfo = problemInfo.AddSectionDictionary("FIELD_SECTION");

                    fieldInfo->SetValue("PHYSICAL_FIELD_LABEL", Module::availableModules()[QString::fromStdString(field.field_id())].toStdString());

                    fieldInfo->SetValue("ANALYSIS_TYPE_LABEL", tr("Analysis:").toStdString());
                    fieldInfo->SetValue("ANALYSIS_TYPE", analysisTypeString(analysisTypeFromStringKey(QString::fromStdString(field.analysis_type()))).toStdString());

                    fieldInfo->SetValue("LINEARITY_TYPE_LABEL", tr("Solver:").toStdString());
                    fieldInfo->SetValue("LINEARITY_TYPE", linearityTypeString(linearityTypeFromStringKey(QString::fromStdString(field.linearity_type()))).toStdString());

                    problemInfo.ShowSection("FIELD");
                }
            }
            catch (...)
            {

            }
        }
        else if (fileInfo.suffix() == "py")
        {
            templateName = "example_python.tpl";

            // python
            if (QFile::exists(fileName))
            {
                // replace current path in index.html
                QString python = readFileContent(fileName   );
                problemInfo.SetValue("PROBLEM_PYTHON", python.toStdString());
            }
        }
        else if (fileInfo.suffix() == "ui")
        {
            templateName = "example_form.tpl";
        }

        // details
        QString detailsFilename(QString("%1/%2/index.html").arg(fileInfo.absolutePath()).arg(fileInfo.baseName()));
        if (QFile::exists(detailsFilename))
        {
            // replace current path in index.html
            QString detail = readFileContent(detailsFilename);
            detail = detail.replace("{{DIR}}", QString("%1/%2").arg(QUrl::fromLocalFile(fileInfo.absolutePath()).toString()).arg(fileInfo.baseName()));
            detail = detail.replace("{{RESOURCES}}", QUrl::fromLocalFile(QString("%1/resources/").arg(QDir(datadir()).absolutePath())).toString());

            problemInfo.SetValue("PROBLEM_DETAILS", detail.toStdString());
        }

        ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/" + templateName.toStdString(), ctemplate::DO_NOT_STRIP, &problemInfo, &info);

        // setHtml(...) doesn't work
        // webView->setHtml(QString::fromStdString(info));

        // load(...) works
        writeStringContent(tempProblemDir() + "/example.html", QString::fromStdString(info));
        webView->load(QUrl::fromLocalFile(tempProblemDir() + "/example.html"));
    }
}

QList<QIcon> ExamplesDialog::problemIcons(const QString &fileName)
{
    QList<QIcon> icons;

    if (QFile::exists(fileName))
    {
        // open file
        QFile file(fileName);

        QDomDocument doc;
        if (!doc.setContent(&file))
        {
            file.close();
            throw AgrosException(tr("File '%1' is not valid Agros2D file.").arg(fileName));
            return icons;
        }
        file.close();

        // main document
        QDomElement eleDoc = doc.documentElement();

        // problem info
        QDomNode eleProblemInfo = eleDoc.elementsByTagName("problem").at(0);

        QDomNode eleFields = eleProblemInfo.toElement().elementsByTagName("fields").at(0);
        QDomNode nodeField = eleFields.firstChild();
        while (!nodeField.isNull())
        {
            QDomNode eleField = nodeField.toElement();
            icons.append(icon("fields/" + eleField.toElement().attribute("field_id")));

            // next field
            nodeField = nodeField.nextSibling();
        }
    }

    return icons;
}
