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

class SLineEditDouble;
class ValueLineEdit;

struct ModuleItem
{
    QLineEdit *txtPlanar;
    QLineEdit *txtPlanarX;
    QLineEdit *txtPlanarY;
    QLineEdit *txtPlanarZ;
    QLineEdit *txtAxi;
    QLineEdit *txtAxiR;
    QLineEdit *txtAxiZ;
    QLineEdit *txtAxiPhi;
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
    QLineEdit *txtShortnameHtml;
    QLineEdit *txtShortnameLatex;
    QLineEdit *txtUnit;
    QLineEdit *txtUnitHtml;
    QLineEdit *txtUnitLatex;

    QList<ModuleItem> items;

    void load();
    void save();
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

    void localItemDoubleClicked(QTreeWidgetItem *item, int role);
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

    // postprocessor
    QTreeWidget *treeLocalVariables;
    QTreeWidget *treeVolumeIntegrals;
    QTreeWidget *treeSurfaceIntegrals;

    // main
    QLineEdit *txtId;
    QLineEdit *txtName;
    QLineEdit *txtName2;
    QPlainTextEdit *txtDescription;
};


#endif // WEAKFORMDIALOG_H
