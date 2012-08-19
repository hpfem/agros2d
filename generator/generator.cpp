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

#include "generator.h"

#include "ctemplate/template.h"

const QString GENERATOR_ROOT = "generator";
const QString GENERATOR_TEMPLATEROOT = "templates";
const QString GENERATOR_PLUGINROOT = "plugins";

Agros2DGenerator::Agros2DGenerator(int &argc, char **argv) : QCoreApplication(argc, argv)
{
    QDir appDir(QString("%1/%2").arg(QApplication::applicationDirPath()).arg(GENERATOR_ROOT));
    appDir.mkdir(GENERATOR_PLUGINROOT);
}

void Agros2DGenerator::run()
{
    QString module = "electrostatic";

    // read module
    m_module_xsd = XMLModule::module_((datadir().toStdString() + MODULEROOT.toStdString() + "/" + module.toStdString() + ".xml").c_str());
    m_module = m_module_xsd.get();

    generateProjectFile(module);

    exit(0);
}

void Agros2DGenerator::generateProjectFile(const QString &id)
{
    // create directory
    QDir dir(QString("%1/%2/%3").arg(QApplication::applicationDirPath()).arg(GENERATOR_ROOT).arg(GENERATOR_PLUGINROOT));
    dir.mkdir(id);

    ctemplate::TemplateDictionary output("output");
    output.SetValue("ID", id.toStdString());

    for (int i = 0; i < m_module->general().analyses().analysis().size(); i++)
    {
        XMLModule::analysis analysis = m_module->general().analyses().analysis().at(i);

        foreach (QString coordinate, coordinateList())
        {
            ctemplate::TemplateDictionary *field = output.AddSectionDictionary("SOURCE");

            field->SetValue("ID", id.toStdString());
            field->SetValue("ANALYSIS_TYPE", analysis.id());
            field->SetValue("COORDINATE_TYPE", coordinate.toStdString());
        }
    }

    std::string text;
    ctemplate::ExpandTemplate(QString("%1/%2/%3/module_pro.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_ROOT).arg(GENERATOR_TEMPLATEROOT).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // TODO: -> function
    // save to file
    QFile file(QString("%1/%2/%3/%4/%4.pro").
               arg(QApplication::applicationDirPath()).
               arg(GENERATOR_ROOT).
               arg(GENERATOR_PLUGINROOT).
               arg(id));
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << QString::fromStdString(text);
    file.close();
}

