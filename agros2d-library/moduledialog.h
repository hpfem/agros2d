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

#ifndef WEAKFORMDIALOG_H
#define WEAKFORMDIALOG_H

#include "util.h"
#include "util/enums.h"

#include "gui/textedit.h"

#include "../../resources_source/classes/module_xml.h"

struct SceneViewSettings;
class ScriptEditor;

class LineEditDouble;
class ValueLineEdit;
class HtmlValueEdit;

class ModuleDialog;

class ModuleHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    ModuleHighlighter(QTextDocument *textDocument);

    void setKeywords(QStringList patterns, const QColor &color);

protected:
    void highlightBlock(const QString &text);
    void highlightBlockParenthesis(const QString &text, char left, char right);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keywordFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat operatorFormat;
    QTextCharFormat numberFormat;
};


class ModuleDialogTextEdit : public PlainTextEditParenthesis
{
  Q_OBJECT

public:
    ModuleDialogTextEdit(QWidget *parent = NULL, int rows = 1);

    void addToolTip(const QString &label, QStringList patterns);

    void setCoordinates(CoordinateType coordinateType);

    void setWeakformFuntions(CoordinateType coordinateType);
    void setWeakformVolumeHighlighter(XMLModule::volume::quantity_sequence sequence, int numberOfSolutions, CoordinateType coordinateType);
    void setWeakformSurfaceHighlighter(XMLModule::surface::quantity_sequence sequence, int numberOfSolutions, CoordinateType coordinateType);

    void setPostprocessorFuntions(int numberOfSolutions, CoordinateType coordinateType);
    void setPostprocessorVolumeHighlighter(XMLModule::volume::quantity_sequence sequence, int numberOfSolutions, CoordinateType coordinateType);
    void setPostprocessorSurfaceHighlighter(XMLModule::surface::quantity_sequence sequence, int numberOfSolutions, CoordinateType coordinateType);

    QSize sizeHint() const;
    inline void setText(const QString &text) { this->setPlainText(text); }
    inline QString text() const { return this->toPlainText(); }

private:
    int m_rows;
    ModuleHighlighter *m_highlighter;
};

class ModuleItem : public QWidget
{
    Q_OBJECT

public:
    ModuleItem(QWidget *parent);

    ModuleDialogTextEdit *txtPlanar;
    ModuleDialogTextEdit *txtPlanarX;
    ModuleDialogTextEdit *txtPlanarY;
    ModuleDialogTextEdit *txtPlanarZ;
    ModuleDialogTextEdit *txtAxi;
    ModuleDialogTextEdit *txtAxiR;
    ModuleDialogTextEdit *txtAxiZ;
    ModuleDialogTextEdit *txtAxiPhi;

    virtual void save() = 0;
};

class ModuleItemLocalValue : public ModuleItem
{
    Q_OBJECT

public:
    ModuleItemLocalValue(ModuleDialog *moduleDialog, const QString &type,
                         XMLModule::expression *expr, QWidget *parent);

    void save();

private:
    QString m_type;
    XMLModule::expression *m_expr;
};

/*
class ModuleWeakform : public QWidget
{
    Q_OBJECT

public:
    ModuleWeakform(WeakFormKind weakForm, QWidget *parent);

    void save();

protected:
    QSpinBox *txtI;
    QSpinBox *txtJ;
    QLineEdit *txtPlanarLinear;
    QLineEdit *txtAxiLinear;
    QLineEdit *txtPlanarNewton;
    QLineEdit *txtAxiNewton;
};
*/

class ModuleItemEmptyDialog : public QDialog
{
    Q_OBJECT

public:
    ModuleItemEmptyDialog(ModuleDialog *moduleDialog, QWidget *parent);

protected slots:
    virtual void doAccept() = 0;

protected:
    ModuleDialog *m_module;

    QList<ModuleItem *> items;

    QVBoxLayout *layoutMain;
    QDialogButtonBox *buttonBox;
};

class ModuleItemDialog : public QDialog
{
    Q_OBJECT

public:
    ModuleItemDialog(QWidget *parent);

protected slots:
    virtual void doAccept() = 0;

protected:
    QLineEdit *txtId;
    QLineEdit *txtName;
    QComboBox *txtType;
    QLineEdit *txtShortname;
    HtmlValueEdit *txtShortnameHtml;
    QLineEdit *txtShortnameLatex;
    QLineEdit *txtUnit;
    HtmlValueEdit *txtUnitHtml;
    QLineEdit *txtUnitLatex;

    QList<ModuleItem *> items;

    QVBoxLayout *layoutMain;
    QDialogButtonBox *buttonBox;
};

class ModuleItemConstantDialog : public ModuleItemEmptyDialog
{
    Q_OBJECT
public:
    ModuleItemConstantDialog(ModuleDialog *moduleDialog, XMLModule::constant *constant, QWidget *parent);
    ~ModuleItemConstantDialog();

protected slots:
    void doAccept();

private:
    XMLModule::constant *m_constant;

protected:
    QLineEdit *txtID;
    LineEditDouble *txtValue;
};

class ModuleItemAnalysisDialog : public ModuleItemEmptyDialog
{
    Q_OBJECT
public:
    ModuleItemAnalysisDialog(ModuleDialog *moduleDialog, XMLModule::analysis *analysis, QWidget *parent);

protected slots:
    void doAccept();

private:
    XMLModule::analysis *m_analysis;

protected:
    QLineEdit *txtID;
    QLineEdit *txtName;
    QComboBox *cmbType;
    QLineEdit *txtSolutions;
};

class ModuleItemQuantityGlobalDialog : public ModuleItemEmptyDialog
{
    Q_OBJECT
public:
    ModuleItemQuantityGlobalDialog(ModuleDialog *moduleDialog, XMLModule::quantity *quantity, QWidget *parent);

protected slots:
    void doAccept();

private:
    XMLModule::quantity *m_quantity;

protected:
    QLineEdit *txtID;
    QLineEdit *txtShortname;
};

class ModuleItemQuantityAnalysisDialog : public ModuleItemEmptyDialog
{
    Q_OBJECT
public:
    ModuleItemQuantityAnalysisDialog(ModuleDialog *moduleDialog, XMLModule::quantity *quantity, QWidget *parent);

protected slots:
    void doAccept();

private:
    XMLModule::quantity *m_quantity;

protected:
    QLineEdit *txtPlanarNonlinearity;
    QLineEdit *txtAxiNonlinearity;
    QLineEdit *txtDependence;
};

class ModuleItemWeakformDialog : public ModuleItemEmptyDialog
{
    Q_OBJECT
public:
    ModuleItemWeakformDialog(ModuleDialog *moduleDialog, QWidget *parent);

protected:
    QLineEdit *txtID;
    QSpinBox *txtSolutionIndex;
    QSpinBox *txtTestFunctionIndex;
    ModuleDialogTextEdit *txtPlanarLinear;
    ModuleDialogTextEdit *txtPlanarNewton;
    ModuleDialogTextEdit *txtAxiLinear;
    ModuleDialogTextEdit *txtAxiNewton;
};

class ModuleItemMatrixFormDialog : public ModuleItemWeakformDialog
{
    Q_OBJECT
public:
    ModuleItemMatrixFormDialog(ModuleDialog *moduleDialog, QString type, int numberOfSolutions,
                               XMLModule::matrix_form *form, QWidget *parent);
    ~ModuleItemMatrixFormDialog();

protected slots:
    void doAccept();

private:
    XMLModule::matrix_form *m_form;
};

class ModuleItemVectorFormDialog : public ModuleItemWeakformDialog
{
    Q_OBJECT
public:
    ModuleItemVectorFormDialog(ModuleDialog *moduleDialog, QString type, int numberOfSolutions,
                               XMLModule::vector_form *form, QWidget *parent);
    ~ModuleItemVectorFormDialog();

protected slots:
    void doAccept();

private:
    XMLModule::vector_form *m_form;
};

class ModuleItemEssentialFormDialog : public ModuleItemWeakformDialog
{
    Q_OBJECT
public:
    ModuleItemEssentialFormDialog(ModuleDialog *moduleDialog, int numberOfSolutions, XMLModule::essential_form *form, QWidget *parent);
    ~ModuleItemEssentialFormDialog();

protected slots:
    void doAccept();

private:
    XMLModule::essential_form *m_form;
};

class ModulePreprocessorDialog : public ModuleItemDialog
{
    Q_OBJECT
public:
    ModulePreprocessorDialog(ModuleDialog *moduleDialog, XMLModule::quantity *quant, QWidget *parent);
    ~ModulePreprocessorDialog();

protected slots:
    void doAccept();

private:
    XMLModule::quantity *m_quant;

    QLineEdit *txtCondition;
    QLineEdit *txtDefault;
};

class ModuleItemLocalValueDialog : public ModuleItemDialog
{
    Q_OBJECT
public:
    ModuleItemLocalValueDialog(ModuleDialog *moduleDialog, XMLModule::localvariable *lv, QWidget *parent);
    ~ModuleItemLocalValueDialog();

protected slots:
    void doAccept();

private:
    XMLModule::localvariable *m_lv;
};

class ModuleItemViewDefaultsDialog : public ModuleItemEmptyDialog
{
    Q_OBJECT
public:
    ModuleItemViewDefaultsDialog(ModuleDialog *moduleDialog,
                                 XMLModule::default_ *def, XMLModule::localvariables *lv,
                                 QWidget *parent);
    ~ModuleItemViewDefaultsDialog();

protected slots:
    void doAccept();

private:
    XMLModule::default_ *m_def;
    XMLModule::localvariables *m_lv;

protected:
    QComboBox *cmbLocalVariable;
};

class ModuleVolumeIntegralValueDialog : public ModuleItemDialog
{
    Q_OBJECT
public:
    ModuleVolumeIntegralValueDialog(ModuleDialog *moduleDialog, XMLModule::volumeintegral *vol, QWidget *parent);
    ~ModuleVolumeIntegralValueDialog();

protected slots:
    void doAccept();

private:
    XMLModule::volumeintegral *m_vol;
};

class ModuleSurfaceIntegralValueDialog : public ModuleItemDialog
{
    Q_OBJECT
public:
    ModuleSurfaceIntegralValueDialog(ModuleDialog *moduleDialog, XMLModule::surfaceintegral *sur, QWidget *parent);
    ~ModuleSurfaceIntegralValueDialog();

protected:
    void addExpressions(QLayout *layout) {}

protected slots:
    void doAccept();

private:
    XMLModule::surfaceintegral *m_sur;
};

class ModuleDialog : public QDialog
{
    Q_OBJECT
public:
    ModuleDialog(const QString &fieldId, QWidget *parent);
    ~ModuleDialog();

    inline XMLModule::module *module() { return m_module_xsd.get(); }
    int numberOfSolutions(std::string analysisType);

private slots:
    void doCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void doAccept();
    void doReject();
    void buildPlugin();
    void showXML();

    void constantDoubleClicked(QTreeWidgetItem *item, int role);
    void doAddConstant();

    void analysisDoubleClicked(QTreeWidgetItem *item, int role);

    void quantityGlobalDoubleClicked(QTreeWidgetItem *item, int role);
    void quantityAnalysisDoubleClicked(QTreeWidgetItem *item, int role);
    void weakformDoubleClicked(QTreeWidgetItem *item, int role);

    void preprocessorDoubleClicked(QTreeWidgetItem *item, int role);

    void localItemDoubleClicked(QTreeWidgetItem *item, int role);
    void viewDefaultsItemDoubleClicked(QTreeWidgetItem *item, int role);
    void volumeIntegralDoubleClicked(QTreeWidgetItem *item, int role);
    void surfaceIntegralDoubleClicked(QTreeWidgetItem *item, int role);

private:
    QString m_fieldId;
    std::auto_ptr<XMLModule::module> m_module_xsd;

    QListWidget *lstView;
    QStackedWidget *pages;
    QWidget *panMain;
    QWidget *panWeakforms;
    QWidget *panPreprocessor;
    QWidget *panPostprocessor;

    void load();
    void save();

    void createControls();
    QWidget *createMainWidget();
    QWidget *createWeakforms();
    QWidget *createPreprocessor();
    QWidget *createPostprocessor();

    // constant
    QTreeWidget *treeConstants;
    QTreeWidget *treeAnalyses;

    // weakforms
    QTreeWidget *treeVolumeQuantityGlobal;
    QTreeWidget *treeVolumeQuantityAnalysis;
    QTreeWidget *treeSurfaceQuantityGlobal;
    QTreeWidget *treeSurfaceQuantityAnalysis;

    QTreeWidget *treeVolumeWeakforms;
    QTreeWidget *treeSurfaceWeakforms;

    // preprocessor
    QTreeWidget *treeMaterials;
    QTreeWidget *treeBoundaries;

    // postprocessor
    QTreeWidget *treeLocalVariables;
    QTreeWidget *treeViewDefaults;
    QTreeWidget *treeVolumeIntegrals;
    QTreeWidget *treeSurfaceIntegrals;

    // main
    QLineEdit *txtId;
    QLineEdit *txtName;
    QPlainTextEdit *txtDescription;
};


#endif // WEAKFORMDIALOG_H
