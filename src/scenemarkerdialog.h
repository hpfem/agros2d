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

#ifndef SCENEMARKERDIALOG_H
#define SCENEMARKERDIALOG_H

#include "util.h"
#include "hermes2d/marker.h"

class LineEditDouble;
class ValueLineEdit;
class LaTeXViewer;

class SceneBoundary;
class SceneMaterial;

Q_DECLARE_METATYPE(SceneBoundary *)
Q_DECLARE_METATYPE(SceneMaterial *)

namespace Module
{
    struct DialogUI;
    struct DialogRow;
}

class SceneBoundary : public Boundary
{
public:
    SceneBoundary(FieldInfo *fieldInfo, QString m_name = "", QString m_type = "",
                  QHash<QString, Value> m_values = (QHash<QString, Value>()));

    int showDialog(QWidget *parent);

    QVariant variant();
};

class SceneBoundaryNone : public SceneBoundary
{
public:
    SceneBoundaryNone();

    QString script() { return ""; }
    int showDialog(QWidget *parent) { return 0; }
};

template <class T>
void deformShapeTemplate(T linVert, int count);

// ************************************************************************************************

class SceneFieldWidget : public QWidget
{
    Q_OBJECT
public:
    SceneFieldWidget(Module::DialogUI *ui, QWidget *parent);

    Module::DialogUI *ui;

    // layout
    QVBoxLayout *layout;

    // equation
    LaTeXViewer *equationLaTeX;

    // quantities
    QList<QString> ids;
    QList<QLabel *> labels;
    QList<ValueLineEdit *> values;
    QList<QString> conditions;

    void createContent();

    virtual void addCustomWidget(QVBoxLayout *layout) = 0;
    virtual ValueLineEdit *addValueEditWidget(const Module::DialogRow &row) = 0;
    virtual void refresh() = 0;
    virtual void load() = 0;
    virtual bool save() = 0;
};

class SceneFieldWidgetMaterial : public SceneFieldWidget
{
    Q_OBJECT
public:
    SceneMaterial *material;

    SceneFieldWidgetMaterial(Module::DialogUI *ui, SceneMaterial *material, QWidget *parent);

    void addCustomWidget(QVBoxLayout *layout) {}
    ValueLineEdit *addValueEditWidget(const Module::DialogRow &row);
    void refresh();
    void load();
    bool save();
};

class SceneFieldWidgetBoundary : public SceneFieldWidget
{
    Q_OBJECT
public:
    SceneBoundary *boundary;

    SceneFieldWidgetBoundary(Module::DialogUI *ui, SceneBoundary *boundary, QWidget *parent);

    QComboBox *comboBox;

    void addCustomWidget(QVBoxLayout *layout);
    ValueLineEdit *addValueEditWidget(const Module::DialogRow &row);
    void refresh();
    void load();
    bool save();

private slots:
    void doTypeChanged(int index);
};

// *************************************************************************************************************************************

class SceneMaterial : public Material
{
public:
    SceneMaterial(FieldInfo *fieldInfo, QString m_name,
                  QHash<QString, Value> m_values = (QHash<QString, Value>()));

    int showDialog(QWidget *parent);

    QVariant variant();
};

class SceneMaterialNone : public SceneMaterial
{
public:
    SceneMaterialNone();

    QString script() { return ""; }
    QMap<QString, QString> data() { return QMap<QString, QString>(); }
    int showDialog(QWidget *parent) { return 0; }
};

// *************************************************************************************************************************************

class SceneBoundaryDialog: public QDialog
{
    Q_OBJECT

public:
    SceneBoundaryDialog(SceneBoundary *boundary, QWidget *parent);

protected:
    QGridLayout *layout;
    QDialogButtonBox *buttonBox;

    QLineEdit *txtName;
    SceneBoundary *boundary;

    SceneFieldWidget *fieldWidget;

    void createContent();
    void createDialog();

    void load();
    bool save();

protected slots:
    void evaluated(bool isError);

private slots:
    void doAccept();
    void doReject();
};

class SceneMaterialDialog: public QDialog
{
    Q_OBJECT

public:
    SceneMaterialDialog(SceneMaterial *material, QWidget *parent);

protected:
    QGridLayout *layout;
    QDialogButtonBox *buttonBox;

    QLineEdit *txtName;
    SceneMaterial *material;

    SceneFieldWidget *fieldWidget;

    void createContent();
    void createDialog();

    void load();
    bool save();

protected slots:
    void evaluated(bool isError);

private slots:
    void doAccept();
    void doReject();
};

#endif // SCENEMARKERDIALOG_H
