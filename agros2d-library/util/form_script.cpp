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
#include "util/constants.h"

#include "pythonlab/pythonengine_agros.h"

#include "../../resources_source/classes/form_xml.h"

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
    }
    else
    {
        mainWidget = new QLabel(tr("File '%1' doesn't exists.").arg(fileName));
    }

    foreach (QWidget *object, mainWidget->findChildren<QWidget *>())
    {
        // line edit
        if (QLineEdit *widget = dynamic_cast<QLineEdit *>(object))
        {
            // trick - double validator
            if (widget->accessibleName() == "double")
            {
                widget->setValidator(new QDoubleValidator(widget));
                widget->setAccessibleName("");
            }
            // trick - int validator
            if (widget->accessibleName() == "int")
            {
                widget->setValidator(new QIntValidator(widget));
                widget->setAccessibleName("");
            }
        }
    }

    // dialog buttons
    QPushButton *btnLoad = new QPushButton(tr("Load"));
    btnLoad->setDefault(false);
    connect(btnLoad, SIGNAL(clicked()), this, SLOT(load()));
    QPushButton *btnSave = new QPushButton(tr("Save"));
    btnSave->setDefault(false);
    connect(btnSave, SIGNAL(clicked()), this, SLOT(save()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->addButton(btnLoad, QDialogButtonBox::ActionRole);
    buttonBox->addButton(btnSave, QDialogButtonBox::ActionRole);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(acceptForm()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(rejectForm()));

    // error
    errorMessage = new QLabel();
    errorMessage->setVisible(false);
    QPalette palette = errorMessage->palette();
    palette.setColor(QPalette::WindowText, Qt::red);
    errorMessage->setPalette(palette);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mainWidget);
    layout->addStretch();
    layout->addWidget(errorMessage);
    layout->addWidget(buttonBox);
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
    errorMessage->clear();
    errorMessage->setVisible(false);

    return exec();
}

QString FormScript::valueForWidget(XMLForm::form *doc, const QString &objectName, const QString &defaultValue)
{
    for (int i = 0; i < doc->config().item().size(); i++)
    {
        XMLForm::item item = doc->config().item().at(i);

        if (objectName.toStdString() == item.key())
            return QString::fromStdString(item.value());
    }

    return defaultValue;
}

void FormScript::load()
{
    QSettings settings;
    QString dir = settings.value("General/LastProblemDir", "data").toString();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), dir, tr("Agros2D form files (*.frm)"));

    if (fileName.isEmpty()) return;

    if (QFile::exists(fileName))
    {
        std::auto_ptr<XMLForm::form> form_xsd = XMLForm::form_(compatibleFilename(fileName).toStdString(), xml_schema::flags::dont_validate);
        XMLForm::form *doc = form_xsd.get();

        foreach (QWidget *object, mainWidget->findChildren<QWidget *>())
        {
            // line edit
            if (QLineEdit *widget = dynamic_cast<QLineEdit *>(object))
                widget->setText(valueForWidget(doc, widget->objectName(), ""));
            // radio button
            if (QRadioButton *widget = dynamic_cast<QRadioButton  *>(object))
                widget->setChecked(valueForWidget(doc, widget->objectName(), "False") == "True");
            // checkbox
            if (QCheckBox *widget = dynamic_cast<QCheckBox  *>(object))
                widget->setChecked(valueForWidget(doc, widget->objectName(), "False") == "True");
            // combobox
            if (QComboBox *widget = dynamic_cast<QComboBox  *>(object))
            {
                widget->setCurrentIndex(widget->findText(valueForWidget(doc, widget->objectName(), "")));
                if ((widget->count() > 0) && (widget->currentIndex() == -1))
                    widget->setCurrentIndex(0);
            }
            // spinbox
            if (QSpinBox *widget = dynamic_cast<QSpinBox  *>(object))
                widget->setValue(valueForWidget(doc, widget->objectName(), "0").toInt());
            // double spinbox
            if (QDoubleSpinBox *widget = dynamic_cast<QDoubleSpinBox  *>(object))
                widget->setValue(valueForWidget(doc, widget->objectName(), "0.0").toDouble());
        }
    }
}

void FormScript::save()
{
    QSettings settings;
    QString dir = settings.value("General/LastProblemDir", "data").toString();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save file"), dir, tr("Agros2D form files (*.frm)"));

    if (fileName.isEmpty()) return;

    QFileInfo fileInfo(fileName);
    if (fileInfo.suffix().toLower() != "frm") fileName += ".frm";

    XMLForm::config config;

    foreach (QWidget *object, mainWidget->findChildren<QWidget *>())
    {
        // line edit
        if (QLineEdit *widget = dynamic_cast<QLineEdit  *>(object))
            config.item().push_back(XMLForm::item(widget->objectName().toStdString(), widget->text().toStdString()));
        // radio button
        if (QRadioButton *widget = dynamic_cast<QRadioButton  *>(object))
            config.item().push_back(XMLForm::item(widget->objectName().toStdString(), widget->isChecked() ? "True" : "False"));
        // checkbox
        if (QCheckBox *widget = dynamic_cast<QCheckBox  *>(object))
            config.item().push_back(XMLForm::item(widget->objectName().toStdString(), widget->isChecked() ? "True" : "False"));
        // combobox
        if (QComboBox *widget = dynamic_cast<QComboBox  *>(object))
            config.item().push_back(XMLForm::item(widget->objectName().toStdString(), widget->currentText().toStdString()));
        // spinbox
        if (QSpinBox *widget = dynamic_cast<QSpinBox  *>(object))
            config.item().push_back(XMLForm::item(widget->objectName().toStdString(), QString::number(widget->value()).toStdString()));
        // double spinbox
        if (QDoubleSpinBox *widget = dynamic_cast<QDoubleSpinBox  *>(object))
            config.item().push_back(XMLForm::item(widget->objectName().toStdString(), QString::number(widget->value()).toStdString()));
    }

    XMLForm::form doc(config);

    std::string problem_schema_location("");

    problem_schema_location.append(QString("%1/form_xml.xsd").arg(QFileInfo(datadir() + XSDROOT).absoluteFilePath()).toStdString());
    ::xml_schema::namespace_info namespace_info_problem("XMLForm", problem_schema_location);

    ::xml_schema::namespace_infomap namespace_info_map;
    namespace_info_map.insert(std::pair<std::basic_string<char>, xml_schema::namespace_info>("problem", namespace_info_problem));

    std::ofstream out(compatibleFilename(fileName).toStdString().c_str());
    XMLForm::form_(out, doc, namespace_info_map);
}

void FormScript::acceptForm()
{
    QString scriptFileName = QString("%1/%2.py").arg(QFileInfo(fileName).absolutePath()).arg(QFileInfo(fileName).baseName());

    if (QFile::exists(scriptFileName))
    {
        std::string info;
        ctemplate::TemplateDictionary script("script");

        foreach (QWidget *object, mainWidget->findChildren<QWidget *>())
        {
            // line edit
            if (QLineEdit *widget = dynamic_cast<QLineEdit *>(object))
            {
                script.SetValue(QString("%1_text").arg(widget->objectName()).toStdString(), widget->text().toStdString());
            }
            // radio button
            if (QRadioButton *widget = dynamic_cast<QRadioButton *>(object))
            {
                script.SetValue(QString("%1_checked").arg(widget->objectName()).toStdString(), widget->isChecked() ? "True" : "False");
            }
            // checkbox
            if (QCheckBox *widget = dynamic_cast<QCheckBox *>(object))
            {
                script.SetValue(QString("%1_checked").arg(widget->objectName()).toStdString(), widget->isChecked() ? "True" : "False");
            }
            // combobox
            if (QComboBox *widget = dynamic_cast<QComboBox *>(object))
            {
                script.SetValue(QString("%1_index").arg(widget->objectName()).toStdString(), QString::number(widget->currentIndex()).toStdString());
                script.SetValue(QString("%1_text").arg(widget->objectName()).toStdString(), widget->currentText().toStdString());
            }
            // spinbox
            if (QSpinBox *widget = dynamic_cast<QSpinBox *>(object))
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
        ScriptResult result = currentPythonEngineAgros()->runScript(QString::fromStdString(info));

        if (result.isError)
        {
            errorMessage->setVisible(true);
            errorMessage->setText(result.text + "\n" + result.traceback);
        }
        else
        {
            accept();
        }
    }
    else
    {
        accept();
    }
}

void FormScript::rejectForm()
{    
    reject();
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(agros2d_form_script, FormScript)
#endif
