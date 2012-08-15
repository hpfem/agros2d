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
#include "../../resources_source/classes/module_xml.h"

struct SceneViewSettings;
class ScriptEditor;

class LineEditDouble;
class ValueLineEdit;
class HtmlValueEdit;

class ModuleItem : public QWidget
{
    Q_OBJECT

public:
    ModuleItem(QWidget *parent);

    QLineEdit *txtPlanar;
    QLineEdit *txtPlanarX;
    QLineEdit *txtPlanarY;
    QLineEdit *txtPlanarZ;
    QLineEdit *txtAxi;
    QLineEdit *txtAxiR;
    QLineEdit *txtAxiZ;
    QLineEdit *txtAxiPhi;

    virtual void save() = 0;
};

class ModuleItemLocalValue : public ModuleItem
{
    Q_OBJECT

public:
    ModuleItemLocalValue(const QString &type, XMLModule::expression *expr, QWidget *parent);

    void save();

private:
    QString m_type;
    XMLModule::expression *m_expr;
};

class ModuleItemEmptyDialog : public QDialog
{
    Q_OBJECT

public:
    ModuleItemEmptyDialog(QWidget *parent);

protected slots:
    virtual void doAccept() = 0;

protected:
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

class ModuleItemConstantDialog : public ModuleItemEmptyDialog
{
    Q_OBJECT
public:
    ModuleItemConstantDialog(XMLModule::constant *constant, QWidget *parent);

protected slots:
    void doAccept();

private:
    XMLModule::constant *m_constant;

protected:
    QLineEdit *txtID;
    QLineEdit *txtValue;
};

class ModuleItemAnalysisDialog : public ModuleItemEmptyDialog
{
    Q_OBJECT
public:
    ModuleItemAnalysisDialog(XMLModule::analysis *analysis, QWidget *parent);

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
    ModuleItemQuantityGlobalDialog(XMLModule::quantity *quantity, QWidget *parent);

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
    ModuleItemQuantityAnalysisDialog(XMLModule::quantity *quantity, QWidget *parent);

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
    ModuleItemWeakformDialog(QWidget *parent);

protected:
    QLineEdit *txtI;
    QLineEdit *txtJ;
    QTextEdit *txtPlanarLinear;
    QTextEdit *txtPlanarNewton;
    QTextEdit *txtAxiLinear;
    QTextEdit *txtAxiNewton;
};

class ModuleItemMatrixFormDialog : public ModuleItemWeakformDialog
{
    Q_OBJECT
public:
    ModuleItemMatrixFormDialog(XMLModule::matrix_form *form, QWidget *parent);

protected slots:
    void doAccept();

private:
    XMLModule::matrix_form *m_form;
};

class ModuleItemVectorFormDialog : public ModuleItemWeakformDialog
{
    Q_OBJECT
public:
    ModuleItemVectorFormDialog(XMLModule::vector_form *form, QWidget *parent);

protected slots:
    void doAccept();

private:
    XMLModule::vector_form *m_form;
};

class ModuleItemEssentialFormDialog : public ModuleItemWeakformDialog
{
    Q_OBJECT
public:
    ModuleItemEssentialFormDialog(XMLModule::essential_form *form, QWidget *parent);

protected slots:
    void doAccept();

private:
    XMLModule::essential_form *m_form;
};

class ModulePreprocessorDialog : public ModuleItemDialog
{
    Q_OBJECT
public:
    ModulePreprocessorDialog(XMLModule::quantity *quant, QWidget *parent);

protected slots:
    void doAccept();

private:
    XMLModule::quantity *m_quant;
};

class ModuleItemLocalValueDialog : public ModuleItemDialog
{
    Q_OBJECT
public:
    ModuleItemLocalValueDialog(XMLModule::localvariable *lv, QWidget *parent);

protected slots:
    void doAccept();

private:
    XMLModule::localvariable *m_lv;
};

class ModuleItemViewDefaultsDialog : public ModuleItemEmptyDialog
{
    Q_OBJECT
public:
    ModuleItemViewDefaultsDialog(XMLModule::default_ *def, XMLModule::localvariables *lv, QWidget *parent);

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
    ModuleVolumeIntegralValueDialog(XMLModule::volumeintegral *vol, QWidget *parent);

protected slots:
    void doAccept();

private:
    XMLModule::volumeintegral *m_vol;
};

class ModuleSurfaceIntegralValueDialog : public ModuleItemDialog
{
    Q_OBJECT
public:
    ModuleSurfaceIntegralValueDialog(XMLModule::surfaceintegral *sur, QWidget *parent);

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

private slots:
    void doCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void doAccept();
    void doReject();

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
