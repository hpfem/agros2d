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

#ifdef _MSC_VER
# ifdef _DEBUG
#  undef _DEBUG
#  include <Python.h>
#  define _DEBUG
# else
#  include <Python.h>
# endif
#else
#  include <Python.h>
#endif

#include "optilab_single.h"

#include "pythonlab/pythonengine_agros.h"
#include "util/constants.h"
#include "gui/common.h"

#include "ctemplate/template.h"

OptilabSingle::OptilabSingle(OptilabWindow *parent) : QWidget(parent), optilabMain(parent)
{
    // problem information
    webView = new QWebView();
    webView->page()->setNetworkAccessManager(new QNetworkAccessManager());
    webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    connect(webView->page(), SIGNAL(linkClicked(QUrl)), this, SLOT(linkClicked(QUrl)));

    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", htmlFontFamily().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(htmlFontSize()).toStdString()));

    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/style_common.css").toStdString(), ctemplate::DO_NOT_STRIP, &stylesheet, &style);
    m_cascadeStyleSheet = QString::fromStdString(style);

    QHBoxLayout *layout = new QHBoxLayout();
    // layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(webView);

    setLayout(layout);
}

void OptilabSingle::variantInfo(const QString &key)
{
    QString str = QString("agros2d_model = variant.optilab_interface._md_model('%1')").arg(key);
    currentPythonEngine()->runExpression(str);
    // qDebug() << key << str << successfulRun;

    // extract values
    PyObject *result = PyDict_GetItemString(currentPythonEngine()->dict(), "agros2d_model");
    if (result)
    {
        Py_INCREF(result);

        // template
        ctemplate::TemplateDictionary variant("model");

        // problem info
        variant.SetValue("AGROS2D", "file:///" + compatibleFilename(QDir(datadir() + TEMPLATEROOT + "/panels/agros2d_logo.png").absolutePath()).toStdString());

        variant.SetValue("STYLESHEET", m_cascadeStyleSheet.toStdString());
        variant.SetValue("PANELS_DIRECTORY", QUrl::fromLocalFile(QString("%1%2").arg(QDir(datadir()).absolutePath()).arg(TEMPLATEROOT + "/panels")).toString().toStdString());
        variant.SetValue("BASIC_INFORMATION_LABEL", tr("Basic informations").toStdString());

        variant.SetValue("NAME_LABEL", tr("Name:").toStdString());
        variant.SetValue("NAME", QFileInfo(key).baseName().toStdString());

        variant.SetValue("IMAGES_LABEL", tr("Geometry:").toStdString());

        // input parameters
        PyObject *input = PyObject_GetAttrString(result, "parameters");
        if (input)
        {
            Py_INCREF(input);
            variant.SetValue("PARAMETER_LABEL", tr("Input parameters").toStdString());

            PyObject *key, *value;
            Py_ssize_t pos = 0;

            while (PyDict_Next(input, &pos, &key, &value))
            {
                QString name = QString::fromWCharArray(PyUnicode_AsUnicode(key));
                double val = PyFloat_AsDouble(value);

                ctemplate::TemplateDictionary *paramSection = variant.AddSectionDictionary("PARAM_SECTION");

                paramSection->SetValue("PARAM_LABEL", name.toStdString());
                paramSection->SetValue("PARAM_VALUE", QString::number(val).toStdString());
                /*
                QString tp = QString(value->ob_type->tp_name);
                qDebug() << tp;
                // variable value
                if (tp == "bool")
                {
                    paramSection->SetValue("PARAM_VALUE", PyLong_AsLong(value) ? "True" : "False");
                }
                else if (tp == "int")
                {
                    paramSection->SetValue("PARAM_VALUE", QString::number((int) PyLong_AsLong(value)).toStdString());
                }
                else if (tp == "float" || tp == "numpy.float64")
                {
                    paramSection->SetValue("PARAM_VALUE", QString::number(PyFloat_AsDouble(value)).toStdString());
                }
                else if (tp == "str")
                {
                    paramSection->SetValue("PARAM_VALUE", QString::fromWCharArray(PyUnicode_AsUnicode(value)).toStdString());
                }
                else if (tp == "list")
                {
                    paramSection->SetValue("PARAM_VALUE", QString::number((int) PyList_Size(value)).toStdString());
                }
                else if (tp == "tuple")
                {
                    paramSection->SetValue("PARAM_VALUE", QString::number((int) PyTuple_Size(value)).toStdString());
                }
                else if (tp == "dict")
                {
                    paramSection->SetValue("PARAM_VALUE", QString::number((int) PyDict_Size(value)).toStdString());
                }
                else
                {
                    qDebug() << "Unknown type:" << tp;
                }
                */
                // paramSection->SetValue("PARAM_UNIT", parameter.param_unit());
            }
            Py_XDECREF(input);
        }
        /*
        // info
        PyObject *info = PyObject_GetAttrString(result, "info");
        if (info)
        {
            Py_INCREF(info);
            variant.SetValue("INFO_LABEL", tr("Variant info").toStdString());

            PyObject *key, *value;
            Py_ssize_t pos = 0;

            while (PyDict_Next(info, &pos, &key, &value))
            {
                QString name = QString::fromWCharArray(PyUnicode_AsUnicode(key));
                double val = PyFloat_AsDouble(value);

                ctemplate::TemplateDictionary *paramSection = variant.AddSectionDictionary("INFO_SECTION");

                paramSection->SetValue("INFO_LABEL", name.toStdString());
                paramSection->SetValue("INFO_VALUE", QString::number(val).toStdString());
            }
            Py_XDECREF(info);
        }
        */
        QString templateName = "variant.tpl";
        std::string output;
        ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/" + templateName.toStdString(), ctemplate::DO_NOT_STRIP, &variant, &output);

        // setHtml(...) doesn't work
        // webView->setHtml(QString::fromStdString(info));

        // load(...) works
        writeStringContent(tempProblemDir() + "/variant.html", QString::fromStdString(output));
        webView->load(QUrl::fromLocalFile(tempProblemDir() + "/variant.html"));        

        Py_XDECREF(result);
    }

    /*
    // TODO: more images
    QDomElement eleGeometry = nodeImages.childNodes().at(0).toElement();
    QString geometry = eleGeometry.attribute("source");
    if (!geometry.isEmpty())
        info.SetValue("IMAGE", geometry.toStdString());

    info.SetValue("SOLVED", (nodeSolution.toElement().attribute("solved").toInt() == 1) ? "YES" : "NO");

    // output
    info.SetValue("VARIABLE_LABEL", tr("Output variables").toStdString());
    for (unsigned int i = 0; i < nodeOutput.childNodes().count(); i++)
    {
        QDomElement eleVariable = nodeOutput.childNodes().at(i).toElement();

        OutputVariable result(eleVariable.attribute("name"),
                              eleVariable.attribute("value"));

        if (result.isNumber())
        {
            ctemplate::TemplateDictionary *varSection = info.AddSectionDictionary("VAR_VALUE_SECTION");

            // double value
            varSection->SetValue("VAR_LABEL", result.name().toStdString());
            varSection->SetValue("VAR_VALUE", QString::number(result.number()).toStdString());
            // varSection->SetValue("VAR_UNIT", variable.var_unit());
        }
        else
        {
            ctemplate::TemplateDictionary *varSection = info.AddSectionDictionary("VAR_CHART_SECTION");

            QString chartData = "[";
            for (int j = 0; j < result.size(); j++)
                chartData += QString("[%1, %2], ").arg(result.x().at(j)).arg(result.y().at(j));
            chartData += "]";

            // chart time step vs. steps
            QString chart = QString("<script type=\"text/javascript\">$(function () { $.plot($(\"#chart_%1\"), [ { data: %2, color: \"rgb(61, 61, 251)\", lines: { show: true }, points: { show: true } } ], { grid: { hoverable : true }, xaxes: [ { axisLabel: 'N' } ], yaxes: [ { axisLabel: '%3' } ] });});</script>").
                    arg(i).
                    arg(chartData).
                    arg(result.name());

            varSection->SetValue("VAR_CHART_DIV", QString("chart_%1").arg(i).toStdString());
            varSection->SetValue("VAR_CHART", chart.toStdString());
        }
    }

    // info
    info.SetValue("INFO_LABEL", tr("Variant info").toStdString());
    for (unsigned int i = 0; i < nodeInfo.childNodes().count(); i++)
    {
        ctemplate::TemplateDictionary *infoSection = info.AddSectionDictionary("INFO_SECTION");

        QDomElement eleInfo = nodeInfo.childNodes().at(i).toElement();

        infoSection->SetValue("INFO_LABEL", eleInfo.attribute("name").toStdString());
        infoSection->SetValue("INFO_VALUE", eleInfo.attribute("value").toStdString());
    }
    */
}

void OptilabSingle::welcomeInfo()
{
    // template
    std::string info;
    ctemplate::TemplateDictionary variants("variants");

    variants.SetValue("AGROS2D", "file:///" + compatibleFilename(QDir(datadir() + TEMPLATEROOT + "/panels/agros2d_logo.png").absolutePath()).toStdString());

    variants.SetValue("STYLESHEET", m_cascadeStyleSheet.toStdString());
    variants.SetValue("PANELS_DIRECTORY", QUrl::fromLocalFile(QString("%1%2").arg(QDir(datadir()).absolutePath()).arg(TEMPLATEROOT + "/panels")).toString().toStdString());

    // recent problem files
    variants.SetValue("RECENT_PROBLEMS_LABEL", tr("Recent Problems").toStdString());
    for (int i = 0; i < qMin(10, optilabMain->recentFiles.count()); i++)
    {
        ctemplate::TemplateDictionary *recent = variants.AddSectionDictionary("RECENT_PROBLEM_SECTION");
        recent->SetValue("PROBLEM_FILENAME", QUrl::fromUserInput(optilabMain->recentFiles.at(i)).toString().toStdString());
        recent->SetValue("PROBLEM_FILENAME_LABEL", QFileInfo(optilabMain->recentFiles.at(i)).absolutePath().replace("/", "/&thinsp;").toStdString());
        recent->SetValue("PROBLEM_BASE", QFileInfo(optilabMain->recentFiles.at(i)).baseName().toStdString());
    }

    // links
    variants.SetValue("LINKS_LABEL", tr("Links").toStdString());

    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/optilab.tpl").toStdString(), ctemplate::DO_NOT_STRIP, &variants, &info);

    // setHtml(...) doesn't work
    // webView->setHtml(QString::fromStdString(info));

    // load(...) works
    writeStringContent(tempProblemDir() + "/variants.html", QString::fromStdString(info));
    webView->load(QUrl::fromLocalFile(tempProblemDir() + "/variants.html"));
}

void OptilabSingle::linkClicked(const QUrl &url)
{
    QString search = "/open?";
    if (url.toString().contains(search))
    {
#if QT_VERSION < 0x050000
        QString fileName = url.queryItemValue("filename");
#else
        QString fileName = QUrlQuery(url).queryItemValue("filename");
#endif

        if (QFile::exists(QUrl(fileName + "/problem.py").toLocalFile()))
            optilabMain->documentOpen(QUrl(fileName + "/problem.py").toLocalFile());
    }
}

void OptilabSingle::doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    webView->setHtml("");

    if (current)
    {
        variantInfo(current->data(0, Qt::UserRole).toString());
    }
}
