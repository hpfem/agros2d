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

#include "moduledialog.h"

#include "gui.h"

#include "scene.h"
#include "sceneview_common.h"
#include "pythonlabagros.h"
#include "hermes2d/module.h"

Q_DECLARE_METATYPE(XMLModule::constant *)
Q_DECLARE_METATYPE(XMLModule::analysis *)
Q_DECLARE_METATYPE(XMLModule::quantity *)
Q_DECLARE_METATYPE(XMLModule::matrix_form *)
Q_DECLARE_METATYPE(XMLModule::vector_form *)
Q_DECLARE_METATYPE(XMLModule::essential_form *)
Q_DECLARE_METATYPE(XMLModule::localvariable *)
Q_DECLARE_METATYPE(XMLModule::default_ *)
Q_DECLARE_METATYPE(XMLModule::surfaceintegral *)
Q_DECLARE_METATYPE(XMLModule::volumeintegral *)

ModuleItem::ModuleItem(QWidget *parent)
    : QWidget(parent)
{
}

ModuleItemLocalValue::ModuleItemLocalValue(const QString &type, XMLModule::expression *expr, QWidget *parent)
    : ModuleItem(parent), m_type(type), m_expr(expr)
{
    QGridLayout *layout = new QGridLayout(this);

    if (m_type == "scalar")
    {
        txtPlanar = new QLineEdit();
        txtAxi = new QLineEdit();

        layout->addWidget(new QLabel(tr("Planar:")), 0, 0);
        layout->addWidget(txtPlanar, 0, 1);
        layout->addWidget(new QLabel(tr("Axisymmetric:")), 1, 0);
        layout->addWidget(txtAxi, 1, 1);

        txtPlanar->setText(QString::fromStdString(expr->planar().get()));
        txtAxi->setText(QString::fromStdString(expr->axi().get()));
    }
    else
    {
        txtPlanarX = new QLineEdit();
        txtPlanarY = new QLineEdit();
        txtAxiR = new QLineEdit();
        txtAxiZ = new QLineEdit();

        layout->addWidget(new QLabel(tr("Planar X:")), 0, 0);
        layout->addWidget(txtPlanarX, 0, 1);
        layout->addWidget(new QLabel(tr("Planar Y:")), 1, 0);
        layout->addWidget(txtPlanarY, 1, 1);
        layout->addWidget(new QLabel(tr("Axisymmetric R:")), 2, 0);
        layout->addWidget(txtAxiR, 2, 1);
        layout->addWidget(new QLabel(tr("Axisymmetric Z:")), 3, 0);
        layout->addWidget(txtAxiZ, 3, 1);

        txtPlanarX->setText(QString::fromStdString(expr->planar_x().get()));
        txtPlanarY->setText(QString::fromStdString(expr->planar_y().get()));
        txtAxiR->setText(QString::fromStdString(expr->axi_r().get()));
        txtAxiZ->setText(QString::fromStdString(expr->axi_z().get()));
    }
}

void ModuleItemLocalValue::save()
{
    if (m_type == "scalar")
    {
        m_expr->planar(txtPlanar->text().toStdString());
        m_expr->axi(txtAxi->text().toStdString());
    }
    else
    {
        m_expr->planar_x(txtPlanarX->text().toStdString());
        m_expr->planar_y(txtPlanarY->text().toStdString());
        m_expr->axi_r(txtAxiR->text().toStdString());
        m_expr->axi_z(txtAxiZ->text().toStdString());
    }
}

ModuleItemEmptyDialog::ModuleItemEmptyDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowIcon(icon(""));
    setWindowTitle(tr("Item editor"));

    // dialog buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    // layout
    layoutMain = new QVBoxLayout();

    setLayout(layoutMain);
}

ModuleItemDialog::ModuleItemDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowIcon(icon(""));
    setWindowTitle(tr("Item editor"));

    txtId = new QLineEdit();
    txtName = new QLineEdit();
    txtShortname = new QLineEdit();
    txtShortnameHtml = new HtmlValueEdit();
    txtShortnameLatex = new QLineEdit();
    txtUnit = new QLineEdit();
    txtUnitHtml = new HtmlValueEdit();
    txtUnitLatex = new QLineEdit();

    QGridLayout *layoutGeneral = new QGridLayout();

    layoutGeneral->addWidget(new QLabel(tr("ID:")), 0, 0);
    layoutGeneral->addWidget(txtId, 0, 1, 1, 3);
    layoutGeneral->addWidget(new QLabel(tr("Name:")), 1, 0);
    layoutGeneral->addWidget(txtName, 1, 1, 1, 3);
    layoutGeneral->addWidget(new QLabel(tr("Shortname:")), 2, 0);
    layoutGeneral->addWidget(txtShortname, 2, 1);
    layoutGeneral->addWidget(new QLabel(tr("Shortname (HTML):")), 3, 0);
    layoutGeneral->addWidget(txtShortnameHtml, 3, 1);
    layoutGeneral->addWidget(new QLabel(tr("Shortname (LaTeX):")), 4, 0);
    layoutGeneral->addWidget(txtShortnameLatex, 4, 1);
    layoutGeneral->addWidget(new QLabel(tr("Unit:")), 2, 2);
    layoutGeneral->addWidget(txtUnit, 2, 3);
    layoutGeneral->addWidget(new QLabel(tr("Unit (HTML):")), 3, 2);
    layoutGeneral->addWidget(txtUnitHtml, 3, 3);
    layoutGeneral->addWidget(new QLabel(tr("Unit (LaTeX):")), 4, 2);
    layoutGeneral->addWidget(txtUnitLatex, 4, 3);

    QGroupBox *grpGeneral = new QGroupBox(tr("General"));
    grpGeneral->setLayout(layoutGeneral);

    // dialog buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    // layout
    layoutMain = new QVBoxLayout();
    layoutMain->addWidget(grpGeneral);

    setLayout(layoutMain);
}

ModuleWeakform::ModuleWeakform(WeakFormKind weakForm, QWidget *parent)
{
    txtI = new QSpinBox();
    txtI->setMinimum(1);
    txtI->setMaximum(2); // TODO: FIX
    txtJ = new QSpinBox();
    txtJ->setMinimum(1);
    txtJ->setMaximum(2); // TODO: FIX

    txtPlanarLinear = new QLineEdit();
    txtAxiLinear = new QLineEdit();
    txtPlanarNewton = new QLineEdit();
    txtAxiNewton = new QLineEdit();

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(new QLabel(tr("I:")), 0, 0);
    layout->addWidget(txtI, 0, 1);
    layout->addWidget(new QLabel(tr("J:")), 0, 2);
    layout->addWidget(txtJ, 0, 3);
    layout->addWidget(new QLabel(tr("Planar - linear:")), 1, 0);
    layout->addWidget(txtPlanarLinear, 1, 1, 1, 2);
    layout->addWidget(new QLabel(tr("Axisymmetric - linear:")), 2, 0);
    layout->addWidget(txtAxiLinear, 2, 1, 1, 2);
    layout->addWidget(new QLabel(tr("Planar - Newton:")), 3, 0);
    layout->addWidget(txtPlanarLinear, 3, 1, 1, 2);
    layout->addWidget(new QLabel(tr("Axisymmetric - Newton:")), 4, 0);
    layout->addWidget(txtAxiLinear, 4, 1, 1, 2);
}

// ***********************************************************************************************************************

ModuleItemConstantDialog::ModuleItemConstantDialog(XMLModule::constant *constant, QWidget *parent)
    : ModuleItemEmptyDialog(parent), m_constant(constant)
{
    txtID = new QLineEdit();
    txtID->setText(QString::fromStdString(constant->id()));

    txtValue = new QLineEdit();
    txtValue->setText(QString::number(constant->value()));

    QGridLayout *layoutGeneral = new QGridLayout(this);
    layoutGeneral->addWidget(new QLabel(tr("ID:")), 0, 0);
    layoutGeneral->addWidget(txtID, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("Value:")), 1, 0);
    layoutGeneral->addWidget(txtValue, 1, 1);

    layoutMain->addLayout(layoutGeneral);
    layoutMain->addStretch();
    layoutMain->addWidget(buttonBox);
}

void ModuleItemConstantDialog::doAccept()
{
    m_constant->id(txtID->text().toStdString());
    m_constant->value(txtValue->text().toDouble());

    accept();
}

// ***********************************************************************************************************************

ModuleItemAnalysisDialog::ModuleItemAnalysisDialog(XMLModule::analysis *analysis, QWidget *parent)
    : ModuleItemEmptyDialog(parent), m_analysis(analysis)
{
    txtID = new QLineEdit();
    txtID->setText(QString::fromStdString(analysis->id()));

    txtName = new QLineEdit();
    txtName->setText(QString::fromStdString(analysis->name()));

    // TODO: (Franta)
    cmbType = new QComboBox();
    cmbType->addItem(analysisTypeToStringKey(AnalysisType_SteadyState));
    cmbType->addItem(analysisTypeToStringKey(AnalysisType_Transient));
    cmbType->addItem(analysisTypeToStringKey(AnalysisType_Harmonic));
    cmbType->setCurrentIndex(cmbType->findText(analysisTypeToStringKey(analysisTypeFromStringKey(
                                                                           QString::fromStdString(analysis->type())))));

    txtSolutions = new QLineEdit();
    txtSolutions->setText(QString::number(analysis->solutions()));

    QGridLayout *layoutGeneral = new QGridLayout(this);
    layoutGeneral->addWidget(new QLabel(tr("ID:")), 0, 0);
    layoutGeneral->addWidget(txtID, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("Name:")), 1, 0);
    layoutGeneral->addWidget(txtName, 1, 1);
    layoutGeneral->addWidget(new QLabel(tr("Type:")), 2, 0);
    layoutGeneral->addWidget(cmbType, 2, 1);
    layoutGeneral->addWidget(new QLabel(tr("Solution:")), 3, 0);
    layoutGeneral->addWidget(txtSolutions, 3, 1);

    layoutMain->addLayout(layoutGeneral);
    layoutMain->addStretch();
    layoutMain->addWidget(buttonBox);
}

void ModuleItemAnalysisDialog::doAccept()
{
    m_analysis->id(txtID->text().toStdString());
    m_analysis->name(txtName->text().toStdString());
    m_analysis->type(cmbType->currentText().toStdString());
    m_analysis->solutions(txtSolutions->text().toInt());

    accept();
}

// ***********************************************************************************************************************

ModuleItemQuantityGlobalDialog::ModuleItemQuantityGlobalDialog(XMLModule::quantity *quantity, QWidget *parent)
    : ModuleItemEmptyDialog(parent), m_quantity(quantity)
{
    txtID = new QLineEdit();
    txtID->setText(QString::fromStdString(quantity->id()));

    txtShortname = new QLineEdit();
    txtShortname->setText(QString::fromStdString(quantity->shortname().get()));

    QGridLayout *layoutGeneral = new QGridLayout(this);
    layoutGeneral->addWidget(new QLabel(tr("ID:")), 0, 0);
    layoutGeneral->addWidget(txtID, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("Shortname:")), 1, 0);
    layoutGeneral->addWidget(txtShortname, 1, 1);

    layoutMain->addLayout(layoutGeneral);
    layoutMain->addStretch();
    layoutMain->addWidget(buttonBox);
}

void ModuleItemQuantityGlobalDialog::doAccept()
{
    m_quantity->id(txtID->text().toStdString());
    m_quantity->shortname(txtShortname->text().toStdString());

    accept();
}

ModuleItemQuantityAnalysisDialog::ModuleItemQuantityAnalysisDialog(XMLModule::quantity *quantity, QWidget *parent)
    : ModuleItemEmptyDialog(parent), m_quantity(quantity)
{
    txtPlanarNonlinearity = new QLineEdit();
    if (quantity->nonlinearity_planar().present())
        txtPlanarNonlinearity->setText(QString::fromStdString(quantity->nonlinearity_planar().get()));

    txtAxiNonlinearity = new QLineEdit();
    if (quantity->nonlinearity_axi().present())
        txtAxiNonlinearity->setText(QString::fromStdString(quantity->nonlinearity_axi().get()));

    txtDependence = new QLineEdit();
    if (quantity->dependence().present())
        txtDependence->setText(QString::fromStdString(quantity->dependence().get()));

    QGridLayout *layoutGeneral = new QGridLayout(this);
    layoutGeneral->addWidget(new QLabel(tr("Planar nelinearity:")), 0, 0);
    layoutGeneral->addWidget(txtPlanarNonlinearity, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("Axisymmetric nelinearity:")), 1, 0);
    layoutGeneral->addWidget(txtAxiNonlinearity, 1, 1);
    layoutGeneral->addWidget(new QLabel(tr("Dependence:")), 2, 0);
    layoutGeneral->addWidget(txtDependence, 2, 1);

    layoutMain->addLayout(layoutGeneral);
    layoutMain->addStretch();
    layoutMain->addWidget(buttonBox);
}

void ModuleItemQuantityAnalysisDialog::doAccept()
{
    m_quantity->nonlinearity_planar(txtPlanarNonlinearity->text().toStdString());
    m_quantity->nonlinearity_axi(txtAxiNonlinearity->text().toStdString());
    m_quantity->dependence(txtDependence->text().toStdString());

    accept();
}

// ***********************************************************************************************************************

ModuleItemWeakformDialog::ModuleItemWeakformDialog(QWidget *parent)
    : ModuleItemEmptyDialog(parent)
{
    txtI = new QLineEdit();
    txtJ = new QLineEdit();

    txtPlanarLinear = new QTextEdit();
    txtPlanarLinear->setAcceptRichText(false);

    txtPlanarNewton = new QTextEdit();
    txtPlanarLinear->setAcceptRichText(false);

    txtAxiLinear = new QTextEdit();
    txtPlanarLinear->setAcceptRichText(false);

    txtAxiNewton = new QTextEdit();
    txtPlanarLinear->setAcceptRichText(false);

    QGridLayout *layoutGeneral = new QGridLayout(this);
    layoutGeneral->addWidget(new QLabel(tr("I:")), 0, 0);
    layoutGeneral->addWidget(txtI, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("J:")), 0, 2);
    layoutGeneral->addWidget(txtJ, 0, 3);

    QGridLayout *layoutLinear = new QGridLayout(this);
    layoutLinear->addWidget(new QLabel(tr("Planar:")), 0, 0);
    layoutLinear->addWidget(txtPlanarLinear, 1, 0);
    layoutLinear->addWidget(new QLabel(tr("Axisymetric:")), 2, 0);
    layoutLinear->addWidget(txtAxiLinear, 3, 0);

    QGroupBox *grpLinear = new QGroupBox(tr("Linear"));
    grpLinear->setLayout(layoutLinear);

    QGridLayout *layoutNewton = new QGridLayout(this);
    layoutNewton->addWidget(new QLabel(tr("Planar:")), 0, 0);
    layoutNewton->addWidget(txtPlanarNewton, 1, 0);
    layoutNewton->addWidget(new QLabel(tr("Axisymmetric:")), 2, 0);
    layoutNewton->addWidget(txtAxiNewton, 3, 0);

    QGroupBox *grpNewton = new QGroupBox(tr("Nonlinear"));
    grpNewton->setLayout(layoutNewton);

    layoutMain->addLayout(layoutGeneral);
    layoutMain->addWidget(grpLinear);
    layoutMain->addWidget(grpNewton);
    layoutMain->addStretch();
    layoutMain->addWidget(buttonBox);
}

ModuleItemMatrixFormDialog::ModuleItemMatrixFormDialog(XMLModule::matrix_form *form, QWidget *parent)
    : ModuleItemWeakformDialog(parent), m_form(form)
{
    txtI->setText(QString::number(m_form->i()));
    txtJ->setText(QString::number(m_form->j()));
    txtPlanarLinear->setText(QString::fromStdString(m_form->planar_linear()));
    txtAxiLinear->setText(QString::fromStdString(m_form->axi_linear()));
    txtPlanarNewton->setText(QString::fromStdString(m_form->planar_newton()));
    txtAxiNewton->setText(QString::fromStdString(m_form->axi_newton()));
}

void ModuleItemMatrixFormDialog::doAccept()
{
    m_form->i(txtI->text().toInt());
    m_form->j(txtJ->text().toInt());
    m_form->planar_linear(txtPlanarLinear->toPlainText().toStdString());
    m_form->axi_linear(txtAxiLinear->toPlainText().toStdString());
    m_form->planar_newton(txtPlanarNewton->toPlainText().toStdString());
    m_form->axi_newton(txtAxiNewton->toPlainText().toStdString());

    accept();
}

ModuleItemVectorFormDialog::ModuleItemVectorFormDialog(XMLModule::vector_form *form, QWidget *parent)
    : ModuleItemWeakformDialog(parent), m_form(form)
{
    txtI->setText(QString::number(m_form->i()));
    txtJ->setText(QString::number(m_form->j()));
    txtPlanarLinear->setText(QString::fromStdString(m_form->planar_linear()));
    txtAxiLinear->setText(QString::fromStdString(m_form->axi_linear()));
    txtPlanarNewton->setText(QString::fromStdString(m_form->planar_newton()));
    txtAxiNewton->setText(QString::fromStdString(m_form->axi_newton()));
}

void ModuleItemVectorFormDialog::doAccept()
{
    m_form->i(txtI->text().toInt());
    m_form->j(txtJ->text().toInt());
    m_form->planar_linear(txtPlanarLinear->toPlainText().toStdString());
    m_form->axi_linear(txtAxiLinear->toPlainText().toStdString());
    m_form->planar_newton(txtPlanarNewton->toPlainText().toStdString());
    m_form->axi_newton(txtAxiNewton->toPlainText().toStdString());

    accept();
}

ModuleItemEssentialFormDialog::ModuleItemEssentialFormDialog(XMLModule::essential_form *form, QWidget *parent)
    : ModuleItemWeakformDialog(parent), m_form(form)
{
    txtI->setText(QString::number(m_form->i()));
    txtJ->setText("");
    txtJ->setDisabled(true);
    txtPlanarLinear->setText(QString::fromStdString(m_form->planar_linear()));
    txtAxiLinear->setText(QString::fromStdString(m_form->axi_linear()));
    txtPlanarNewton->setText(QString::fromStdString(m_form->planar_newton()));
    txtAxiNewton->setText(QString::fromStdString(m_form->axi_newton()));
}

void ModuleItemEssentialFormDialog::doAccept()
{
    m_form->i(txtI->text().toInt());
    m_form->planar_linear(txtPlanarLinear->toPlainText().toStdString());
    m_form->axi_linear(txtAxiLinear->toPlainText().toStdString());
    m_form->planar_newton(txtPlanarNewton->toPlainText().toStdString());
    m_form->axi_newton(txtAxiNewton->toPlainText().toStdString());

    accept();
}

// ***********************************************************************************************************************

ModulePreprocessorDialog::ModulePreprocessorDialog(XMLModule::quantity *quant, QWidget *parent)
    : ModuleItemDialog(parent), m_quant(quant)
{
    txtId->setText(QString::fromStdString(quant->id()));
    if (quant->name().present())
        txtName->setText(QString::fromStdString(quant->name().get()));
    if (quant->shortname().present())
        txtShortname->setText(QString::fromStdString(quant->shortname().get()));
    if (quant->shortname_html().present())
        txtShortnameHtml->setText(QString::fromStdString(quant->shortname_html().get()));
    if (quant->shortname_latex().present())
        txtShortnameLatex->setText(QString::fromStdString(quant->shortname_latex().get()));
    if (quant->unit().present())
        txtUnit->setText(QString::fromStdString(quant->unit().get()));
    if (quant->unit_html().present())
        txtUnitHtml->setText(QString::fromStdString(quant->unit_html().get()));
    if (quant->unit_latex().present())
        txtUnitLatex->setText(QString::fromStdString(quant->unit_latex().get()));

    layoutMain->addStretch();
    layoutMain->addWidget(buttonBox);
}

void ModulePreprocessorDialog::doAccept()
{
    m_quant->id(txtId->text().toStdString());
    m_quant->name(txtName->text().toStdString());
    m_quant->shortname(txtShortname->text().toStdString());
    m_quant->shortname_html(txtShortnameHtml->text().toStdString());
    m_quant->shortname_latex(txtShortnameLatex->text().toStdString());
    m_quant->unit(txtUnit->text().toStdString());
    m_quant->unit_html(txtUnitHtml->text().toStdString());
    m_quant->unit_latex(txtUnitLatex->text().toStdString());

    accept();
}

// ***********************************************************************************************************************

ModuleItemLocalValueDialog::ModuleItemLocalValueDialog(XMLModule::localvariable *lv, QWidget *parent)
    : ModuleItemDialog(parent), m_lv(lv)
{
    txtId->setText(QString::fromStdString(lv->id()));
    txtName->setText(QString::fromStdString(lv->name()));
    txtShortname->setText(QString::fromStdString(lv->shortname()));
    if (lv->shortname_html().present())
        txtShortnameHtml->setText(QString::fromStdString(lv->shortname_html().get()));
    if (lv->shortname_latex().present())
        txtShortnameLatex->setText(QString::fromStdString(lv->shortname_latex().get()));
    txtUnit->setText(QString::fromStdString(lv->unit()));
    if (lv->unit_html().present())
        txtUnitHtml->setText(QString::fromStdString(lv->unit_html().get()));
    if (lv->unit_latex().present())
        txtUnitLatex->setText(QString::fromStdString(lv->unit_latex().get()));

    for (int i = 0; i < m_lv->expression().size(); i++)
    {
        XMLModule::expression *expr = &lv->expression().at(i);

        ModuleItemLocalValue *item = new ModuleItemLocalValue(QString::fromStdString(lv->type()), expr, this);
        items.append(item);

        QHBoxLayout *layoutGeneral = new QHBoxLayout(this);
        layoutGeneral->addWidget(item);

        QGroupBox *grpGeneral = new QGroupBox(analysisTypeString(analysisTypeFromStringKey(QString::fromStdString(expr->analysistype()))));
        grpGeneral->setLayout(layoutGeneral);

        layoutMain->addWidget(grpGeneral);
    }

    layoutMain->addStretch();
    layoutMain->addWidget(buttonBox);
}

void ModuleItemLocalValueDialog::doAccept()
{
    m_lv->id(txtId->text().toStdString());
    m_lv->name(txtName->text().toStdString());
    m_lv->shortname(txtShortname->text().toStdString());
    m_lv->shortname_html(txtShortnameHtml->text().toStdString());
    m_lv->shortname_latex(txtShortnameLatex->text().toStdString());
    m_lv->unit(txtUnit->text().toStdString());
    m_lv->unit_html(txtUnitHtml->text().toStdString());
    m_lv->unit_latex(txtUnitLatex->text().toStdString());

    foreach (ModuleItem *item, items)
    {
        item->save();
        delete item;
    }
    items.clear();

    accept();
}

// ***********************************************************************************************************************

ModuleItemViewDefaultsDialog::ModuleItemViewDefaultsDialog(XMLModule::default_ *def, XMLModule::localvariables *lv, QWidget *parent)
    : ModuleItemEmptyDialog(parent), m_def(def), m_lv(lv)
{
    cmbLocalVariable = new QComboBox();
    for (int i = 0; i < lv->localvariable().size(); i++)
    {
        XMLModule::localvariable *variable = &lv->localvariable().at(i);
        cmbLocalVariable->addItem(QString::fromStdString(variable->id()));
    }
    cmbLocalVariable->setCurrentIndex(cmbLocalVariable->findText(QString::fromStdString(m_def->id())));

    QGridLayout *layoutGeneral = new QGridLayout(this);
    layoutGeneral->addWidget(new QLabel(tr("Local variable:")), 0, 0);
    layoutGeneral->addWidget(cmbLocalVariable, 0, 1);

    layoutMain->addLayout(layoutGeneral);
    layoutMain->addStretch();
    layoutMain->addWidget(buttonBox);
}

void ModuleItemViewDefaultsDialog::doAccept()
{
    m_def->id(cmbLocalVariable->currentText().toStdString());

    accept();
}

// ***********************************************************************************************************************

ModuleVolumeIntegralValueDialog::ModuleVolumeIntegralValueDialog(XMLModule::volumeintegral *vol, QWidget *parent)
    : ModuleItemDialog(parent), m_vol(vol)
{
    txtId->setText(QString::fromStdString(vol->id()));
    txtName->setText(QString::fromStdString(vol->name()));
    txtShortname->setText(QString::fromStdString(vol->shortname()));
    if (vol->shortname_html().present())
        txtShortnameHtml->setText(QString::fromStdString(vol->shortname_html().get()));
    if (vol->shortname_latex().present())
        txtShortnameLatex->setText(QString::fromStdString(vol->shortname_latex().get()));
    txtUnit->setText(QString::fromStdString(vol->unit()));
    if (vol->unit_html().present())
        txtUnitHtml->setText(QString::fromStdString(vol->unit_html().get()));
    if (vol->unit_latex().present())
        txtUnitLatex->setText(QString::fromStdString(vol->unit_latex().get()));

    for (int i = 0; i < vol->expression().size(); i++)
    {
        XMLModule::expression *expr = &vol->expression().at(i);

        ModuleItemLocalValue *item = new ModuleItemLocalValue("scalar", expr, this);
        items.append(item);

        QHBoxLayout *layoutGeneral = new QHBoxLayout(this);
        layoutGeneral->addWidget(item);

        QGroupBox *grpGeneral = new QGroupBox(analysisTypeString(analysisTypeFromStringKey(QString::fromStdString(expr->analysistype()))));
        grpGeneral->setLayout(layoutGeneral);

        layoutMain->addWidget(grpGeneral);
    }

    layoutMain->addStretch();
    layoutMain->addWidget(buttonBox);
}

void ModuleVolumeIntegralValueDialog::doAccept()
{
    m_vol->id(txtId->text().toStdString());
    m_vol->name(txtName->text().toStdString());
    m_vol->shortname(txtShortname->text().toStdString());
    m_vol->shortname_html(txtShortnameHtml->text().toStdString());
    m_vol->shortname_latex(txtShortnameLatex->text().toStdString());
    m_vol->unit(txtUnit->text().toStdString());
    m_vol->unit_html(txtUnitHtml->text().toStdString());
    m_vol->unit_latex(txtUnitLatex->text().toStdString());

    accept();
}

// ***********************************************************************************************************************

ModuleSurfaceIntegralValueDialog::ModuleSurfaceIntegralValueDialog(XMLModule::surfaceintegral *sur, QWidget *parent)
    : ModuleItemDialog(parent), m_sur(sur)
{
    txtId->setText(QString::fromStdString(sur->id()));
    txtName->setText(QString::fromStdString(sur->name()));
    txtShortname->setText(QString::fromStdString(sur->shortname()));
    if (sur->shortname_html().present())
        txtShortnameHtml->setText(QString::fromStdString(sur->shortname_html().get()));
    if (sur->shortname_latex().present())
        txtShortnameLatex->setText(QString::fromStdString(sur->shortname_latex().get()));
    txtUnit->setText(QString::fromStdString(sur->unit()));
    if (sur->unit_html().present())
        txtUnitHtml->setText(QString::fromStdString(sur->unit_html().get()));
    if (sur->unit_latex().present())
        txtUnitLatex->setText(QString::fromStdString(sur->unit_latex().get()));

    for (int i = 0; i < sur->expression().size(); i++)
    {

        XMLModule::expression *expr = &sur->expression().at(i);

        ModuleItemLocalValue *item = new ModuleItemLocalValue("scalar", expr, this);
        items.append(item);

        QHBoxLayout *layoutGeneral = new QHBoxLayout(this);
        layoutGeneral->addWidget(item);

        QGroupBox *grpGeneral = new QGroupBox(analysisTypeString(analysisTypeFromStringKey(QString::fromStdString(expr->analysistype()))));
        grpGeneral->setLayout(layoutGeneral);

        layoutMain->addWidget(grpGeneral);
    }

    layoutMain->addStretch();
    layoutMain->addWidget(buttonBox);
}

void ModuleSurfaceIntegralValueDialog::doAccept()
{
    m_sur->id(txtId->text().toStdString());
    m_sur->name(txtName->text().toStdString());
    m_sur->shortname(txtShortname->text().toStdString());
    m_sur->shortname_html(txtShortnameHtml->text().toStdString());
    m_sur->shortname_latex(txtShortnameLatex->text().toStdString());
    m_sur->unit(txtUnit->text().toStdString());
    m_sur->unit_html(txtUnitHtml->text().toStdString());
    m_sur->unit_latex(txtUnitLatex->text().toStdString());

    accept();
}

// ***********************************************************************************************************************

ModuleDialog::ModuleDialog(const QString &fieldId, QWidget *parent)
    : QDialog(parent), m_fieldId(fieldId)
{
    setWindowIcon(icon(""));
    setWindowTitle(tr("Module editor"));

    createControls();

    m_module_xsd = XMLModule::module_((datadir().toStdString() + MODULEROOT.toStdString() + "/" + m_fieldId.toStdString() + ".xml").c_str());

    load();

    setMinimumSize(QSize(950, 600));
}

void ModuleDialog::load()
{
    XMLModule::module *module = m_module_xsd.get();

    // main
    txtId->setText(QString::fromStdString(module->general().id()));
    txtName->setText(QString::fromStdString(module->general().name()));
    txtDescription->setPlainText(QString::fromStdString(module->general().description()));

    // constants
    treeConstants->clear();
    for (int i = 0; i < module->constants().constant().size(); i++)
    {
        XMLModule::constant *cnst = &module->constants().constant().at(i);

        QTreeWidgetItem *item = new QTreeWidgetItem(treeConstants);

        item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::constant *>(cnst));
        item->setText(0, QString::fromStdString(cnst->id()));
        item->setText(1, QString::number(cnst->value()));
    }

    // analyses
    treeAnalyses->clear();
    for (int i = 0; i < module->general().analyses().analysis().size(); i++)
    {
        XMLModule::analysis *analysis = &module->general().analyses().analysis().at(i);

        QTreeWidgetItem *item = new QTreeWidgetItem(treeAnalyses);

        item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::analysis *>(analysis));
        item->setText(0, QString::fromStdString(analysis->name()));
        item->setText(1, QString::fromStdString(analysis->type()));
        item->setText(2, QString::number(analysis->solutions()));
    }

    // volume weakform quantities
    treeVolumeQuantityGlobal->clear();
    for (int i = 0; i < module->volume().quantity().size(); i++)
    {
        XMLModule::quantity *quantity = &module->volume().quantity().at(i);

        QTreeWidgetItem *item = new QTreeWidgetItem(treeVolumeQuantityGlobal);

        item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::quantity *>(quantity));
        item->setText(0, QString::fromStdString(quantity->id()));
        item->setText(1, QString::fromStdString((quantity->shortname().present()) ? quantity->shortname().get() : ""));
    }

    treeVolumeQuantityAnalysis->clear();
    for (int i = 0; i < module->volume().weakforms_volume().weakform_volume().size(); i++)
    {
        XMLModule::weakform_volume *wf = &module->volume().weakforms_volume().weakform_volume().at(i);

        QTreeWidgetItem *analysis = new QTreeWidgetItem(treeVolumeQuantityAnalysis);
        analysis->setExpanded(true);
        analysis->setText(0, analysisTypeString(analysisTypeFromStringKey(QString::fromStdString(wf->analysistype()))));

        for (int i = 0; i < wf->quantity().size(); i++)
        {
            XMLModule::quantity *quantity = &wf->quantity().at(i);

            QTreeWidgetItem *item = new QTreeWidgetItem(analysis);

            item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::quantity *>(quantity));
            item->setText(0, QString::fromStdString(quantity->id()));

            item->setText(1, tr("linear"));
            if (quantity->nonlinearity_planar().present() || quantity->nonlinearity_axi().present())
            {
                if (!QString::fromStdString(quantity->nonlinearity_planar().get()).isEmpty() ||
                        !QString::fromStdString(quantity->nonlinearity_axi().get()).isEmpty())
                    item->setText(1, tr("nonlinear"));
            }

            item->setText(2, (quantity->dependence().present() && !QString::fromStdString(quantity->dependence().get()).isEmpty()) ?
                              QString::fromStdString(quantity->dependence().get()) : "");
        }
    }

    // volume weakforms
    for (int i = 0; i < module->volume().weakforms_volume().weakform_volume().size(); i++)
    {
        XMLModule::weakform_volume *wf = &module->volume().weakforms_volume().weakform_volume().at(i);

        QTreeWidgetItem *analysis = new QTreeWidgetItem(treeVolumeWeakforms);
        analysis->setExpanded(true);
        analysis->setText(0, analysisTypeString(analysisTypeFromStringKey(QString::fromStdString(wf->analysistype()))));

        // matrix form
        for (int i = 0; i < wf->matrix_form().size(); i++)
        {
            XMLModule::matrix_form *form = &wf->matrix_form().at(i);

            QTreeWidgetItem *item = new QTreeWidgetItem(analysis);

            item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::matrix_form *>(form));
            item->setText(0, tr("Matrix form"));
            item->setText(1, QString::number(form->i()));
            item->setText(2, QString::number(form->j()));
        }

        //vector form
        for (int i = 0; i < wf->vector_form().size(); i++)
        {
            XMLModule::vector_form *form = &wf->vector_form().at(i);

            QTreeWidgetItem *item = new QTreeWidgetItem(analysis);

            item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::vector_form *>(form));
            item->setText(0, tr("Vector form"));
            item->setText(1, QString::number(form->i()));
            item->setText(2, QString::number(form->j()));
        }
    }

    // surface weakform quantities
    treeSurfaceQuantityGlobal->clear();
    for (int i = 0; i < module->surface().quantity().size(); i++)
    {
        XMLModule::quantity *quantity = &module->surface().quantity().at(i);

        QTreeWidgetItem *item = new QTreeWidgetItem(treeSurfaceQuantityGlobal);

        item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::quantity *>(quantity));
        item->setText(0, QString::fromStdString(quantity->id()));
        item->setText(1, QString::fromStdString((quantity->shortname().present()) ? quantity->shortname().get() : ""));
    }

    treeSurfaceQuantityAnalysis->clear();
    for (int i = 0; i < module->surface().weakforms_surface().weakform_surface().size(); i++)
    {
        XMLModule::weakform_surface *wf = &module->surface().weakforms_surface().weakform_surface().at(i);

        QTreeWidgetItem *analysis = new QTreeWidgetItem(treeSurfaceQuantityAnalysis);
        analysis->setExpanded(true);
        analysis->setText(0, analysisTypeString(analysisTypeFromStringKey(QString::fromStdString(wf->analysistype()))));
        for (int i = 0; i < wf->boundary().size(); i++)
        {
            XMLModule::boundary *bnd = &wf->boundary().at(i);

            QTreeWidgetItem *boundary = new QTreeWidgetItem(analysis);
            boundary->setExpanded(true);
            boundary->setText(0, QString::fromStdString(bnd->name()));

            for (int i = 0; i < bnd->quantity().size(); i++)
            {
                XMLModule::quantity *quantity = &bnd->quantity().at(i);

                QTreeWidgetItem *item = new QTreeWidgetItem(boundary);

                item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::quantity *>(quantity));
                item->setText(0, QString::fromStdString(quantity->id()));

                item->setText(1, tr("linear"));
                if (quantity->nonlinearity_planar().present() || quantity->nonlinearity_axi().present())
                {
                    if (!QString::fromStdString(quantity->nonlinearity_planar().get()).isEmpty() ||
                            !QString::fromStdString(quantity->nonlinearity_axi().get()).isEmpty())
                        item->setText(1, tr("nonlinear"));
                }

                item->setText(2, (quantity->dependence().present() && !QString::fromStdString(quantity->dependence().get()).isEmpty()) ?
                                  QString::fromStdString(quantity->dependence().get()) : "");
            }
        }
    }

    // surface weakforms
    for (int i = 0; i < module->surface().weakforms_surface().weakform_surface().size(); i++)
    {
        XMLModule::weakform_surface *wf = &module->surface().weakforms_surface().weakform_surface().at(i);

        QTreeWidgetItem *analysis = new QTreeWidgetItem(treeSurfaceWeakforms);
        analysis->setExpanded(true);
        analysis->setText(0, analysisTypeString(analysisTypeFromStringKey(QString::fromStdString(wf->analysistype()))));

        for (int i = 0; i < wf->boundary().size(); i++)
        {
            XMLModule::boundary *bnd = &wf->boundary().at(i);

            QTreeWidgetItem *boundary = new QTreeWidgetItem(analysis);
            boundary->setExpanded(true);
            boundary->setText(0, QString::fromStdString(bnd->name()));

            //matrix form
            for (int i = 0; i < bnd->matrix_form().size(); i++)
            {
                XMLModule::matrix_form *form = &bnd->matrix_form().at(i);

                QTreeWidgetItem *item = new QTreeWidgetItem(boundary);

                item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::matrix_form *>(form));
                item->setText(0, tr("Matrix form"));
                item->setText(1, QString::number(form->i()));
                item->setText(2, QString::number(form->j()));
            }

            // vector form
            for (int i = 0; i < bnd->vector_form().size(); i++)
            {
                XMLModule::vector_form *form = &bnd->vector_form().at(i);

                QTreeWidgetItem *item = new QTreeWidgetItem(boundary);

                item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::vector_form *>(form));
                item->setText(0, tr("Vector form"));
                item->setText(1, QString::number(form->i()));
                item->setText(2, QString::number(form->j()));
            }

            // essential form
            for (int i = 0; i < bnd->essential_form().size(); i++)
            {
                XMLModule::essential_form *form = &bnd->essential_form().at(i);

                QTreeWidgetItem *item = new QTreeWidgetItem(boundary);

                item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::essential_form *>(form));
                item->setText(0, tr("Essential form"));
                item->setText(1, QString::number(form->i()));
                item->setText(2, "");
            }
        }
    }

    // materials and boundaries
    treeMaterials->clear();
    treeBoundaries->clear();
    for (int i = 0; i < module->preprocessor().gui().size(); i++)
    {
        XMLModule::gui *ui = &module->preprocessor().gui().at(i);

        for (int i = 0; i < ui->group().size(); i++)
        {
            XMLModule::group *grp = &ui->group().at(i);

            QTreeWidgetItem *group = NULL;
            if (ui->type() == "volume")
                group = new QTreeWidgetItem(treeMaterials);
            else
                group = new QTreeWidgetItem(treeBoundaries);

            group->setExpanded(true);

            // group name
            group->setText(0, (grp->name().present()) ? QString::fromStdString(grp->name().get()) : "unnamed");

            for (int i = 0; i < grp->quantity().size(); i++)
            {
                XMLModule::quantity *quant = &grp->quantity().at(i);

                QTreeWidgetItem *item = new QTreeWidgetItem(group);

                item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::quantity *>(quant));
                item->setText(0, QString::fromStdString((quant->name().present()) ? quant->name().get() : ""));
                item->setText(1, QString::fromStdString((quant->shortname().present()) ? quant->shortname().get() : ""));
                item->setText(2, QString::fromStdString((quant->unit().present()) ? quant->unit().get() : ""));
            }
        }
    }

    // local values
    treeLocalVariables->clear();
    for (int i = 0; i < module->postprocessor().localvariables().localvariable().size(); i++)
    {
        XMLModule::localvariable *lv = &module->postprocessor().localvariables().localvariable().at(i);

        QTreeWidgetItem *item = new QTreeWidgetItem(treeLocalVariables);

        item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::localvariable *>(lv));
        item->setText(0, QString::fromStdString(lv->name()));
        item->setText(1, QString::fromStdString(lv->shortname()));
        item->setText(2, QString::fromStdString(lv->unit()));
        item->setText(3, QString::fromStdString(lv->type()));

        QString analyses;
        for (int j = 0; j < lv->expression().size(); j++)
        {
            XMLModule::expression expr = lv->expression().at(j);
            analyses += QString("%1, ").arg(QString::fromStdString(expr.analysistype()));
        }
        if (analyses.length() > 0)
            analyses = analyses.left(analyses.length() - 2);
        item->setText(4, analyses);
    }

    // view defaults
    treeViewDefaults->clear();
    QTreeWidgetItem *scalarView = new QTreeWidgetItem(treeViewDefaults);
    scalarView->setExpanded(true);
    scalarView->setText(0, tr("Scalar view"));

    XMLModule::scalar_view *sv = &module->postprocessor().view().scalar_view();

    for (int i = 0; i < sv->default_().size(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(scalarView);

        XMLModule::default_ *def = &sv->default_().at(i);
        item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::default_ *>(def));
        item->setText(0, analysisTypeString(analysisTypeFromStringKey(QString::fromStdString(def->analysistype()))));
        item->setText(1, QString::fromStdString(def->id()));
    }

    QTreeWidgetItem *vectorView = new QTreeWidgetItem(treeViewDefaults);
    vectorView->setExpanded(true);
    vectorView->setText(0, tr("Vector view"));

    XMLModule::vector_view *vv = &module->postprocessor().view().vector_view();

    for (int i = 0; i < vv->default_().size(); i++)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(vectorView);

        XMLModule::default_ *def = &vv->default_().at(i);
        item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::default_ *>(def));
        item->setText(0, analysisTypeString(analysisTypeFromStringKey(QString::fromStdString(def->analysistype()))));
        item->setText(1, QString::fromStdString(def->id()));
    }

    // surface integrals
    treeSurfaceIntegrals->clear();
    for (int i = 0; i < module->postprocessor().surfaceintegrals().surfaceintegral().size(); i++)
    {
        XMLModule::surfaceintegral *sur = &module->postprocessor().surfaceintegrals().surfaceintegral().at(i);

        QTreeWidgetItem *item = new QTreeWidgetItem(treeSurfaceIntegrals);

        item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::surfaceintegral *>(sur));
        item->setText(0, QString::fromStdString(sur->name()));
        item->setText(1, QString::fromStdString(sur->shortname()));
        item->setText(2, QString::fromStdString(sur->unit()));

        QString analyses;
        for (int j = 0; j < sur->expression().size(); j++)
        {
            XMLModule::expression expr = sur->expression().at(j);
            analyses += QString("%1, ").arg(QString::fromStdString(expr.analysistype()));
        }
        if (analyses.length() > 0)
            analyses = analyses.left(analyses.length() - 2);
        item->setText(3, analyses);
    }

    // volume integrals
    treeVolumeIntegrals->clear();
    for (int i = 0; i < module->postprocessor().volumeintegrals().volumeintegral().size(); i++)
    {
        XMLModule::volumeintegral *vol = &module->postprocessor().volumeintegrals().volumeintegral().at(i);

        QTreeWidgetItem *item = new QTreeWidgetItem(treeVolumeIntegrals);

        item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::volumeintegral *>(vol));
        item->setText(0, QString::fromStdString(vol->name()));
        item->setText(1, QString::fromStdString(vol->shortname()));
        item->setText(2, QString::fromStdString(vol->unit()));

        QString analyses;
        for (int j = 0; j < vol->expression().size(); j++)
        {
            XMLModule::expression expr = vol->expression().at(j);
            analyses += QString("%1, ").arg(QString::fromStdString(expr.analysistype()));
        }
        if (analyses.length() > 0)
            analyses = analyses.left(analyses.length() - 2);
        item->setText(3, analyses);
    }
}

void ModuleDialog::save()
{
    XMLModule::module *module = m_module_xsd.get();

    // main
    module->general().name(txtName->text().toStdString());
    module->general().description(txtDescription->toPlainText().toStdString());

    try
    {
        std::string mesh_schema_location("");
        mesh_schema_location.append("../xsd/module_xml.xsd");
        ::xml_schema::namespace_info namespace_info_mesh("XMLModule", mesh_schema_location);

        ::xml_schema::namespace_infomap namespace_info_map;
        namespace_info_map.insert(std::pair<std::basic_string<char>, xml_schema::namespace_info>("module", namespace_info_mesh));

        XMLModule::module *module = m_module_xsd.get();
        std::ofstream out((datadir().toStdString() + MODULEROOT.toStdString() + "/" + m_fieldId.toStdString() + ".xml").c_str());
        XMLModule::module_(out, *module, namespace_info_map);
    }
    catch (const xml_schema::exception& e)
    {
        std::cerr << e << std::endl;
    }
}

void ModuleDialog::createControls()
{
    lstView = new QListWidget(this);
    pages = new QStackedWidget(this);

    panMain = createMainWidget();
    panWeakforms = createWeakforms();
    panPreprocessor = createPreprocessor();
    panPostprocessor = createPostprocessor();

    // List View
    lstView->setCurrentRow(0);
    lstView->setViewMode(QListView::IconMode);
    lstView->setResizeMode(QListView::Adjust);
    lstView->setMovement(QListView::Static);
    lstView->setFlow(QListView::TopToBottom);
    lstView->setIconSize(QSize(60, 60));
    lstView->setMinimumWidth(135);
    lstView->setMaximumWidth(135);
    lstView->setMinimumHeight((45+fontMetrics().height()*4)*5);
    connect(lstView, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(doCurrentItemChanged(QListWidgetItem *, QListWidgetItem *)));

    QSize sizeItem(131, 85);

    // listView items
    QListWidgetItem *itemMain = new QListWidgetItem(icon("document-properties"), tr("Main"), lstView);
    itemMain->setTextAlignment(Qt::AlignHCenter);
    itemMain->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    itemMain->setSizeHint(sizeItem);

    QListWidgetItem *itemWeakforms = new QListWidgetItem(icon("options-weakform"), tr("Weakforms"), lstView);
    itemWeakforms->setTextAlignment(Qt::AlignHCenter);
    itemWeakforms->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    itemWeakforms->setSizeHint(sizeItem);

    QListWidgetItem *itemPreprocessor = new QListWidgetItem(icon("scene-geometry"), tr("Preprocessor"), lstView);
    itemPreprocessor->setTextAlignment(Qt::AlignHCenter);
    itemPreprocessor->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    itemPreprocessor->setSizeHint(sizeItem);

    QListWidgetItem *itemPostprocessor = new QListWidgetItem(icon("scene-post2d"), tr("Postprocessor"), lstView);
    itemPostprocessor->setTextAlignment(Qt::AlignHCenter);
    itemPostprocessor->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    itemPostprocessor->setSizeHint(sizeItem);

    pages->addWidget(panMain);
    pages->addWidget(panWeakforms);
    pages->addWidget(panPreprocessor);
    pages->addWidget(panPostprocessor);

    QHBoxLayout *layoutHorizontal = new QHBoxLayout();
    layoutHorizontal->addWidget(lstView);
    layoutHorizontal->addWidget(pages);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutHorizontal);
    // layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

QWidget *ModuleDialog::createMainWidget()
{
    QWidget *mainWidget = new QWidget(this);

    // general
    txtId = new QLineEdit();
    txtName = new QLineEdit();
    txtDescription = new QPlainTextEdit();

    QGridLayout *layoutGeneral = new QGridLayout();
    layoutGeneral->addWidget(new QLabel(tr("Name:")), 0, 0);
    layoutGeneral->addWidget(txtName, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("ID:")), 0, 2);
    layoutGeneral->addWidget(txtId, 0, 3);
    layoutGeneral->addWidget(new QLabel(tr("Description:")), 1, 0);

    // constants
    treeConstants = new QTreeWidget(this);
    treeConstants->setMouseTracking(true);
    treeConstants->setColumnCount(2);
    treeConstants->setColumnWidth(0, 150);
    treeConstants->setIndentation(5);
    QStringList headConstants;
    headConstants << tr("ID") << tr("Value");
    treeConstants->setHeaderLabels(headConstants);

    connect(treeConstants, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(constantDoubleClicked(QTreeWidgetItem *, int)));

    /*
    QPushButton *btnAddConstant = new QPushButton(tr("Add constant"));
    connect(btnAddConstant, SIGNAL(clicked()), this, SLOT(doAddConstant()));

    QHBoxLayout *layoutConstantsButtons = new QHBoxLayout();
    layoutConstantsButtons->addStretch();
    layoutConstantsButtons->addWidget(btnAddConstant);
    */

    // analyses
    treeAnalyses = new QTreeWidget(this);
    treeAnalyses->setMouseTracking(true);
    treeAnalyses->setColumnCount(3);
    treeAnalyses->setColumnWidth(0, 150);
    treeAnalyses->setIndentation(5);
    QStringList headAnalyses;
    headAnalyses << tr("Name") << tr("Type") << tr("Number of Solution");
    treeAnalyses->setHeaderLabels(headAnalyses);

    connect(treeAnalyses, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(analysisDoubleClicked(QTreeWidgetItem *, int)));

    QGridLayout *layoutConstantsAnalysis = new QGridLayout();
    layoutConstantsAnalysis->addWidget(txtDescription, 2, 0, 1, 4);
    layoutConstantsAnalysis->addWidget(new QLabel(tr("Constants:")), 3, 0);
    layoutConstantsAnalysis->addWidget(new QLabel(tr("Analysis:")), 3, 2);
    layoutConstantsAnalysis->addWidget(treeConstants, 4, 0, 1, 2);
    layoutConstantsAnalysis->addWidget(treeAnalyses, 4, 2, 1, 1);
    //layoutConstantsAnalysis->addLayout(layoutConstantsButtons, 5, 0, 1, 1);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutGeneral);
    layout->addLayout(layoutConstantsAnalysis);

    mainWidget->setLayout(layout);
    return mainWidget;
}

QWidget *ModuleDialog::createWeakforms()
{
    // volume weakforms
    treeVolumeQuantityGlobal = new QTreeWidget(this);
    treeVolumeQuantityGlobal->setMouseTracking(true);
    treeVolumeQuantityGlobal->setColumnCount(2);
    treeVolumeQuantityGlobal->setColumnWidth(0, 150);
    treeVolumeQuantityGlobal->setIndentation(12);
    QStringList headVolumeQuantityGlobal;
    headVolumeQuantityGlobal << tr("ID") << tr("Shortname");
    treeVolumeQuantityGlobal->setHeaderLabels(headVolumeQuantityGlobal);

    connect(treeVolumeQuantityGlobal, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(quantityGlobalDoubleClicked(QTreeWidgetItem *, int)));

    treeVolumeQuantityAnalysis = new QTreeWidget(this);
    treeVolumeQuantityAnalysis->setMouseTracking(true);
    treeVolumeQuantityAnalysis->setColumnCount(3);
    treeVolumeQuantityAnalysis->setColumnWidth(0, 150);
    treeVolumeQuantityAnalysis->setIndentation(12);
    QStringList headVolumeQuantityAnalysis;
    headVolumeQuantityAnalysis << tr("ID") << tr("Linearity") << tr("Dependence");
    treeVolumeQuantityAnalysis->setHeaderLabels(headVolumeQuantityAnalysis);

    connect(treeVolumeQuantityAnalysis, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(quantityAnalysisDoubleClicked(QTreeWidgetItem *, int)));

    QGridLayout *layoutVolumeQuantity = new QGridLayout();
    layoutVolumeQuantity->addWidget(new QLabel(tr("Definitions:")), 0, 0);
    layoutVolumeQuantity->addWidget(new QLabel(tr("Parameters for partial analysis:")), 0, 2);
    layoutVolumeQuantity->addWidget(treeVolumeQuantityGlobal, 1, 0, 1, 2);
    layoutVolumeQuantity->addWidget(treeVolumeQuantityAnalysis, 1, 2, 1, 1);

    QGroupBox *grpVolumeQuantities = new QGroupBox(tr("Quantities"));
    grpVolumeQuantities->setLayout(layoutVolumeQuantity);

    treeVolumeWeakforms = new QTreeWidget(this);
    treeVolumeWeakforms->setMouseTracking(true);
    treeVolumeWeakforms->setColumnCount(3);
    treeVolumeWeakforms->setColumnWidth(0, 150);
    treeVolumeWeakforms->setIndentation(12);
    QStringList headVolumeWeakforms;
    headVolumeWeakforms << tr("Type") << tr("i") << tr("j");
    treeVolumeWeakforms->setHeaderLabels(headVolumeWeakforms);

    connect(treeVolumeWeakforms, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(weakformDoubleClicked(QTreeWidgetItem *, int)));

    QVBoxLayout *layoutVolumeWeakforms = new QVBoxLayout();
    layoutVolumeWeakforms->addWidget(treeVolumeWeakforms);

    QGroupBox *grpVolumeWeakforms = new QGroupBox(tr("Weakforms"));
    grpVolumeWeakforms->setLayout(layoutVolumeWeakforms);

    QVBoxLayout *layoutVolume = new QVBoxLayout();
    layoutVolume->addWidget(grpVolumeQuantities);
    layoutVolume->addWidget(grpVolumeWeakforms);

    QWidget *weakformVolume = new QWidget(this);
    weakformVolume->setLayout(layoutVolume);

    // surface weakforms
    treeSurfaceQuantityGlobal = new QTreeWidget(this);
    treeSurfaceQuantityGlobal->setMouseTracking(true);
    treeSurfaceQuantityGlobal->setColumnCount(2);
    treeSurfaceQuantityGlobal->setColumnWidth(0, 150);
    treeSurfaceQuantityGlobal->setIndentation(5);
    QStringList headSurfaceQuantity;
    headSurfaceQuantity << tr("ID") << tr("Shortname");
    treeSurfaceQuantityGlobal->setHeaderLabels(headSurfaceQuantity);

    connect(treeSurfaceQuantityGlobal, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(quantityGlobalDoubleClicked(QTreeWidgetItem *, int)));

    treeSurfaceQuantityAnalysis = new QTreeWidget(this);
    treeSurfaceQuantityAnalysis->setMouseTracking(true);
    treeSurfaceQuantityAnalysis->setColumnCount(3);
    treeSurfaceQuantityAnalysis->setColumnWidth(0, 150);
    treeSurfaceQuantityAnalysis->setIndentation(12);
    QStringList headSurfaceQuantityAnalysis;
    headSurfaceQuantityAnalysis << tr("ID") << tr("Linearity") << tr("Dependence");
    treeSurfaceQuantityAnalysis->setHeaderLabels(headSurfaceQuantityAnalysis);

    connect(treeSurfaceQuantityAnalysis, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(quantityAnalysisDoubleClicked(QTreeWidgetItem *, int)));

    QGridLayout *layoutSurfaceQuantity = new QGridLayout();
    layoutSurfaceQuantity->addWidget(new QLabel(tr("Definitions:")), 0, 0);
    layoutSurfaceQuantity->addWidget(new QLabel(tr("Parameters for partial analysis:")), 0, 2);
    layoutSurfaceQuantity->addWidget(treeSurfaceQuantityGlobal, 1, 0, 1, 2);
    layoutSurfaceQuantity->addWidget(treeSurfaceQuantityAnalysis, 1, 2, 1, 1);

    QGroupBox *grpSurfaceQuantities = new QGroupBox(tr("Quantities"));
    grpSurfaceQuantities->setLayout(layoutSurfaceQuantity);

    treeSurfaceWeakforms = new QTreeWidget(this);
    treeSurfaceWeakforms->setMouseTracking(true);
    treeSurfaceWeakforms->setColumnCount(3);
    treeSurfaceWeakforms->setColumnWidth(0, 150);
    treeSurfaceWeakforms->setIndentation(12);
    QStringList headSurfaceWeakforms;
    headSurfaceWeakforms << tr("Type") << tr("i") << tr("j");
    treeSurfaceWeakforms->setHeaderLabels(headSurfaceWeakforms);

    connect(treeSurfaceWeakforms, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(weakformDoubleClicked(QTreeWidgetItem *, int)));

    QVBoxLayout *layoutSurfaceWeakforms = new QVBoxLayout();
    layoutSurfaceWeakforms->addWidget(treeSurfaceWeakforms);

    QGroupBox *grpSurfaceWeakforms = new QGroupBox(tr("Weakforms"));
    grpSurfaceWeakforms->setLayout(layoutSurfaceWeakforms);

    QVBoxLayout *layoutSurface = new QVBoxLayout();
    layoutSurface->addWidget(grpSurfaceQuantities);
    layoutSurface->addWidget(grpSurfaceWeakforms);

    QWidget *weakformSurface = new QWidget(this);
    weakformSurface->setLayout(layoutSurface);

    // weakform
    QTabWidget *weakformWidget = new QTabWidget(this);
    weakformWidget->addTab(weakformVolume, icon(""), tr("Volume"));
    weakformWidget->addTab(weakformSurface, icon(""), tr("Surface"));

    return weakformWidget;
}

QWidget *ModuleDialog::createPreprocessor()
{
    // materials
    treeMaterials = new QTreeWidget(this);
    treeMaterials->setMouseTracking(true);
    treeMaterials->setColumnCount(3);
    treeMaterials->setColumnWidth(0, 250);
    treeMaterials->setIndentation(12);
    QStringList headMaterials;
    headMaterials << tr("Name") << tr("Shortname") << tr("Unit");
    treeMaterials->setHeaderLabels(headMaterials);

    connect(treeMaterials, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(preprocessorDoubleClicked(QTreeWidgetItem *, int)));

    QVBoxLayout *layoutMaterials = new QVBoxLayout();
    layoutMaterials->addWidget(treeMaterials);
    // layoutLocalVariables->addStretch();

    QWidget *materials = new QWidget(this);
    materials->setLayout(layoutMaterials);

    // boundaries
    treeBoundaries = new QTreeWidget(this);
    treeBoundaries->setMouseTracking(true);
    treeBoundaries->setColumnCount(3);
    treeBoundaries->setColumnWidth(0, 250);
    treeBoundaries->setIndentation(12);
    QStringList headBoundaries;
    headBoundaries << tr("Name") << tr("Shortname") << tr("Unit");
    treeBoundaries->setHeaderLabels(headBoundaries);

    connect(treeBoundaries, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(preprocessorDoubleClicked(QTreeWidgetItem *, int)));

    QVBoxLayout *layoutBoundaries = new QVBoxLayout();
    layoutBoundaries->addWidget(treeBoundaries);
    // layoutVolume->addStretch();

    QWidget *boundaries = new QWidget(this);
    boundaries->setLayout(layoutBoundaries);

    QTabWidget *preprocessorWidget = new QTabWidget(this);
    preprocessorWidget->addTab(materials, icon(""), tr("Materials"));
    preprocessorWidget->addTab(boundaries, icon(""), tr("Boundary conditions"));

    return preprocessorWidget;
}

QWidget *ModuleDialog::createPostprocessor()
{
    // local variables
    treeLocalVariables = new QTreeWidget(this);
    treeLocalVariables->setMouseTracking(true);
    treeLocalVariables->setColumnCount(5);
    treeLocalVariables->setColumnWidth(0, 200);
    treeLocalVariables->setIndentation(5);
    QStringList headLocalVariables;
    headLocalVariables << tr("Name") << tr("Shortname") << tr("Unit") << tr("Type") << tr("Analyses");
    treeLocalVariables->setHeaderLabels(headLocalVariables);

    connect(treeLocalVariables, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(localItemDoubleClicked(QTreeWidgetItem *, int)));

    // view defaults
    treeViewDefaults = new QTreeWidget(this);
    treeViewDefaults->setMouseTracking(true);
    treeViewDefaults->setColumnCount(2);
    treeViewDefaults->setColumnWidth(0, 200);
    treeLocalVariables->setIndentation(5);
    QStringList headViewDefaults;
    headViewDefaults << tr("View") << tr("Local variable");
    treeViewDefaults->setHeaderLabels(headViewDefaults);

    connect(treeViewDefaults, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(viewDefaultsItemDoubleClicked(QTreeWidgetItem *, int)));

    QVBoxLayout *layoutLocalVariables = new QVBoxLayout();
    layoutLocalVariables->addWidget(treeLocalVariables);
    layoutLocalVariables->addWidget(treeViewDefaults);
    // layoutLocalVariables->addStretch();

    QWidget *localVariables = new QWidget(this);
    localVariables->setLayout(layoutLocalVariables);

    // volume integrals
    treeVolumeIntegrals = new QTreeWidget(this);
    treeVolumeIntegrals->setMouseTracking(true);
    treeVolumeIntegrals->setColumnCount(4);
    treeVolumeIntegrals->setColumnWidth(0, 200);
    treeVolumeIntegrals->setIndentation(5);
    QStringList headVolume;
    headVolume << tr("Name") << tr("Shortname") << tr("Unit") << tr("Analyses");
    treeVolumeIntegrals->setHeaderLabels(headVolume);

    connect(treeVolumeIntegrals, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(volumeIntegralDoubleClicked(QTreeWidgetItem *, int)));

    QVBoxLayout *layoutVolume = new QVBoxLayout();
    layoutVolume->addWidget(treeVolumeIntegrals);
    // layoutVolume->addStretch();

    QWidget *volumeIntegrals = new QWidget(this);
    volumeIntegrals->setLayout(layoutVolume);

    // surface integrals
    treeSurfaceIntegrals = new QTreeWidget(this);
    treeSurfaceIntegrals->setMouseTracking(true);
    treeSurfaceIntegrals->setColumnCount(4);
    treeSurfaceIntegrals->setColumnWidth(0, 200);
    treeSurfaceIntegrals->setIndentation(5);
    QStringList headSurface;
    headSurface << tr("Name") << tr("Shortname") << tr("Unit") << tr("Analyses");
    treeSurfaceIntegrals->setHeaderLabels(headSurface);

    connect(treeSurfaceIntegrals, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(surfaceIntegralDoubleClicked(QTreeWidgetItem *, int)));

    QVBoxLayout *layoutSurface = new QVBoxLayout();
    layoutSurface->addWidget(treeSurfaceIntegrals);
    // layoutSurface->addStretch();

    QWidget *surfaceIntegrals = new QWidget(this);
    surfaceIntegrals->setLayout(layoutSurface);

    QTabWidget *postprocessorWidget = new QTabWidget(this);
    postprocessorWidget->addTab(localVariables, icon(""), tr("Local variables"));
    postprocessorWidget->addTab(volumeIntegrals, icon(""), tr("Volume integrals"));
    postprocessorWidget->addTab(surfaceIntegrals, icon(""), tr("Surface integrals"));

    return postprocessorWidget;
}

void ModuleDialog::doCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    pages->setCurrentIndex(lstView->row(current));
}

void ModuleDialog::doAccept()
{
    save();
    accept();
}

void ModuleDialog::doReject()
{
    reject();
}

void ModuleDialog::constantDoubleClicked(QTreeWidgetItem *item, int role)
{
    XMLModule::constant *constant = item->data(0, Qt::UserRole).value<XMLModule::constant *>();
    if (constant)
    {
        ModuleItemConstantDialog dialog(constant, this);
        if (dialog.exec())
        {
            item->setText(0, QString::fromStdString(constant->id()));
            item->setText(1, QString::number(constant->value()));
        }
    }
}

void ModuleDialog::doAddConstant()
{
}

void ModuleDialog::analysisDoubleClicked(QTreeWidgetItem *item, int role)
{
    XMLModule::analysis *analysis = item->data(0, Qt::UserRole).value<XMLModule::analysis *>();
    if (analysis)
    {
        ModuleItemAnalysisDialog dialog(analysis, this);
        if (dialog.exec())
        {
            item->setText(0, QString::fromStdString(analysis->name()));
            item->setText(1, QString::fromStdString(analysis->type()));
            item->setText(2, QString::number(analysis->solutions()));
        }
    }
}

void ModuleDialog::quantityGlobalDoubleClicked(QTreeWidgetItem *item, int role)
{
    XMLModule::quantity *quantity = item->data(0, Qt::UserRole).value<XMLModule::quantity *>();
    if (quantity)
    {
        ModuleItemQuantityGlobalDialog dialog(quantity, this);
        if (dialog.exec())
        {
            item->setText(0, QString::fromStdString(quantity->id()));
            item->setText(1, QString::fromStdString(quantity->shortname().get()));
        }
    }
}

void ModuleDialog::quantityAnalysisDoubleClicked(QTreeWidgetItem *item, int role)
{
    XMLModule::quantity *quantity = item->data(0, Qt::UserRole).value<XMLModule::quantity *>();
    if (quantity)
    {
        ModuleItemQuantityAnalysisDialog dialog(quantity, this);
        if (dialog.exec())
        {
            item->setText(1, tr("linear"));
            if (quantity->nonlinearity_planar().present() || quantity->nonlinearity_axi().present())
            {
                if (!QString::fromStdString(quantity->nonlinearity_planar().get()).isEmpty() ||
                        !QString::fromStdString(quantity->nonlinearity_axi().get()).isEmpty())
                    item->setText(1, tr("nonlinear"));
            }

            item->setText(2, (quantity->dependence().present() && !QString::fromStdString(quantity->dependence().get()).isEmpty()) ?
                              QString::fromStdString(quantity->dependence().get()) : "");
        }
    }
}

void ModuleDialog::weakformDoubleClicked(QTreeWidgetItem *item, int role)
{
    if (item->data(0, role).toString() == tr("Matrix form"))
    {
        XMLModule::matrix_form *form = item->data(0, Qt::UserRole).value<XMLModule::matrix_form *>();
        if (form)
        {
            ModuleItemMatrixFormDialog dialog(form, this);
            if (dialog.exec())
            {
                item->setText(1, QString::number(form->i()));
                item->setText(2, QString::number(form->j()));
            }
        }
    }
    else if (item->data(0, role).toString() == tr("Vector form"))
    {
        XMLModule::vector_form *form = item->data(0, Qt::UserRole).value<XMLModule::vector_form *>();
        if (form)
        {
            ModuleItemVectorFormDialog dialog(form, this);
            if (dialog.exec())
            {
                item->setText(1, QString::number(form->i()));
                item->setText(2, QString::number(form->j()));
            }
        }
    }
    else if (item->data(0, role).toString() == tr("Essential form"))
    {
        XMLModule::essential_form *form = item->data(0, Qt::UserRole).value<XMLModule::essential_form *>();
        if (form)
        {
            ModuleItemEssentialFormDialog dialog(form, this);
            if (dialog.exec())
            {
                item->setText(1, QString::number(form->i()));
            }
        }
    }
}

void ModuleDialog::preprocessorDoubleClicked(QTreeWidgetItem *item, int role)
{
    XMLModule::quantity *quant = item->data(0, Qt::UserRole).value<XMLModule::quantity *>();

    if (quant)
    {
        ModulePreprocessorDialog dialog(quant, this);
        if (dialog.exec())
        {
            item->setText(0, QString::fromStdString(quant->name().get()));
            item->setText(1, QString::fromStdString(quant->shortname().get()));
            item->setText(2, QString::fromStdString(quant->unit().get()));
        }
    }
}

void ModuleDialog::localItemDoubleClicked(QTreeWidgetItem *item, int role)
{
    XMLModule::localvariable *lv = item->data(0, Qt::UserRole).value<XMLModule::localvariable *>();
    if (lv)
    {
        ModuleItemLocalValueDialog dialog(lv, this);
        if (dialog.exec())
        {
            item->setText(0, QString::fromStdString(lv->name()));
            item->setText(1, QString::fromStdString(lv->shortname()));
            item->setText(2, QString::fromStdString(lv->unit()));
            item->setText(3, QString::fromStdString(lv->type()));
        }
    }
}

void ModuleDialog::viewDefaultsItemDoubleClicked(QTreeWidgetItem *item, int role)
{
    XMLModule::default_ *def = item->data(0, Qt::UserRole).value<XMLModule::default_ *>();
    if (def)
    {
        XMLModule::module *module = m_module_xsd.get();
        XMLModule::localvariables *lv = &module->postprocessor().localvariables();

        ModuleItemViewDefaultsDialog dialog(def, lv, this);
        if (dialog.exec())
            item->setText(1, QString::fromStdString(def->id()));
    }
}

void ModuleDialog::surfaceIntegralDoubleClicked(QTreeWidgetItem *item, int role)
{
    XMLModule::surfaceintegral *sur = item->data(0, Qt::UserRole).value<XMLModule::surfaceintegral *>();
    if (sur)
    {
        ModuleSurfaceIntegralValueDialog dialog(sur, this);
        if (dialog.exec())
        {
            item->setText(0, QString::fromStdString(sur->name()));
            item->setText(1, QString::fromStdString(sur->shortname()));
            item->setText(2, QString::fromStdString(sur->unit()));
        }
    }
}

void ModuleDialog::volumeIntegralDoubleClicked(QTreeWidgetItem *item, int role)
{
    XMLModule::volumeintegral *vol = item->data(0, Qt::UserRole).value<XMLModule::volumeintegral *>();
    if (vol)
    {
        ModuleVolumeIntegralValueDialog dialog(vol, this);
        if (dialog.exec())
        {
            item->setText(0, QString::fromStdString(vol->name()));
            item->setText(1, QString::fromStdString(vol->shortname()));
            item->setText(2, QString::fromStdString(vol->unit()));
        }
    }
}
