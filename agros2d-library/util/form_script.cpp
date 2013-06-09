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

#include <QUiLoader>
#include "form_script.h"

#include "ctemplate/template.h"

#include "util.h"
#include "pythonlab/pythonengine_agros.h"

FormScript::FormScript(const QString &fileName, QWidget *parent)
    : FormInterface(parent), fileName(fileName)
{
    QString name = QFileInfo(fileName).baseName();
    setWindowTitle(name);

    actShow = new QAction((name), this);
    connect(actShow, SIGNAL(triggered()), this, SLOT(show()));

    if (QFile::exists(fileName))
    {
        QUiLoader loader;
        QFile file(fileName);
        file.open(QFile::ReadOnly);
        mainWidget = loader.load(&file, this);
        file.close();

        foreach (QObject *object, mainWidget->children())
        {
            if (QDialogButtonBox *buttonBox = dynamic_cast<QDialogButtonBox  *>(object))
            {
                connect(buttonBox, SIGNAL(accepted()), this, SLOT(acceptForm()));
                connect(buttonBox, SIGNAL(rejected()), this, SLOT(rejectForm()));
            }
        }
    }
    else
    {
        mainWidget = new QLabel(tr("File '%1' doesn't exists.").arg(fileName));
    }

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mainWidget);
    setLayout(layout);
}

FormScript::~FormScript()
{
}

QString FormScript::formId()
{
    return QString("script_").arg(QFileInfo(fileName).baseName());
}

QAction *FormScript::action()
{
    return actShow;
}

int FormScript::show()
{
    return exec();
}

void FormScript::acceptForm()
{
    QString scriptFileName = QString("%1/%2.py").arg(QFileInfo(fileName).absolutePath()).arg(QFileInfo(fileName).baseName());

    if (QFile::exists(scriptFileName))
    {
        std::string info;
        ctemplate::TemplateDictionary script("script");

        foreach (QObject *object, mainWidget->children())
        {
            // line edit
            if (QLineEdit *widget = dynamic_cast<QLineEdit  *>(object))
            {
                script.SetValue(QString("%1_text").arg(widget->objectName()).toStdString(), widget->text().toStdString());
            }
            // radio button
            if (QRadioButton *widget = dynamic_cast<QRadioButton  *>(object))
            {
                script.SetValue(QString("%1_checked").arg(widget->objectName()).toStdString(), widget->isChecked() ? "True" : "False");
            }
            // radio button
            if (QCheckBox *widget = dynamic_cast<QCheckBox  *>(object))
            {
                script.SetValue(QString("%1_checked").arg(widget->objectName()).toStdString(), widget->isChecked() ? "True" : "False");
            }
            // combobox
            if (QComboBox *widget = dynamic_cast<QComboBox  *>(object))
            {
                script.SetValue(QString("%1_index").arg(widget->objectName()).toStdString(), QString::number(widget->currentIndex()).toStdString());
                script.SetValue(QString("%1_text").arg(widget->objectName()).toStdString(), widget->currentText().toStdString());
            }
            // spinbox
            if (QSpinBox *widget = dynamic_cast<QSpinBox  *>(object))
            {
                script.SetValue(QString("%1_value").arg(widget->objectName()).toStdString(), QString::number(widget->value()).toStdString());
            }
            // double spinbox
            if (QDoubleSpinBox *widget = dynamic_cast<QDoubleSpinBox  *>(object))
            {
                script.SetValue(QString("%1_value").arg(widget->objectName()).toStdString(), QString::number(widget->value()).toStdString());
            }
        }

        ctemplate::ExpandTemplate(compatibleFilename(scriptFileName).toStdString(),
                                  ctemplate::DO_NOT_STRIP, &script, &info);

        writeStringContent(tempProblemDir() + "/script.py", QString::fromStdString(info));
        currentPythonEngineAgros()->runScript(QString::fromStdString(info));
    }

    accept();
}

void FormScript::rejectForm()
{    
    reject();
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(agros2d_form_script, FormScript)
#endif
