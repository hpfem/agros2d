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

#include "hermes2d/module.h"

#include "ctemplate/template.h"

ExamplesDialog::ExamplesDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Examples"));
    setModal(true);

    m_selectedFilename = "";

    // problem information
    webView = new QWebView(this);

    lstProblems = new QTreeWidget(this);
    lstProblems->setMouseTracking(true);
    lstProblems->setColumnCount(1);
    lstProblems->setIndentation(20);
    lstProblems->setHeaderHidden(true);
    lstProblems->setMinimumWidth(230);

    connect(lstProblems, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
            this, SLOT(doItemDoubleClicked(QTreeWidgetItem *, int)));
    connect(lstProblems, SIGNAL(itemActivated(QTreeWidgetItem *, int)),
            this, SLOT(doItemSelected(QTreeWidgetItem *, int)));
    connect(lstProblems, SIGNAL(itemPressed(QTreeWidgetItem *, int)),
            this, SLOT(doItemSelected(QTreeWidgetItem *, int)));

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

void ExamplesDialog::doItemSelected(QTreeWidgetItem *item, int column)
{
    m_selectedFilename = item->data(0, Qt::UserRole).toString();
    if (!m_selectedFilename.isEmpty())
    {
        showProblemInfo(m_selectedFilename);
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
    else
    {
        webView->setHtml("");
    }
}

void ExamplesDialog::doItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (lstProblems->currentItem())
    {
        m_selectedFilename = lstProblems->currentItem()->data(0, Qt::UserRole).toString();
        accept();
    }
}

void ExamplesDialog::readProblems()
{
    // clear listview
    lstProblems->clear();

    QDir dir(QString("%1/resources/examples").arg(datadir()));
    dir.setFilter(QDir::Dirs | QDir::NoSymLinks);

    QFileInfoList listDir = dir.entryInfoList();
    for (int i = 0; i < listDir.size(); ++i)
    {
        QFileInfo dirInfo = listDir.at(i);
        if (dirInfo.fileName() == "." || dirInfo.fileName() == "..")
            continue;

        QTreeWidgetItem *dirItem = new QTreeWidgetItem(lstProblems);
        dirItem->setText(0, dirInfo.fileName());
        dirItem->setExpanded(true);

        // examples
        QDir dirExamples(QString("%1/resources/examples/%2").arg(datadir()).arg(dirInfo.fileName()));
        dirExamples.setFilter(QDir::Files | QDir::NoSymLinks);

        QFileInfoList listExamples = dirExamples.entryInfoList();
        for (int j = 0; j < listExamples.size(); ++j)
        {
            QFileInfo fileInfo = listExamples.at(j);
            if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
                continue;

            if (fileInfo.suffix() == "a2d")
            {
                QTreeWidgetItem *exampleItem = new QTreeWidgetItem(dirItem);
                // item->setIcon(icon("fields/" + it.key()));
                exampleItem->setText(0, fileInfo.baseName());
                exampleItem->setData(0, Qt::UserRole, fileInfo.absoluteFilePath());
            }
        }
    }
}

void ExamplesDialog::showProblemInfo(const QString &fileName)
{
    if (QFile::exists(fileName))
    {
        // open file
        QFile file(fileName);

        QDomDocument doc;
        if (!doc.setContent(&file))
        {
            file.close();
            throw AgrosException(tr("File '%1' is not valid Agros2D file.").arg(fileName));
            return;
        }
        file.close();

        // main document
        QDomElement eleDoc = doc.documentElement();

        // stylesheet
        std::string style;
        ctemplate::TemplateDictionary stylesheet("style");
        stylesheet.SetValue("FONTFAMILY", QApplication::font().family().toStdString());
        stylesheet.SetValue("FONTSIZE", (QString("%1").arg(QApplication::font().pointSize() - 1).toStdString()));

        ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/problem_style.tpl", ctemplate::DO_NOT_STRIP, &stylesheet, &style);

        // template
        std::string info;
        ctemplate::TemplateDictionary problemInfo("info");

        // problem info
        QDomNode eleProblemInfo = eleDoc.elementsByTagName("problem").at(0);

        problemInfo.SetValue("STYLESHEET", style);
        problemInfo.SetValue("BASIC_INFORMATION_LABEL", tr("Basic informations").toStdString());

        problemInfo.SetValue("NAME_LABEL", tr("Name:").toStdString());
        problemInfo.SetValue("NAME", eleProblemInfo.toElement().attribute("name").toStdString());

        problemInfo.SetValue("COORDINATE_TYPE_LABEL", tr("Coordinate type:").toStdString());
        problemInfo.SetValue("COORDINATE_TYPE", coordinateTypeString(coordinateTypeFromStringKey(eleProblemInfo.toElement().attribute("coordinate_type"))).toStdString());

        // geometry
        QDomNode eleGeometry = eleDoc.elementsByTagName("geometry").at(0);

        // nodes
        QDomNode eleNodes = eleGeometry.toElement().elementsByTagName("nodes").at(0);
        QDomNode nodeNode = eleNodes.firstChild();
        QList<SceneNode *> nodes;
        while (!nodeNode.isNull())
        {
            QDomElement element = nodeNode.toElement();

            Point point = Point(element.attribute("x").toDouble(),
                                element.attribute("y").toDouble());

            nodes.append(new SceneNode(point));

            nodeNode = nodeNode.nextSibling();
        }

        // edges
        QList<SceneEdge *> edges;
        QDomNode eleEdges = eleGeometry.toElement().elementsByTagName("edges").at(0);
        QDomNode nodeEdge = eleEdges.firstChild();
        while (!nodeEdge.isNull())
        {
            QDomElement element = nodeEdge.toElement();

            SceneNode *nodeFrom = nodes.at(element.attribute("start").toInt());
            SceneNode *nodeTo = nodes.at(element.attribute("end").toInt());
            double angle = element.attribute("angle", "0").toDouble();

            SceneEdge *edge = new SceneEdge(nodeFrom, nodeTo, angle);
            edges.append(edge);

            nodeEdge = nodeEdge.nextSibling();
        }

        // labels
        QDomNode eleLabels = eleGeometry.toElement().elementsByTagName("labels").at(0);

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
        problemInfo.SetValue("GEOMETRY_NODES", QString::number(eleNodes.childNodes().count()).toStdString());
        problemInfo.SetValue("GEOMETRY_EDGES_LABEL", tr("Edges:").toStdString());
        problemInfo.SetValue("GEOMETRY_EDGES", QString::number(eleEdges.childNodes().count()).toStdString());
        problemInfo.SetValue("GEOMETRY_LABELS_LABEL", tr("Labels:").toStdString());
        problemInfo.SetValue("GEOMETRY_LABELS", QString::number(eleLabels.childNodes().count()).toStdString());
        problemInfo.SetValue("GEOMETRY_SVG", geometry.toStdString());

        // description
        QDomNode eleDescription = eleProblemInfo.toElement().elementsByTagName("description").at(0);
        problemInfo.SetValue("DESCRIPTION", eleDescription.toElement().text().toStdString());

        problemInfo.SetValue("PHYSICAL_FIELD_MAIN_LABEL", tr("Physical fields").toStdString());

        QDomNode eleFields = eleProblemInfo.toElement().elementsByTagName("fields").at(0);
        QDomNode nodeField = eleFields.firstChild();
        while (!nodeField.isNull())
        {
            QDomNode eleField = nodeField.toElement();

            ctemplate::TemplateDictionary *field = problemInfo.AddSectionDictionary("FIELD_SECTION");

            field->SetValue("PHYSICAL_FIELD_LABEL", availableModules()[eleField.toElement().attribute("field_id")].toStdString());

            field->SetValue("ANALYSIS_TYPE_LABEL", tr("Analysis:").toStdString());
            field->SetValue("ANALYSIS_TYPE", analysisTypeString(analysisTypeFromStringKey(eleField.toElement().attribute("analysis_type"))).toStdString());

            // linearity
            QDomNode eleFieldLinearity = eleField.toElement().elementsByTagName("solver").at(0);

            field->SetValue("LINEARITY_TYPE_LABEL", tr("Solver:").toStdString());
            field->SetValue("LINEARITY_TYPE", linearityTypeString(linearityTypeFromStringKey(eleFieldLinearity.toElement().attribute("linearity_type"))).toStdString());

            problemInfo.ShowSection("FIELD");

            // next field
            nodeField = nodeField.nextSibling();
        }

        // image
        QFileInfo fileInfo(fileName);
        QString imageFilename(QString("%1/%2.png").arg(fileInfo.absolutePath()).arg(fileInfo.baseName()));
        if (QFile::exists(imageFilename))
        {
            problemInfo.ShowSection("IMAGE_SECTION");
            problemInfo.SetValue("IMAGE_LABEL", tr("Image").toStdString());
            problemInfo.SetValue("EXAMPLE_IMAGE", imageFilename.toStdString());
        }

        ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/example.tpl", ctemplate::DO_NOT_STRIP, &problemInfo, &info);

        // setHtml(...) doesn't work
        // webView->setHtml(QString::fromStdString(info));

        // load(...) works
        writeStringContent(tempProblemDir() + "/example.html", QString::fromStdString(info));
        webView->load(tempProblemDir() + "/example.html");
    }
}
