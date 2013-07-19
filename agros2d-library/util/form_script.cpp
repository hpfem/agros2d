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

#include "util/global.h"
#include "scene.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "pythonlab/pythonengine_agros.h"

#include "../../resources_source/classes/form_xml.h"

FormScript::FormScript(const QString &fileName, PythonScriptingConsoleView *consoleView, QWidget *parent)
    : FormInterface(parent), m_fileName(fileName), consoleView(consoleView), mainWidget(NULL)
{
    // setAttribute(Qt::WA_DeleteOnClose);

    actShow = new QAction(this);
    connect(actShow, SIGNAL(triggered()), this, SLOT(show()));

    process = new QProcess();
    connect(process, SIGNAL(finished(int)), this, SLOT(designerFinished(int)));

    // dialog buttons
    btnMore = new QPushButton(tr("More..."));

    QAction *actLoad = new QAction(tr("Load"), this);
    connect(actLoad, SIGNAL(triggered()), this, SLOT(loadFromFile()));
    QAction *actSave = new QAction(tr("Save"), this);
    connect(actSave, SIGNAL(triggered()), this, SLOT(saveToFile()));

    QAction *actReload = new QAction(tr("Reload"), this);
    connect(actReload, SIGNAL(triggered()), this, SLOT(reloadWidget()));
    QAction *actDesigner = new QAction(tr("Designer"), this);
    connect(actDesigner, SIGNAL(triggered()), this, SLOT(designer()));

    menu = new QMenu();
    menu->addAction(actLoad);
    menu->addAction(actSave);
    menu->addSeparator();
    menu->addAction(actReload);
    menu->addAction(actDesigner);

    btnMore->setMenu(menu);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->addButton(btnMore, QDialogButtonBox::ActionRole);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(acceptForm()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(rejectForm()));

    // error
    errorMessage = new QLabel();
    errorMessage->setVisible(false);
    QPalette palette = errorMessage->palette();
    palette.setColor(QPalette::WindowText, Qt::red);
    errorMessage->setPalette(palette);

    QHBoxLayout *buttons = new QHBoxLayout();
    buttons->addWidget(buttonBox);

    QVBoxLayout *layout = new QVBoxLayout();
    // layout->addStretch();
    layout->addWidget(errorMessage);
    layout->addLayout(buttons);
    setLayout(layout);

    loadWidget(fileName);

    QSettings settings;
    restoreGeometry(settings.value(QString("FormScript/Geometry_%1").arg(QFileInfo(fileName).baseName()), saveGeometry()).toByteArray());
}

FormScript::~FormScript()
{
    QSettings settings;
    settings.setValue(QString("FormScript/Geometry_%1").arg(QFileInfo(m_fileName).baseName()), saveGeometry());

    delete process;
}

void FormScript::loadWidget(const QString &fileName)
{
    QString name = QFileInfo(fileName).baseName();

    setWindowTitle(name);
    actShow->setText(name);

    if (mainWidget)
    {
        layout()->removeWidget(mainWidget);
        delete mainWidget;
    }

    if (QFile::exists(fileName))
    {
        QUiLoader loader;
        // loader.setWorkingDirectory(QDir(datadir() + "/resources/forms"));
        loader.setWorkingDirectory(QFileInfo(fileName).absoluteDir());
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
            // double validator
            if (widget->property("dataType") == "double")
            {
                widget->setValidator(new QRegExpValidator(QRegExp("^[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?")));
                // widget->setValidator(new QDoubleValidator(widget));
            }
            // int validator
            if (widget->property("dataType") == "int")
            {
                widget->setValidator(new QIntValidator(widget));
            }
        }
        // tree widget
        if (QTreeWidget *widget = dynamic_cast<QTreeWidget *>(object))
        {
            // postprocessor - local values, surface integrals, volume integrals
            if (widget->property("dataType") == "local_values" || (widget->property("dataType") == "surface_integrals") || (widget->property("dataType") == "volume_integrals"))
            {
                widget->setHeaderHidden(true);
            }
        }
    }

    dynamic_cast<QVBoxLayout *>(layout())->insertWidget(0, mainWidget);
}

void FormScript::reloadWidget()
{
    loadWidget(m_fileName);
    showWidget();
}

void FormScript::showWidget()
{
    foreach (QWidget *object, mainWidget->findChildren<QWidget *>())
    {
        // list widget
        if (QListWidget *widget = dynamic_cast<QListWidget *>(object))
        {
            // preprocessor - labels
            if (widget->property("dataType") == "nodes")
            {
                QList<int> nodes;
                for (int i = 0; i < widget->count(); i++)
                    if (widget->item(i)->checkState() == Qt::Checked)
                        nodes.append(i);

                widget->clear();

                int i = 0;
                foreach (SceneNode *node, Agros2D::scene()->nodes->items())
                {
                    QListWidgetItem *item = new QListWidgetItem(widget);

                    item->setText(QString("%1 - [%2; %3]").
                                  arg(i).
                                  arg(node->point().x, 0, 'e', 2).
                                  arg(node->point().y, 0, 'e', 2));
                    item->setData(Qt::UserRole, node->variant());
                    if (nodes.contains(i))
                        item->setCheckState(Qt::Checked);
                    else
                        item->setCheckState(Qt::Unchecked);

                    widget->addItem(item);

                    i++;
                }
            }
            // preprocessor - edges
            if (widget->property("dataType") == "edges")
            {
                QList<int> edges;
                for (int i = 0; i < widget->count(); i++)
                    if (widget->item(i)->checkState() == Qt::Checked)
                        edges.append(i);

                widget->clear();

                int i = 0;
                foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
                {
                    QListWidgetItem *item = new QListWidgetItem(widget);

                    item->setText(QString("%1 - %2 m").
                                  arg(i).
                                  arg((edge->angle() < EPS_ZERO) ?
                                          sqrt(Hermes::sqr(edge->nodeEnd()->point().x - edge->nodeStart()->point().x) + Hermes::sqr(edge->nodeEnd()->point().y - edge->nodeStart()->point().y)) :
                                          edge->radius() * edge->angle() / 180.0 * M_PI, 0, 'e', 2));
                    item->setData(Qt::UserRole, edge->variant());
                    if (edges.contains(i))
                        item->setCheckState(Qt::Checked);
                    else
                        item->setCheckState(Qt::Unchecked);

                    widget->addItem(item);

                    i++;
                }
            }
            // preprocessor - labels
            if (widget->property("dataType") == "labels")
            {
                QList<int> labels;
                for (int i = 0; i < widget->count(); i++)
                    if (widget->item(i)->checkState() == Qt::Checked)
                        labels.append(i);

                widget->clear();

                int i = 0;
                foreach (SceneLabel *label, Agros2D::scene()->labels->items())
                {
                    QListWidgetItem *item = new QListWidgetItem(widget);

                    item->setText(QString("%1 - [%2; %3]").
                                  arg(i).
                                  arg(label->point().x, 0, 'e', 2).
                                  arg(label->point().y, 0, 'e', 2));
                    item->setData(Qt::UserRole, label->variant());
                    item->setCheckState(Qt::Unchecked);
                    if (labels.contains(i))
                        item->setCheckState(Qt::Checked);
                    else
                        item->setCheckState(Qt::Unchecked);

                    widget->addItem(item);

                    i++;
                }
            }
        }
        // tree widget
        if (QTreeWidget *widget = dynamic_cast<QTreeWidget *>(object))
        {
            QFont fnt = widget->font();
            fnt.setBold(true);

            // postprocessor - local values
            if (widget->property("dataType") == "local_values")
            {
                QString selectedFieldid = "";
                QString selectedVariable = "";
                QTreeWidgetItem *selectedItem = NULL;
                if (widget->currentItem())
                {
                    selectedVariable = widget->currentItem()->data(0, Qt::UserRole).toString();
                    selectedFieldid = widget->currentItem()->data(1, Qt::UserRole).toString();
                }

                widget->clear();

                foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
                {
                    QTreeWidgetItem *itemField = new QTreeWidgetItem(widget);

                    itemField->setText(0, fieldInfo->name());
                    itemField->setData(0, Qt::UserRole, "");
                    itemField->setData(1, Qt::UserRole, "");
                    itemField->setFont(0, fnt);
                    itemField->setExpanded(true);

                    widget->addTopLevelItem(itemField);

                    foreach (Module::LocalVariable variable, fieldInfo->localPointVariables())
                    {
                        QTreeWidgetItem *item = new QTreeWidgetItem(itemField);

                        item->setText(0, variable.name());
                        item->setData(0, Qt::UserRole, variable.shortname());
                        item->setData(1, Qt::UserRole, fieldInfo->fieldId());

                        if ((!selectedVariable.isEmpty()) && (!selectedFieldid.isEmpty())
                                && (selectedVariable == variable.shortname()) && (selectedFieldid == fieldInfo->fieldId()))
                            selectedItem = item;
                    }
                }

                if (selectedItem)
                    widget->setCurrentItem(selectedItem);
            }
            // postprocessor - surface integrals
            if (widget->property("dataType") == "surface_integrals")
            {
                QString selectedFieldid = "";
                QString selectedVariable = "";
                QTreeWidgetItem *selectedItem = NULL;
                if (widget->currentItem())
                {
                    selectedVariable = widget->currentItem()->data(0, Qt::UserRole).toString();
                    selectedFieldid = widget->currentItem()->data(1, Qt::UserRole).toString();
                }

                widget->clear();

                foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
                {
                    QTreeWidgetItem *itemField = new QTreeWidgetItem(widget);

                    itemField->setText(0, fieldInfo->name());
                    itemField->setData(0, Qt::UserRole, "");
                    itemField->setData(1, Qt::UserRole, "");
                    itemField->setFont(0, fnt);
                    itemField->setExpanded(true);

                    widget->addTopLevelItem(itemField);

                    foreach (Module::Integral variable, fieldInfo->surfaceIntegrals())
                    {
                        QTreeWidgetItem *item = new QTreeWidgetItem(itemField);

                        item->setText(0, variable.name());
                        item->setData(0, Qt::UserRole, variable.shortname());
                        item->setData(1, Qt::UserRole, fieldInfo->fieldId());

                        if ((!selectedVariable.isEmpty()) && (!selectedFieldid.isEmpty())
                                && (selectedVariable == variable.shortname()) && (selectedFieldid == fieldInfo->fieldId()))
                            selectedItem = item;
                    }
                }

                if (selectedItem)
                    widget->setCurrentItem(selectedItem);

            }
            // postprocessor - volume integrals
            if (widget->property("dataType") == "volume_integrals")
            {
                QString selectedFieldid = "";
                QString selectedVariable = "";
                QTreeWidgetItem *selectedItem = NULL;
                if (widget->currentItem())
                {
                    selectedVariable = widget->currentItem()->data(0, Qt::UserRole).toString();
                    selectedFieldid = widget->currentItem()->data(1, Qt::UserRole).toString();
                }

                widget->clear();

                foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
                {
                    QTreeWidgetItem *itemField = new QTreeWidgetItem(widget);

                    itemField->setText(0, fieldInfo->name());
                    itemField->setData(0, Qt::UserRole, "");
                    itemField->setData(1, Qt::UserRole, "");
                    itemField->setFont(0, fnt);
                    itemField->setExpanded(true);

                    widget->addTopLevelItem(itemField);

                    foreach (Module::Integral variable, fieldInfo->volumeIntegrals())
                    {
                        QTreeWidgetItem *item = new QTreeWidgetItem(itemField);

                        item->setText(0, variable.name());
                        item->setData(0, Qt::UserRole, variable.shortname());
                        item->setData(1, Qt::UserRole, fieldInfo->fieldId());

                        if ((!selectedVariable.isEmpty()) && (!selectedFieldid.isEmpty())
                                && (selectedVariable == variable.shortname()) && (selectedFieldid == fieldInfo->fieldId()))
                            selectedItem = item;
                    }
                }

                if (selectedItem)
                    widget->setCurrentItem(selectedItem);
            }
        }
    }
}

void FormScript::designer()
{
    QString designerBinary = "designer";
    if (QFile::exists(QApplication::applicationDirPath() + QDir::separator() + "designer.exe"))
        designerBinary = "\"" + QApplication::applicationDirPath() + QDir::separator() + "designer.exe\"";
    if (QFile::exists(QApplication::applicationDirPath() + QDir::separator() + "designer"))
        designerBinary = QApplication::applicationDirPath() + QDir::separator() + "designer";

    process->start(QString("%1 \"%2\"").arg(designerBinary).arg(m_fileName));

    // execute an event loop to process the request (nearly-synchronous)
    QEventLoop eventLoop;
    connect(process, SIGNAL(finished(int)), &eventLoop, SLOT(quit()));
    eventLoop.exec();
}

void FormScript::designerFinished(int status)
{
    if (status == 0)
    {
        reloadWidget();
        showWidget();
    }
}

QString FormScript::formId()
{
    return QString("script_").arg(QFileInfo(m_fileName).baseName());
}

QAction *FormScript::action()
{
    return actShow;
}

int FormScript::show()
{
    return showForm();
}

int FormScript::showForm(const QString &fileName)
{
    errorMessage->clear();
    errorMessage->setVisible(false);

    showWidget();

    if (!fileName.isEmpty())
        loadFromFile(fileName);

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

void FormScript::loadFromFile(const QString &fileName)
{
    QString fn = fileName;

    if (fn.isEmpty())
    {
        QSettings settings;
        QString dir = settings.value("FormScript/LastProblemDir", "").toString();
        fn = QFileDialog::getOpenFileName(this, tr("Open file"), dir, tr("Agros2D form files (*.frm)"));
    }

    if (fn.isEmpty()) return;

    if (QFile::exists(fn))
    {
        QSettings settings;
        settings.setValue("FormScript/LastProblemDir", QFileInfo(fn).absolutePath());

        std::auto_ptr<XMLForm::form> form_xsd = XMLForm::form_(compatibleFilename(fn).toStdString(), xml_schema::flags::dont_validate);
        XMLForm::form *doc = form_xsd.get();

        foreach (QWidget *object, mainWidget->findChildren<QWidget *>())
        {
            if (object->objectName().startsWith("qt_"))
                continue;

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
            // check list widget
            if (QListWidget *widget = dynamic_cast<QListWidget *>(object))
            {
                QString itemsString = valueForWidget(doc, widget->objectName(), "");
                if (!itemsString.isEmpty())
                {
                    QStringList itemsStringList = itemsString.split(",");

                    QList<int> items;
                    foreach (QString itemString, itemsStringList)
                        items.append(itemString.toDouble());

                    for (int i = 0; i < widget->count(); i++)
                        if (items.contains(i))
                            widget->item(i)->setCheckState(Qt::Checked);
                        else
                            widget->item(i)->setCheckState(Qt::Unchecked);
                }
            }
            // local values, surface and volume integrals tree widget
            if (QTreeWidget *widget = dynamic_cast<QTreeWidget *>(object))
            {
                QStringList itemsString = valueForWidget(doc, widget->objectName(), "").split("$");

                QString selectedVariable = "";
                QString selectedFieldid = "";
                if (itemsString.count() == 2)
                {
                    selectedVariable = itemsString.at(0);
                    selectedFieldid = itemsString.at(1);
                }

                if ((!selectedFieldid.isEmpty()) && (!selectedVariable.isEmpty()))
                {
                    for (int field = 0; field < widget->topLevelItemCount(); field++)
                    {
                        QTreeWidgetItem *fieldItem = widget->topLevelItem(field);
                        for (int i = 0; i < fieldItem->childCount(); i++)
                        {
                            QTreeWidgetItem *item = fieldItem->child(i);

                            if ((selectedVariable == item->data(0, Qt::UserRole)) && (selectedFieldid == item->data(1, Qt::UserRole)))
                                widget->setCurrentItem(item);
                        }
                    }
                }
            }
        }
    }
}

void FormScript::saveToFile(const QString &fileName)
{
    QString fn = fileName;

    if (fn.isEmpty())
    {
        QSettings settings;
        QString dir = settings.value("FormScript/LastProblemDir", "").toString();
        fn = QFileDialog::getSaveFileName(this, tr("Save file"), dir, tr("Agros2D form files (*.frm)"));
    }

    if (fn.isEmpty()) return;

    QFileInfo fileInfo(fn);
    if (fileInfo.suffix().toLower() != "frm") fn += ".frm";

    QSettings settings;
    settings.setValue("FormScript/LastProblemDir", QFileInfo(fn).absolutePath());

    XMLForm::config config;

    foreach (QWidget *object, mainWidget->findChildren<QWidget *>())
    {
        if (object->objectName().startsWith("qt_"))
            continue;

        // line edit
        if (QLineEdit *widget = dynamic_cast<QLineEdit  *>(object))
        {
            // replace "," to "."
            QString str = widget->text();
            if (widget->property("dataType") == "double")
                str = str.replace(",", ".");

            config.item().push_back(XMLForm::item(widget->objectName().toStdString(), str.toStdString()));
        }
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
        // list widget
        if (QListWidget *widget = dynamic_cast<QListWidget *>(object))
        {
            // preprocessor - nodes, edges or labels
            if ((widget->property("dataType") == "nodes") || (widget->property("dataType") == "edges") || (widget->property("dataType") == "labels"))
            {
                QString out;
                for (int i = 0; i < widget->count(); i++)
                {
                    if (widget->item(i)->checkState() == Qt::Checked)
                        out += QString("%1,").arg(i);
                }
                if (out.length() > 1)
                    out = out.left(out.count() - 1);

                config.item().push_back(XMLForm::item(widget->objectName().toStdString(), out.toStdString()));
            }
        }
        // tree widget
        if (QTreeWidget *widget = dynamic_cast<QTreeWidget *>(object))
        {
            // postprocessor - local values, surface integrals, volume integrals
            if (widget->property("dataType") == "local_values" || (widget->property("dataType") == "surface_integrals") || (widget->property("dataType") == "volume_integrals"))
            {
                if (widget->currentItem())
                    config.item().push_back(XMLForm::item(widget->objectName().toStdString(),
                                                          (QString("%1$%2")
                                                           .arg(widget->currentItem()->data(0, Qt::UserRole).toString())
                                                           .arg(widget->currentItem()->data(1, Qt::UserRole).toString())).toStdString()));
            }
        }
    }

    XMLForm::form doc(config);

    std::string problem_schema_location("");

    problem_schema_location.append(QString("%1/form_xml.xsd").arg(QFileInfo(datadir() + XSDROOT).absoluteFilePath()).toStdString());
    ::xml_schema::namespace_info namespace_info_problem("XMLForm", problem_schema_location);

    ::xml_schema::namespace_infomap namespace_info_map;
    namespace_info_map.insert(std::pair<std::basic_string<char>, xml_schema::namespace_info>("problem", namespace_info_problem));

    std::ofstream out(compatibleFilename(fn).toStdString().c_str());
    XMLForm::form_(out, doc, namespace_info_map);
}

void FormScript::acceptForm()
{
    QString scriptFileName = QString("%1/%2.py").arg(QFileInfo(m_fileName).absolutePath()).arg(QFileInfo(m_fileName).baseName());

    if (QFile::exists(scriptFileName))
    {
        std::string info;
        ctemplate::TemplateDictionary script("script");

        foreach (QWidget *object, mainWidget->findChildren<QWidget *>())
        {
            // line edit
            if (QLineEdit *widget = dynamic_cast<QLineEdit *>(object))
            {
                // replace "," to "."
                QString str = widget->text();
                if (widget->property("dataType") == "double")
                    str = str.replace(",", ".");

                script.SetValue(QString("%1_text").arg(widget->objectName()).toStdString(), str.toStdString());
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
            // list widget
            if (QListWidget *widget = dynamic_cast<QListWidget *>(object))
            {
                // preprocessor - nodes, edges or labels
                if ((widget->property("dataType") == "nodes") || (widget->property("dataType") == "edges") || (widget->property("dataType") == "labels"))
                {
                    QString out = "[";
                    for (int i = 0; i < widget->count(); i++)
                    {
                        if (widget->item(i)->checkState() == Qt::Checked)
                            out += QString("%1, ").arg(i);
                    }
                    if (out.length() > 1)
                        out = out.left(out.count() - 2);
                    out += "]";

                    script.SetValue(QString("%1_list").arg(widget->objectName()).toStdString(), out.toStdString());
                }
            }
            // tree widget
            if (QTreeWidget *widget = dynamic_cast<QTreeWidget *>(object))
            {
                // postprocessor - local values, surface integrals, volume integrals
                if (widget->property("dataType") == "local_values" || (widget->property("dataType") == "surface_integrals") || (widget->property("dataType") == "volume_integrals"))
                {
                    if (widget->currentItem())
                    {
                        script.SetValue(QString("%1_variable").arg(widget->objectName()).toStdString(), widget->currentItem()->data(0, Qt::UserRole).toString().toStdString());
                        script.SetValue(QString("%1_fieldid").arg(widget->objectName()).toStdString(), widget->currentItem()->data(1, Qt::UserRole).toString().toStdString());
                    }
                }
            }
        }

        ctemplate::ExpandTemplate(compatibleFilename(scriptFileName).toStdString(),
                                  ctemplate::DO_NOT_STRIP, &script, &info);

        // writeStringContent(tempProblemDir() + "/script.py", QString::fromStdString(info));
        consoleView->console()->consoleMessage(tr("Custom form: %1\n").
                                               arg(windowTitle()),
                                               Qt::gray);

        consoleView->console()->connectStdOut();
        bool successfulRun = currentPythonEngineAgros()->runScript(QString::fromStdString(info));
        consoleView->console()->disconnectStdOut();

        if (successfulRun)
        {
            consoleView->console()->appendCommandPrompt();
            accept();
        }
        else
        {
            ScriptResult result = currentPythonEngineAgros()->parseError();

            errorMessage->setVisible(true);
            errorMessage->setText(result.text + "\n" + result.traceback);

            consoleView->console()->stdErr(result.text);
            consoleView->console()->appendCommandPrompt();
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
