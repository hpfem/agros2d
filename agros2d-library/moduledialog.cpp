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
#include "pythonlab/pythonhighlighter.h"

#include "scene.h"
#include "sceneview_common.h"
#include "pythonlab/pythonengine_agros.h"

#include "hermes2d/module.h"

#include "util/constants.h"
#include "gui/common.h"
#include "gui/htmledit.h"
#include "gui/systemoutput.h"
#include "gui/lineeditdouble.h"

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

ModuleHighlighter::ModuleHighlighter(QTextDocument *textDocument) : QSyntaxHighlighter(textDocument)
{
    HighlightingRule rule;

    quotationFormat.setForeground(Qt::darkGreen);
    rule.format = quotationFormat;
    rule.pattern = QRegExp("\"[^\"\\\\]*(\\\\.[^\"\\\\]*)*\"");
    highlightingRules.append(rule);
    rule.pattern = QRegExp("'[^'\\\\]*(\\\\.[^'\\\\]*)*'");
    highlightingRules.append(rule);

    operatorFormat.setForeground(Qt::black);
    rule.pattern = QRegExp("[\\\\|\\<|\\>|\\=|\\!|\\+|\\-|\\*|\\/|\\%]+");
    rule.pattern.setMinimal(true);
    rule.format = operatorFormat;
    highlightingRules.append(rule);

    numberFormat.setForeground(Qt::blue);
    rule.format = numberFormat;
    rule.pattern = QRegExp("\\b[+-]?[0-9]+[lL]?\\b");
    highlightingRules.append(rule);
    rule.pattern = QRegExp("\\b[+-]?0[xX][0-9A-Fa-f]+[lL]?\\b");
    highlightingRules.append(rule);
    rule.pattern = QRegExp("\\b[+-]?[0-9]+(?:\\.[0-9]+)?(?:[eE][+-]?[0-9]+)?\\b");
    highlightingRules.append(rule);
}

void ModuleHighlighter::setKeywords(QStringList patterns, const QColor &color)
{
    keywordFormat.setForeground(color);

    HighlightingRule rule;
    foreach (const QString &pattern, patterns)
    {
        rule.pattern = QRegExp("\\b" + pattern + "\\b", Qt::CaseInsensitive);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }
}

void ModuleHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules)
    {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);

    // parenthesis
    highlightBlockParenthesis(text, '(', ')');
}

void ModuleHighlighter::highlightBlockParenthesis(const QString &text, char left, char right)
{
    TextBlockData *data = new TextBlockData();

    int leftPos = text.indexOf(left);
    while (leftPos != -1)
    {
        ParenthesisInfo *info = new ParenthesisInfo();
        info->character = left;
        info->position = leftPos;

        data->insert(info);
        leftPos = text.indexOf(left, leftPos + 1);
    }

    int rightPos = text.indexOf(right);
    while (rightPos != -1)
    {
        ParenthesisInfo *info = new ParenthesisInfo();
        info->character = right;
        info->position = rightPos;

        data->insert(info);
        rightPos = text.indexOf(right, rightPos + 1);
    }

    setCurrentBlockUserData(data);
}

// ********************************************************************************************

ModuleItem::ModuleItem(QWidget *parent)
    : QWidget(parent)
{
}

ModuleItemLocalValue::ModuleItemLocalValue(ModuleDialog *moduleDialog, const QString &type,
                                           XMLModule::expression *expr, QWidget *parent)
    : ModuleItem(parent), m_type(type), m_expr(expr)
{
    QGridLayout *layout = new QGridLayout();

    int numberOfSolutions = moduleDialog->numberOfSolutions(expr->analysistype());

    if (m_type == "scalar")
    {
        txtPlanar = new ModuleDialogTextEdit(this, 1);
        txtPlanar->setPostprocessorVolumeHighlighter(moduleDialog->module()->volume().quantity(),
                                                     numberOfSolutions, CoordinateType_Planar);
        txtAxi = new ModuleDialogTextEdit(this, 1);
        txtAxi->setPostprocessorVolumeHighlighter(moduleDialog->module()->volume().quantity(),
                                                  numberOfSolutions, CoordinateType_Axisymmetric);

        layout->addWidget(new QLabel(tr("Planar:")), 0, 0);
        layout->addWidget(txtPlanar, 0, 1);
        layout->addWidget(new QLabel(tr("Axisymmetric:")), 1, 0);
        layout->addWidget(txtAxi, 1, 1);

        txtPlanar->setText(QString::fromStdString(expr->planar().get()));
        txtAxi->setText(QString::fromStdString(expr->axi().get()));
    }
    else
    {
        txtPlanarX = new ModuleDialogTextEdit(this, 1);
        txtPlanarX->setPostprocessorVolumeHighlighter(moduleDialog->module()->volume().quantity(),
                                                      numberOfSolutions, CoordinateType_Planar);
        txtPlanarY = new ModuleDialogTextEdit(this, 1);
        txtPlanarY->setPostprocessorVolumeHighlighter(moduleDialog->module()->volume().quantity(),
                                                      numberOfSolutions, CoordinateType_Planar);
        txtAxiR = new ModuleDialogTextEdit(this, 1);
        txtAxiR->setPostprocessorVolumeHighlighter(moduleDialog->module()->volume().quantity(),
                                                   numberOfSolutions, CoordinateType_Axisymmetric);
        txtAxiZ = new ModuleDialogTextEdit(this, 1);
        txtAxiZ->setPostprocessorVolumeHighlighter(moduleDialog->module()->volume().quantity(),
                                                   numberOfSolutions, CoordinateType_Axisymmetric);

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

    setLayout(layout);
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

ModuleDialogTextEdit::ModuleDialogTextEdit(QWidget *parent, int rows)
    : PlainTextEditParenthesis(parent), m_rows(rows)
{
    m_highlighter = new ModuleHighlighter(this->document());

    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(matchParentheses()));

    // set minimum size
    setMinimumSize(sizeHint());
}

void ModuleDialogTextEdit::addToolTip(const QString &label, QStringList patterns)
{
    QString list;
    foreach (QString str, patterns)
    {
        list += str + ", ";
    }
    if (list.length() > 2)
        list = list.left(list.length() - 2);

    setToolTip(QString("%1<h4>%2</h4>%3").
               arg(toolTip()).
               arg(label).
               arg(list));
}

void ModuleDialogTextEdit::setCoordinates(CoordinateType coordinateType)
{
    QStringList patternsCoordinates;

    if (coordinateType == CoordinateType_Planar)
    {
        patternsCoordinates.append("x");
        patternsCoordinates.append("y");
    }
    else
    {
        patternsCoordinates.append("r");
        patternsCoordinates.append("z");
    }

    // highlight coordinates
    m_highlighter->setKeywords(patternsCoordinates, Qt::darkBlue);
    addToolTip(tr("Coordinates"), patternsCoordinates);
}

void ModuleDialogTextEdit::setWeakformFuntions(CoordinateType coordinateType)
{
    QStringList patternsFunctions;

    if (coordinateType == CoordinateType_Planar)
    {
        patternsFunctions.append("uval");
        patternsFunctions.append("udx");
        patternsFunctions.append("udy");

        patternsFunctions.append("upval");
        patternsFunctions.append("updx");
        patternsFunctions.append("updy");

        patternsFunctions.append("vval");
        patternsFunctions.append("vdx");
        patternsFunctions.append("vdy");
    }
    else
    {
        patternsFunctions.append("uval");
        patternsFunctions.append("udr");
        patternsFunctions.append("udz");

        patternsFunctions.append("upval");
        patternsFunctions.append("updr");
        patternsFunctions.append("updz");

        patternsFunctions.append("vval");
        patternsFunctions.append("vdr");
        patternsFunctions.append("vdz");
    }
    m_highlighter->setKeywords(patternsFunctions, Qt::darkMagenta);
    addToolTip(tr("Functions"), patternsFunctions);
}

void ModuleDialogTextEdit::setWeakformVolumeHighlighter(XMLModule::volume::quantity_sequence sequence, int numberOfSolutions, CoordinateType coordinateType)
{
    // highlight coordinates
    setCoordinates(coordinateType);

    // highlight functions u and v
    setWeakformFuntions(coordinateType);

    // highlight variables
    QStringList patterns;
    for (int i = 0; i < sequence.size(); i++)
    {
        XMLModule::quantity quantity = sequence.at(i);
        patterns.append(QString::fromStdString(quantity.shortname().get()));
    }
    m_highlighter->setKeywords(patterns, Qt::darkGreen);
    addToolTip(tr("Variables"), patterns);
}

void ModuleDialogTextEdit::setWeakformSurfaceHighlighter(XMLModule::surface::quantity_sequence sequence, int numberOfSolutions, CoordinateType coordinateType)
{
    // highlight coordinates
    setCoordinates(coordinateType);

    // highlight functions u and v
    setWeakformFuntions(coordinateType);

    // highlight variables
    QStringList patterns;
    for (int i = 0; i < sequence.size(); i++)
    {
        XMLModule::quantity quantity = sequence.at(i);
        patterns.append(QString::fromStdString(quantity.shortname().get()));
    }
    m_highlighter->setKeywords(patterns, Qt::darkGreen);
    addToolTip(tr("Variables"), patterns);
}

void ModuleDialogTextEdit::setPostprocessorFuntions(int numberOfSolutions, CoordinateType coordinateType)
{
    QStringList patternsFunctions;
    for (int i = 1; i < numberOfSolutions + 1; i++)
    {
        patternsFunctions.append(QString("value%1").arg(i));
        if (coordinateType == CoordinateType_Planar)
        {
            patternsFunctions.append(QString("dx%1").arg(i));
            patternsFunctions.append(QString("dy%1").arg(i));
        }
        else
        {
            patternsFunctions.append(QString("dr%1").arg(i));
            patternsFunctions.append(QString("dz%1").arg(i));
        }
        patternsFunctions.append(QString("value%1").arg(i));
    }
    m_highlighter->setKeywords(patternsFunctions, Qt::darkMagenta);
    addToolTip(tr("Functions"), patternsFunctions);
}

void ModuleDialogTextEdit::setPostprocessorVolumeHighlighter(XMLModule::volume::quantity_sequence sequence, int numberOfSolutions, CoordinateType coordinateType)
{
    // highlight coordinates
    setCoordinates(coordinateType);

    // highlight functions
    setPostprocessorFuntions(numberOfSolutions, coordinateType);

    // highlight variables
    QStringList patterns;
    for (int i = 0; i < sequence.size(); i++)
    {
        XMLModule::quantity quantity = sequence.at(i);
        patterns.append(QString::fromStdString(quantity.shortname().get()));
    }
    m_highlighter->setKeywords(patterns, Qt::darkGreen);
    addToolTip(tr("Variables"), patterns);
}

void ModuleDialogTextEdit::setPostprocessorSurfaceHighlighter(XMLModule::surface::quantity_sequence sequence, int numberOfSolutions, CoordinateType coordinateType)
{
    // highlight coordinates
    setCoordinates(coordinateType);

    // highlight functions
    setPostprocessorFuntions(numberOfSolutions, coordinateType);

    // highlight variables
    QStringList patterns;
    for (int i = 0; i < sequence.size(); i++)
    {
        XMLModule::quantity quantity = sequence.at(i);
        patterns.append(QString::fromStdString(quantity.shortname().get()));
    }
    m_highlighter->setKeywords(patterns, Qt::darkGreen);
    addToolTip(tr("Variables"), patterns);
}

QSize ModuleDialogTextEdit::sizeHint() const
{
    QFontMetrics fm(font());
    int h = fm.height();
    int w = fm.width(QLatin1Char('m'));

    return QSize(30 * w, 8 + h* m_rows);
}

ModuleItemEmptyDialog::ModuleItemEmptyDialog(ModuleDialog *moduleDialog, QWidget *parent)
    : QDialog(parent), m_module(moduleDialog)
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

/*
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

    QGridLayout *layout = new QGridLayout();
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
*/

// ***********************************************************************************************************************

ModuleItemConstantDialog::ModuleItemConstantDialog(ModuleDialog *moduleDialog, XMLModule::constant *constant, QWidget *parent)
    : ModuleItemEmptyDialog(moduleDialog, parent), m_constant(constant)
{
    txtID = new QLineEdit();
    txtID->setText(QString::fromStdString(constant->id()));

    txtValue = new LineEditDouble();
    txtValue->setValue(constant->value());

    QGridLayout *layoutGeneral = new QGridLayout();
    layoutGeneral->addWidget(new QLabel(tr("ID:")), 0, 0);
    layoutGeneral->addWidget(txtID, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("Value:")), 1, 0);
    layoutGeneral->addWidget(txtValue, 1, 1);

    layoutMain->addLayout(layoutGeneral);
    layoutMain->addStretch();
    layoutMain->addWidget(buttonBox);

    QSettings settings;
    restoreGeometry(settings.value("ModuleItemConstantDialog/Geometry", saveGeometry()).toByteArray());
}

ModuleItemConstantDialog::~ModuleItemConstantDialog()
{
    QSettings settings;
    settings.setValue("ModuleItemConstantDialog/Geometry", saveGeometry());
}

void ModuleItemConstantDialog::doAccept()
{
    m_constant->id(txtID->text().toStdString());
    m_constant->value(txtValue->value());

    accept();
}

// ***********************************************************************************************************************

ModuleItemAnalysisDialog::ModuleItemAnalysisDialog(ModuleDialog *moduleDialog, XMLModule::analysis *analysis, QWidget *parent)
    : ModuleItemEmptyDialog(moduleDialog, parent), m_analysis(analysis)
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

    QGridLayout *layoutGeneral = new QGridLayout();
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

ModuleItemQuantityGlobalDialog::ModuleItemQuantityGlobalDialog(ModuleDialog *moduleDialog, XMLModule::quantity *quantity, QWidget *parent)
    : ModuleItemEmptyDialog(moduleDialog, parent), m_quantity(quantity)
{
    txtID = new QLineEdit();
    txtID->setText(QString::fromStdString(quantity->id()));

    txtShortname = new QLineEdit();
    txtShortname->setText(QString::fromStdString(quantity->shortname().get()));

    QGridLayout *layoutGeneral = new QGridLayout();
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

ModuleItemQuantityAnalysisDialog::ModuleItemQuantityAnalysisDialog(ModuleDialog *moduleDialog, XMLModule::quantity *quantity, QWidget *parent)
    : ModuleItemEmptyDialog(moduleDialog, parent), m_quantity(quantity)
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

    QGridLayout *layoutGeneral = new QGridLayout();
    layoutGeneral->addWidget(new QLabel(tr("Planar nonlinearity:")), 0, 0);
    layoutGeneral->addWidget(txtPlanarNonlinearity, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("Axisymmetric nonlinearity:")), 1, 0);
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

ModuleItemWeakformDialog::ModuleItemWeakformDialog(ModuleDialog *moduleDialog, QWidget *parent)
    : ModuleItemEmptyDialog(moduleDialog, parent)
{
    setWindowTitle(tr("Matrix form"));
    setWindowIcon(icon("form-matrix"));

    txtID = new QLineEdit();

    txtSolutionIndex = new QSpinBox();
    txtSolutionIndex->setMinimum(1);
    txtTestFunctionIndex = new QSpinBox();
    txtTestFunctionIndex->setMinimum(1);

    txtPlanarLinear = new ModuleDialogTextEdit(this, 3);
    txtPlanarNewton = new ModuleDialogTextEdit(this, 3);
    txtAxiLinear = new ModuleDialogTextEdit(this, 3);
    txtAxiNewton = new ModuleDialogTextEdit(this, 3);

    QGridLayout *layoutGeneral = new QGridLayout();

    layoutGeneral->addWidget(new QLabel(tr("ID:")), 0, 0);
    layoutGeneral->addWidget(txtID, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("Solution index:")), 1, 0);
    layoutGeneral->addWidget(txtSolutionIndex, 1, 1);
    layoutGeneral->addWidget(new QLabel(tr("Test function index:")), 2, 0);
    layoutGeneral->addWidget(txtTestFunctionIndex, 2, 1);

    QGridLayout *layoutLinear = new QGridLayout();
    layoutLinear->addWidget(new QLabel(tr("Planar:")), 0, 0);
    layoutLinear->addWidget(txtPlanarLinear, 1, 0);
    layoutLinear->addWidget(new QLabel(tr("Axisymmetric:")), 2, 0);
    layoutLinear->addWidget(txtAxiLinear, 3, 0);

    QGroupBox *grpLinear = new QGroupBox(tr("Linear"));
    grpLinear->setLayout(layoutLinear);

    QGridLayout *layoutNewton = new QGridLayout();
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

ModuleItemMatrixFormDialog::ModuleItemMatrixFormDialog(ModuleDialog *moduleDialog, QString type, int numberOfSolutions,
                                                       XMLModule::matrix_form *form, QWidget *parent)
    : ModuleItemWeakformDialog(moduleDialog, parent), m_form(form)
{
    txtID->setText(QString::fromStdString(m_form->id()));
    txtSolutionIndex->setValue(m_form->i().get());
    txtTestFunctionIndex->setValue(m_form->j().get());

    if (type == "volume")
    {
        txtPlanarLinear->setWeakformVolumeHighlighter(moduleDialog->module()->volume().quantity(), numberOfSolutions, CoordinateType_Planar);
        txtPlanarNewton->setWeakformVolumeHighlighter(moduleDialog->module()->volume().quantity(), numberOfSolutions, CoordinateType_Planar);
        txtAxiLinear->setWeakformVolumeHighlighter(moduleDialog->module()->volume().quantity(), numberOfSolutions, CoordinateType_Axisymmetric);
        txtAxiNewton->setWeakformVolumeHighlighter(moduleDialog->module()->volume().quantity(), numberOfSolutions, CoordinateType_Axisymmetric);
    }
    else if (type == "surface")
    {
        txtPlanarLinear->setWeakformSurfaceHighlighter(moduleDialog->module()->surface().quantity(), numberOfSolutions, CoordinateType_Planar);
        txtPlanarNewton->setWeakformSurfaceHighlighter(moduleDialog->module()->surface().quantity(), numberOfSolutions, CoordinateType_Planar);
        txtAxiLinear->setWeakformSurfaceHighlighter(moduleDialog->module()->surface().quantity(), numberOfSolutions, CoordinateType_Axisymmetric);
        txtAxiNewton->setWeakformSurfaceHighlighter(moduleDialog->module()->surface().quantity(), numberOfSolutions, CoordinateType_Axisymmetric);
    }

    assert(0);
//    txtPlanarLinear->setText(QString::fromStdString(m_form->planar_linear()));
//    txtAxiLinear->setText(QString::fromStdString(m_form->axi_linear()));
//    txtPlanarNewton->setText(QString::fromStdString(m_form->planar_newton()));
//    txtAxiNewton->setText(QString::fromStdString(m_form->axi_newton()));

    QSettings settings;
    restoreGeometry(settings.value("ModuleItemMatrixFormDialog/Geometry", saveGeometry()).toByteArray());
}

ModuleItemMatrixFormDialog::~ModuleItemMatrixFormDialog()
{
    QSettings settings;
    settings.setValue("ModuleItemMatrixFormDialog/Geometry", saveGeometry());
}

void ModuleItemMatrixFormDialog::doAccept()
{
    m_form->id(txtID->text().toStdString());
    m_form->i(txtSolutionIndex->value());
    m_form->j(txtTestFunctionIndex->value());
    assert(0);
//    m_form->planar_linear(txtPlanarLinear->toPlainText().toStdString());
//    m_form->axi_linear(txtAxiLinear->toPlainText().toStdString());
//    m_form->planar_newton(txtPlanarNewton->toPlainText().toStdString());
//    m_form->axi_newton(txtAxiNewton->toPlainText().toStdString());

    accept();
}

ModuleItemVectorFormDialog::ModuleItemVectorFormDialog(ModuleDialog *moduleDialog, QString type, int numberOfSolutions,
                                                       XMLModule::vector_form *form, QWidget *parent)
    : ModuleItemWeakformDialog(moduleDialog, parent), m_form(form)
{
    setWindowTitle(tr("Vector form"));
    setWindowIcon(icon("form-vector"));

    txtID->setText(QString::fromStdString(m_form->id()));
    txtSolutionIndex->setValue(m_form->i().get());
    txtTestFunctionIndex->setValue(m_form->j().get());

    if (type == "volume")
    {
        txtPlanarLinear->setWeakformVolumeHighlighter(moduleDialog->module()->volume().quantity(), numberOfSolutions, CoordinateType_Planar);
        txtPlanarNewton->setWeakformVolumeHighlighter(moduleDialog->module()->volume().quantity(), numberOfSolutions, CoordinateType_Planar);
        txtAxiLinear->setWeakformVolumeHighlighter(moduleDialog->module()->volume().quantity(), numberOfSolutions, CoordinateType_Axisymmetric);
        txtAxiNewton->setWeakformVolumeHighlighter(moduleDialog->module()->volume().quantity(), numberOfSolutions, CoordinateType_Axisymmetric);
    }
    else if (type == "surface")
    {
        txtPlanarLinear->setWeakformSurfaceHighlighter(moduleDialog->module()->surface().quantity(), numberOfSolutions, CoordinateType_Planar);
        txtPlanarNewton->setWeakformSurfaceHighlighter(moduleDialog->module()->surface().quantity(), numberOfSolutions, CoordinateType_Planar);
        txtAxiLinear->setWeakformSurfaceHighlighter(moduleDialog->module()->surface().quantity(), numberOfSolutions, CoordinateType_Axisymmetric);
        txtAxiNewton->setWeakformSurfaceHighlighter(moduleDialog->module()->surface().quantity(), numberOfSolutions, CoordinateType_Axisymmetric);
    }

    assert(0);
//    txtPlanarLinear->setText(QString::fromStdString(m_form->planar_linear()));
//    txtAxiLinear->setText(QString::fromStdString(m_form->axi_linear()));
//    txtPlanarNewton->setText(QString::fromStdString(m_form->planar_newton()));
//    txtAxiNewton->setText(QString::fromStdString(m_form->axi_newton()));

    QSettings settings;
    restoreGeometry(settings.value("ModuleItemVectorFormDialog/Geometry", saveGeometry()).toByteArray());
}

ModuleItemVectorFormDialog::~ModuleItemVectorFormDialog()
{
    QSettings settings;
    settings.setValue("ModuleItemVectorFormDialog/Geometry", saveGeometry());
}

void ModuleItemVectorFormDialog::doAccept()
{
    m_form->i(txtSolutionIndex->text().toInt());
    m_form->j(txtTestFunctionIndex->text().toInt());

    assert(0);
//    m_form->planar_linear(txtPlanarLinear->toPlainText().toStdString());
//    m_form->axi_linear(txtAxiLinear->toPlainText().toStdString());
//    m_form->planar_newton(txtPlanarNewton->toPlainText().toStdString());
//    m_form->axi_newton(txtAxiNewton->toPlainText().toStdString());

    accept();
}

// ***********************************************************************************************************************

ModuleItemEssentialFormDialog::ModuleItemEssentialFormDialog(ModuleDialog *moduleDialog, int numberOfSolutions, XMLModule::essential_form *form, QWidget *parent)
    : ModuleItemWeakformDialog(moduleDialog, parent), m_form(form)
{
    setWindowTitle(tr("Essential form"));
    setWindowIcon(icon("form-essential"));

    txtID->setText("function");
    txtSolutionIndex->setValue(m_form->i().get());
    txtTestFunctionIndex->setValue(0);
    txtTestFunctionIndex->setDisabled(true);

    txtPlanarLinear->setWeakformSurfaceHighlighter(moduleDialog->module()->surface().quantity(), numberOfSolutions, CoordinateType_Planar);
    txtPlanarNewton->setWeakformSurfaceHighlighter(moduleDialog->module()->surface().quantity(), numberOfSolutions, CoordinateType_Planar);
    txtAxiLinear->setWeakformSurfaceHighlighter(moduleDialog->module()->surface().quantity(), numberOfSolutions, CoordinateType_Axisymmetric);
    txtAxiNewton->setWeakformSurfaceHighlighter(moduleDialog->module()->surface().quantity(), numberOfSolutions, CoordinateType_Axisymmetric);

    assert(0);
//    txtPlanarLinear->setText(QString::fromStdString(m_form->planar_linear()));
//    txtAxiLinear->setText(QString::fromStdString(m_form->axi_linear()));
//    txtPlanarNewton->setText(QString::fromStdString(m_form->planar_newton()));
//    txtAxiNewton->setText(QString::fromStdString(m_form->axi_newton()));

    QSettings settings;
    restoreGeometry(settings.value("ModuleItemEssentialFormDialog/Geometry", saveGeometry()).toByteArray());
}

ModuleItemEssentialFormDialog::~ModuleItemEssentialFormDialog()
{
    QSettings settings;
    settings.setValue("ModuleItemEssentialFormDialog/Geometry", saveGeometry());
}

void ModuleItemEssentialFormDialog::doAccept()
{
    m_form->i(txtSolutionIndex->text().toInt());

    assert(0);
//    m_form->planar_linear(txtPlanarLinear->toPlainText().toStdString());
//    m_form->axi_linear(txtAxiLinear->toPlainText().toStdString());
//    m_form->planar_newton(txtPlanarNewton->toPlainText().toStdString());
//    m_form->axi_newton(txtAxiNewton->toPlainText().toStdString());

    accept();
}

// ***********************************************************************************************************************

ModulePreprocessorDialog::ModulePreprocessorDialog(ModuleDialog *moduleDialog, XMLModule::quantity *quant, QWidget *parent)
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

    txtCondition = new QLineEdit();
    if (quant->condition().present())
        txtCondition->setText(QString::fromStdString(quant->condition().get()));

    txtDefault = new QLineEdit();
    txtDefault->setValidator(new QDoubleValidator(txtDefault));
    if (quant->default_().present())
        txtDefault->setText(QString::number(quant->default_().get()));

    QGridLayout *layoutConstraints = new QGridLayout;
    layoutConstraints->addWidget(new QLabel(tr("Condition:")), 0, 1);
    layoutConstraints->addWidget(txtCondition, 0, 2);
    layoutConstraints->addWidget(new QLabel(tr("Default:")), 0, 3);
    layoutConstraints->addWidget(txtDefault, 0, 4);

    QGroupBox *grpConstraints = new QGroupBox(tr("Constraints and default value"));
    grpConstraints->setLayout(layoutConstraints);

    layoutMain->addWidget(grpConstraints);
    layoutMain->addStretch();
    layoutMain->addWidget(buttonBox);

    QSettings settings;
    restoreGeometry(settings.value("ModulePreprocessorDialog/Geometry", saveGeometry()).toByteArray());
}

ModulePreprocessorDialog::~ModulePreprocessorDialog()
{
    QSettings settings;
    settings.setValue("ModulePreprocessorDialog/Geometry", saveGeometry());
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
    m_quant->condition(txtCondition->text().toStdString());
    m_quant->default_(txtDefault->text().toDouble());

    accept();
}

// ***********************************************************************************************************************

ModuleItemLocalValueDialog::ModuleItemLocalValueDialog(ModuleDialog *moduleDialog, XMLModule::localvariable *lv, QWidget *parent)
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

    QTabWidget *tabWidget = new QTabWidget(this);

    for (int i = 0; i < m_lv->expression().size(); i++)
    {
        XMLModule::expression *expr = &lv->expression().at(i);

        ModuleItemLocalValue *item = new ModuleItemLocalValue(moduleDialog, QString::fromStdString(lv->type()), expr, this);
        items.append(item);

        tabWidget->addTab(item, analysisTypeString(analysisTypeFromStringKey(QString::fromStdString(expr->analysistype()))));
    }

    layoutMain->addWidget(tabWidget);
    layoutMain->addStretch();
    layoutMain->addWidget(buttonBox);

    QSettings settings;
    restoreGeometry(settings.value("ModuleItemLocalValueDialog/Geometry", saveGeometry()).toByteArray());
}

ModuleItemLocalValueDialog::~ModuleItemLocalValueDialog()
{
    QSettings settings;
    settings.setValue("ModuleItemLocalValueDialog/Geometry", saveGeometry());
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

ModuleItemViewDefaultsDialog::ModuleItemViewDefaultsDialog(ModuleDialog *moduleDialog,
                                                           XMLModule::default_ *def, XMLModule::localvariables *lv,
                                                           QWidget *parent)
    : ModuleItemEmptyDialog(moduleDialog, parent), m_def(def), m_lv(lv)
{
    cmbLocalVariable = new QComboBox();
    for (int i = 0; i < lv->localvariable().size(); i++)
    {
        XMLModule::localvariable *variable = &lv->localvariable().at(i);
        cmbLocalVariable->addItem(QString::fromStdString(variable->id()));
    }
    cmbLocalVariable->setCurrentIndex(cmbLocalVariable->findText(QString::fromStdString(m_def->id())));

    QGridLayout *layoutGeneral = new QGridLayout();
    layoutGeneral->addWidget(new QLabel(tr("Local variable:")), 0, 0);
    layoutGeneral->addWidget(cmbLocalVariable, 0, 1);

    layoutMain->addLayout(layoutGeneral);
    layoutMain->addStretch();
    layoutMain->addWidget(buttonBox);

    QSettings settings;
    restoreGeometry(settings.value("ModuleItemViewDefaultsDialog/Geometry", saveGeometry()).toByteArray());
}

ModuleItemViewDefaultsDialog::~ModuleItemViewDefaultsDialog()
{
    QSettings settings;
    settings.setValue("ModuleItemViewDefaultsDialog/Geometry", saveGeometry());
}

void ModuleItemViewDefaultsDialog::doAccept()
{
    m_def->id(cmbLocalVariable->currentText().toStdString());

    accept();
}

// ***********************************************************************************************************************

ModuleVolumeIntegralValueDialog::ModuleVolumeIntegralValueDialog(ModuleDialog *moduleDialog, XMLModule::volumeintegral *vol, QWidget *parent)
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

        ModuleItemLocalValue *item = new ModuleItemLocalValue(moduleDialog, "scalar", expr, this);
        items.append(item);

        QHBoxLayout *layoutGeneral = new QHBoxLayout();
        layoutGeneral->addWidget(item);

        QGroupBox *grpGeneral = new QGroupBox(analysisTypeString(analysisTypeFromStringKey(QString::fromStdString(expr->analysistype()))));
        grpGeneral->setLayout(layoutGeneral);

        layoutMain->addWidget(grpGeneral);
    }

    layoutMain->addStretch();
    layoutMain->addWidget(buttonBox);

    QSettings settings;
    restoreGeometry(settings.value("ModuleVolumeIntegralValueDialog/Geometry", saveGeometry()).toByteArray());
}

ModuleVolumeIntegralValueDialog::~ModuleVolumeIntegralValueDialog()
{
    QSettings settings;
    settings.setValue("ModuleVolumeIntegralValueDialog/Geometry", saveGeometry());
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

ModuleSurfaceIntegralValueDialog::ModuleSurfaceIntegralValueDialog(ModuleDialog *moduleDialog, XMLModule::surfaceintegral *sur, QWidget *parent)
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

        ModuleItemLocalValue *item = new ModuleItemLocalValue(moduleDialog, "scalar", expr, this);
        items.append(item);

        QHBoxLayout *layoutGeneral = new QHBoxLayout();
        layoutGeneral->addWidget(item);

        QGroupBox *grpGeneral = new QGroupBox(analysisTypeString(analysisTypeFromStringKey(QString::fromStdString(expr->analysistype()))));
        grpGeneral->setLayout(layoutGeneral);

        layoutMain->addWidget(grpGeneral);
    }

    layoutMain->addStretch();
    layoutMain->addWidget(buttonBox);

    QSettings settings;
    restoreGeometry(settings.value("ModuleSurfaceIntegralValueDialog/Geometry", saveGeometry()).toByteArray());
}

ModuleSurfaceIntegralValueDialog::~ModuleSurfaceIntegralValueDialog()
{
    QSettings settings;
    settings.setValue("ModuleSurfaceIntegralValueDialog/Geometry", saveGeometry());
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

    m_module_xsd = XMLModule::module_(QString("%1%2/%3.xml").arg(datadir()).arg(MODULEROOT).arg(m_fieldId).toLatin1().data());

    load();

    setMinimumSize(QSize(950, 600));

    QSettings settings;
    restoreGeometry(settings.value("ModuleDialog/Geometry", saveGeometry()).toByteArray());
}

ModuleDialog::~ModuleDialog()
{
    QSettings settings;
    settings.setValue("ModuleDialog/Geometry", saveGeometry());
}

void ModuleDialog::load()
{
    ModuleDialog *moduleDialog = this;

    // main
    txtId->setText(QString::fromStdString(moduleDialog->module()->general().id()));
    txtName->setText(QString::fromStdString(moduleDialog->module()->general().name()));
    txtDescription->setPlainText(QString::fromStdString(moduleDialog->module()->general().description()));

    // constants
    treeConstants->clear();
    for (int i = 0; i < moduleDialog->module()->constants().constant().size(); i++)
    {
        XMLModule::constant *cnst = &moduleDialog->module()->constants().constant().at(i);

        QTreeWidgetItem *item = new QTreeWidgetItem(treeConstants);

        item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::constant *>(cnst));
        item->setText(0, QString::fromStdString(cnst->id()));
        item->setText(1, QString::number(cnst->value()));
    }

    // analyses
    treeAnalyses->clear();
    for (int i = 0; i < moduleDialog->module()->general().analyses().analysis().size(); i++)
    {
        XMLModule::analysis *analysis = &moduleDialog->module()->general().analyses().analysis().at(i);

        QTreeWidgetItem *item = new QTreeWidgetItem(treeAnalyses);

        item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::analysis *>(analysis));
        item->setText(0, QString::fromStdString(analysis->name()));
        item->setText(1, QString::fromStdString(analysis->type()));
        item->setText(2, QString::number(analysis->solutions()));
    }

    // volume weakform quantities
    treeVolumeQuantityGlobal->clear();
    for (int i = 0; i < moduleDialog->module()->volume().quantity().size(); i++)
    {
        XMLModule::quantity *quantity = &moduleDialog->module()->volume().quantity().at(i);

        QTreeWidgetItem *item = new QTreeWidgetItem(treeVolumeQuantityGlobal);

        item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::quantity *>(quantity));
        item->setText(0, QString::fromStdString(quantity->id()));
        item->setText(1, QString::fromStdString((quantity->shortname().present()) ? quantity->shortname().get() : ""));
    }

    treeVolumeQuantityAnalysis->clear();
    for (int i = 0; i < moduleDialog->module()->volume().weakforms_volume().weakform_volume().size(); i++)
    {
        XMLModule::weakform_volume *wf = &moduleDialog->module()->volume().weakforms_volume().weakform_volume().at(i);

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
    for (int i = 0; i < moduleDialog->module()->volume().weakforms_volume().weakform_volume().size(); i++)
    {
        XMLModule::weakform_volume *wf = &moduleDialog->module()->volume().weakforms_volume().weakform_volume().at(i);

        QTreeWidgetItem *analysis = new QTreeWidgetItem(treeVolumeWeakforms);
        analysis->setExpanded(true);
        analysis->setText(0, analysisTypeString(analysisTypeFromStringKey(QString::fromStdString(wf->analysistype()))));

        // TODO:
//        // matrix form
//        for (int i = 0; i < wf->matrix_form().size(); i++)
//        {
//            XMLModule::matrix_form *form = &wf->matrix_form().at(i);

//            QTreeWidgetItem *item = new QTreeWidgetItem(analysis);

//            item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::matrix_form *>(form));
//            item->setData(1, Qt::UserRole, QString::fromStdString(wf->analysistype()));
//            item->setData(2, Qt::UserRole, "matrix");
//            item->setData(3, Qt::UserRole, "volume");
//            item->setText(0, tr("Matrix form"));
//            item->setText(1, QString::number(form->i()));
//            item->setText(2, QString::number(form->j()));
//            item->setText(3, QString::fromStdString(form->id()));
//            item->setIcon(0, icon("form-matrix"));
//        }

//        // vector form
//        for (int i = 0; i < wf->vector_form().size(); i++)
//        {
//            XMLModule::vector_form *form = &wf->vector_form().at(i);

//            QTreeWidgetItem *item = new QTreeWidgetItem(analysis);

//            item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::vector_form *>(form));
//            item->setData(1, Qt::UserRole, QString::fromStdString(wf->analysistype()));
//            item->setData(2, Qt::UserRole, "vector");
//            item->setData(3, Qt::UserRole, "volume");
//            item->setText(0, tr("Vector form"));
//            item->setText(1, QString::number(form->i()));
//            item->setText(2, QString::number(form->j()));
//            item->setText(3, QString::fromStdString(form->id()));
//            item->setIcon(0, icon("form-vector"));
//        }
    }

    // surface weakform quantities
    treeSurfaceQuantityGlobal->clear();
    for (int i = 0; i < moduleDialog->module()->surface().quantity().size(); i++)
    {
        XMLModule::quantity *quantity = &moduleDialog->module()->surface().quantity().at(i);

        QTreeWidgetItem *item = new QTreeWidgetItem(treeSurfaceQuantityGlobal);

        item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::quantity *>(quantity));
        item->setText(0, QString::fromStdString(quantity->id()));
        item->setText(1, QString::fromStdString((quantity->shortname().present()) ? quantity->shortname().get() : ""));
    }

    treeSurfaceQuantityAnalysis->clear();
    for (int i = 0; i < moduleDialog->module()->surface().weakforms_surface().weakform_surface().size(); i++)
    {
        XMLModule::weakform_surface *wf = &moduleDialog->module()->surface().weakforms_surface().weakform_surface().at(i);

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
    for (int i = 0; i < moduleDialog->module()->surface().weakforms_surface().weakform_surface().size(); i++)
    {
        XMLModule::weakform_surface *wf = &moduleDialog->module()->surface().weakforms_surface().weakform_surface().at(i);

        QTreeWidgetItem *analysis = new QTreeWidgetItem(treeSurfaceWeakforms);
        analysis->setExpanded(true);
        analysis->setText(0, analysisTypeString(analysisTypeFromStringKey(QString::fromStdString(wf->analysistype()))));

        for (int i = 0; i < wf->boundary().size(); i++)
        {
            XMLModule::boundary *bnd = &wf->boundary().at(i);

            QTreeWidgetItem *boundary = new QTreeWidgetItem(analysis);
            boundary->setExpanded(true);
            boundary->setText(0, QString::fromStdString(bnd->name()));

            assert(0);
//            // matrix form
//            for (int i = 0; i < bnd->matrix_form().size(); i++)
//            {
//                XMLModule::matrix_form *form = &bnd->matrix_form().at(i);

//                QTreeWidgetItem *item = new QTreeWidgetItem(boundary);

//                item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::matrix_form *>(form));
//                item->setData(1, Qt::UserRole, QString::fromStdString(wf->analysistype()));
//                item->setData(2, Qt::UserRole, "matrix");
//                item->setData(3, Qt::UserRole, "surface");
//                item->setText(0, tr("Matrix form"));
//                item->setText(1, QString::number(form->i().get()));
//                item->setText(2, QString::number(form->j().get()));
//                item->setText(3, QString::fromStdString(form->id()));
//                item->setIcon(0, icon("form-matrix"));
//            }

//            // vector form
//            for (int i = 0; i < bnd->vector_form().size(); i++)
//            {
//                XMLModule::vector_form *form = &bnd->vector_form().at(i);

//                QTreeWidgetItem *item = new QTreeWidgetItem(boundary);

//                item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::vector_form *>(form));
//                item->setData(1, Qt::UserRole, QString::fromStdString(wf->analysistype()));
//                item->setData(2, Qt::UserRole, "vector");
//                item->setData(3, Qt::UserRole, "surface");
//                item->setText(0, tr("Vector form"));
//                item->setText(1, QString::number(form->i().get()));
//                item->setText(2, QString::number(form->j().get()));
//                item->setText(3, QString::fromStdString(form->id()));
//                item->setIcon(0, icon("form-vector"));
//            }

//            // essential form
//            for (int i = 0; i < bnd->essential_form().size(); i++)
//            {
//                XMLModule::essential_form *form = &bnd->essential_form().at(i);

//                QTreeWidgetItem *item = new QTreeWidgetItem(boundary);

//                item->setData(0, Qt::UserRole, QVariant::fromValue<XMLModule::essential_form *>(form));
//                item->setData(1, Qt::UserRole, QString::fromStdString(wf->analysistype()));
//                item->setData(2, Qt::UserRole, "essential");
//                item->setData(3, Qt::UserRole, "surface");
//                item->setText(0, tr("Essential form"));
//                item->setText(1, QString::number(form->i()));
//                item->setText(2, "");
//                item->setText(3, QString::fromStdString(form->id()));
//                item->setIcon(0, icon("form-essential"));
//            }
        }
    }

    // materials and boundaries
    treeMaterials->clear();
    treeBoundaries->clear();
    for (int i = 0; i < moduleDialog->module()->preprocessor().gui().size(); i++)
    {
        XMLModule::gui *ui = &moduleDialog->module()->preprocessor().gui().at(i);

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
    for (int i = 0; i < moduleDialog->module()->postprocessor().localvariables().localvariable().size(); i++)
    {
        XMLModule::localvariable *lv = &moduleDialog->module()->postprocessor().localvariables().localvariable().at(i);

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

    XMLModule::scalar_view *sv = &moduleDialog->module()->postprocessor().view().scalar_view();

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

    XMLModule::vector_view *vv = &moduleDialog->module()->postprocessor().view().vector_view();

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
    for (int i = 0; i < moduleDialog->module()->postprocessor().surfaceintegrals().surfaceintegral().size(); i++)
    {
        XMLModule::surfaceintegral *sur = &moduleDialog->module()->postprocessor().surfaceintegrals().surfaceintegral().at(i);

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
    for (int i = 0; i < moduleDialog->module()->postprocessor().volumeintegrals().volumeintegral().size(); i++)
    {
        XMLModule::volumeintegral *vol = &moduleDialog->module()->postprocessor().volumeintegrals().volumeintegral().at(i);

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
        std::ofstream out(compatibleFilename(datadir() + MODULEROOT + "/" + m_fieldId + ".xml").toStdString().c_str());
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
    QPushButton *btnBuildModule = new QPushButton(tr("Build plugin"));
    btnBuildModule->setDefault(false);
    connect(btnBuildModule, SIGNAL(clicked()), this, SLOT(buildPlugin()));

    QPushButton *btnShowXML = new QPushButton(tr("XML file"));
    btnShowXML->setDefault(false);
    connect(btnShowXML, SIGNAL(clicked()), this, SLOT(showXML()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->addButton(btnBuildModule, QDialogButtonBox::ActionRole);
    buttonBox->addButton(btnShowXML, QDialogButtonBox::ActionRole);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutHorizontal);
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
    treeVolumeQuantityGlobal->setColumnWidth(0, 250);
    treeVolumeQuantityGlobal->setColumnWidth(1, 100);
    treeVolumeQuantityGlobal->setIndentation(5);
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
    layoutVolumeQuantity->addWidget(new QLabel(tr("Parameters for particular analysis:")), 0, 1);
    layoutVolumeQuantity->addWidget(treeVolumeQuantityGlobal, 1, 0);
    layoutVolumeQuantity->addWidget(treeVolumeQuantityAnalysis, 1, 1);

    QGroupBox *grpVolumeQuantities = new QGroupBox(tr("Quantities"));
    grpVolumeQuantities->setLayout(layoutVolumeQuantity);

    treeVolumeWeakforms = new QTreeWidget(this);
    treeVolumeWeakforms->setMouseTracking(true);
    treeVolumeWeakforms->setColumnCount(3);
    treeVolumeWeakforms->setColumnWidth(0, 200);
    treeVolumeWeakforms->setColumnWidth(1, 80);
    treeVolumeWeakforms->setColumnWidth(2, 80);
    treeVolumeWeakforms->setIndentation(12);
    QStringList headVolumeWeakforms;
    headVolumeWeakforms << tr("Type") << tr("Solution") << tr("Test function") << tr("Label");
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
    treeSurfaceQuantityGlobal->setColumnWidth(0, 250);
    treeSurfaceQuantityGlobal->setColumnWidth(1, 100);
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
    layoutSurfaceQuantity->addWidget(new QLabel(tr("Parameters for partial analysis:")), 0, 1);
    layoutSurfaceQuantity->addWidget(treeSurfaceQuantityGlobal, 1, 0);
    layoutSurfaceQuantity->addWidget(treeSurfaceQuantityAnalysis, 1, 1);

    QGroupBox *grpSurfaceQuantities = new QGroupBox(tr("Quantities"));
    grpSurfaceQuantities->setLayout(layoutSurfaceQuantity);

    treeSurfaceWeakforms = new QTreeWidget(this);
    treeSurfaceWeakforms->setMouseTracking(true);
    treeSurfaceWeakforms->setColumnCount(3);
    treeSurfaceWeakforms->setColumnWidth(0, 200);
    treeSurfaceWeakforms->setColumnWidth(1, 80);
    treeSurfaceWeakforms->setColumnWidth(2, 80);
    treeSurfaceWeakforms->setIndentation(12);
    QStringList headSurfaceWeakforms;
    headSurfaceWeakforms << tr("Type") << tr("Solution") << tr("Test function") << tr("Label");
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

    QWidget *surfaceIntegrals = new QWidget(this);
    surfaceIntegrals->setLayout(layoutSurface);

    QTabWidget *postprocessorWidget = new QTabWidget(this);
    postprocessorWidget->addTab(localVariables, icon(""), tr("Local variables"));
    postprocessorWidget->addTab(volumeIntegrals, icon(""), tr("Volume integrals"));
    postprocessorWidget->addTab(surfaceIntegrals, icon(""), tr("Surface integrals"));

    return postprocessorWidget;
}

int ModuleDialog::numberOfSolutions(std::string analysisType)
{
    for (int i = 0; i < module()->general().analyses().analysis().size(); i++)
    {
        XMLModule::analysis analysis = module()->general().analyses().analysis().at(i);
        if (analysis.id() == analysisType)
            return analysis.solutions();
    }

    return -1;
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

void ModuleDialog::buildPlugin()
{
    save();

    SystemOutputWidget *output = new SystemOutputWidget();
    output->execute(QString(COMMANDS_BUILD_PLUGIN).arg(QString::fromStdString(module()->general().id())));
}

void ModuleDialog::showXML()
{
    save();

    QDesktopServices::openUrl(QUrl(QString("%1%2/%3.xml").arg(datadir()).arg(MODULEROOT).arg(m_fieldId)));
}

void ModuleDialog::constantDoubleClicked(QTreeWidgetItem *item, int role)
{
    XMLModule::constant *constant = item->data(0, Qt::UserRole).value<XMLModule::constant *>();
    if (constant)
    {
        ModuleItemConstantDialog dialog(this, constant, this);
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
        ModuleItemAnalysisDialog dialog(this, analysis, this);
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
        ModuleItemQuantityGlobalDialog dialog(this, quantity, this);
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
        ModuleItemQuantityAnalysisDialog dialog(this, quantity, this);
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
    if (item->data(2, Qt::UserRole).toString() == "matrix")
    {
        XMLModule::matrix_form *form = item->data(0, Qt::UserRole).value<XMLModule::matrix_form *>();
        if (form)
        {
            ModuleItemMatrixFormDialog dialog(this,
                                              item->data(3, Qt::UserRole).toString(),
                                              numberOfSolutions(item->data(1, Qt::UserRole).toString().toStdString()),
                                              form, this);
            if (dialog.exec())
            {
                item->setText(1, QString::number(form->i().get()));
                item->setText(2, QString::number(form->j().get()));
            }
        }
    }
    else if (item->data(2, Qt::UserRole).toString() == "vector")
    {
        XMLModule::vector_form *form = item->data(0, Qt::UserRole).value<XMLModule::vector_form *>();
        if (form)
        {
            ModuleItemVectorFormDialog dialog(this,
                                              item->data(3, Qt::UserRole).toString(),
                                              numberOfSolutions(item->data(1, Qt::UserRole).toString().toStdString()),
                                              form, this);
            if (dialog.exec())
            {
                item->setText(1, QString::number(form->i().get()));
                item->setText(2, QString::number(form->j().get()));
            }
        }
    }
    else if (item->data(2, Qt::UserRole).toString() == "essential")
    {
        XMLModule::essential_form *form = item->data(0, Qt::UserRole).value<XMLModule::essential_form *>();
        if (form)
        {
            ModuleItemEssentialFormDialog dialog(this,
                                                 numberOfSolutions(item->data(1, Qt::UserRole).toString().toStdString()),
                                                 form, this);
            if (dialog.exec())
            {
                item->setText(1, QString::number(form->i().get()));
            }
        }
    }
}

void ModuleDialog::preprocessorDoubleClicked(QTreeWidgetItem *item, int role)
{
    XMLModule::quantity *quant = item->data(0, Qt::UserRole).value<XMLModule::quantity *>();

    if (quant)
    {
        ModulePreprocessorDialog dialog(this, quant, this);
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
        ModuleItemLocalValueDialog dialog(this, lv, this);
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
        ModuleDialog *moduleDialog = this;
        XMLModule::localvariables *lv = &module()->postprocessor().localvariables();

        ModuleItemViewDefaultsDialog dialog(this, def, lv, this);
        if (dialog.exec())
            item->setText(1, QString::fromStdString(def->id()));
    }
}

void ModuleDialog::surfaceIntegralDoubleClicked(QTreeWidgetItem *item, int role)
{
    XMLModule::surfaceintegral *sur = item->data(0, Qt::UserRole).value<XMLModule::surfaceintegral *>();
    if (sur)
    {
        ModuleSurfaceIntegralValueDialog dialog(this, sur, this);
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
        ModuleVolumeIntegralValueDialog dialog(this, vol, this);
        if (dialog.exec())
        {
            item->setText(0, QString::fromStdString(vol->name()));
            item->setText(1, QString::fromStdString(vol->shortname()));
            item->setText(2, QString::fromStdString(vol->unit()));
        }
    }
}
