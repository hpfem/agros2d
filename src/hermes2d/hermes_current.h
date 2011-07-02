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

#ifndef HERMES_CURRENT_H
#define HERMES_CURRENT_H

#include "util.h"
#include "hermes_field.h"


struct ModuleCurrent : public Hermes::Module::ModuleAgros
{
    Q_OBJECT
public:
    ModuleCurrent(ProblemType problemType, AnalysisType analysisType) : Hermes::Module::ModuleAgros(problemType, analysisType) {}

    inline int number_of_solution() const { return 1; }
    bool has_nonlinearity() const { return false; }

    LocalPointValue *local_point_value(const Point &point);
    SurfaceIntegralValue *surface_integral_value();
    VolumeIntegralValue *volume_integral_value();

    ViewScalarFilter *view_scalar_filter(Hermes::Module::LocalVariable *physicFieldVariable,
                                         PhysicFieldVariableComp physicFieldVariableComp);

    Hermes::vector<SolutionArray *> solve(ProgressItemSolve *progressItemSolve);

    // rewrite
    void readBoundaryFromDomElement(QDomElement *element);
    void writeBoundaryToDomElement(QDomElement *element, SceneBoundary *marker);
    void readMaterialFromDomElement(QDomElement *element);
    void writeMaterialToDomElement(QDomElement *element, SceneMaterial *marker);

    SceneBoundary *newBoundary();
    SceneBoundary *newBoundary(PyObject *self, PyObject *args);
    SceneBoundary *modifyBoundary(PyObject *self, PyObject *args);
    SceneMaterial *newMaterial();
    SceneMaterial *newMaterial(PyObject *self, PyObject *args);
    SceneMaterial *modifyMaterial(PyObject *self, PyObject *args);
};

// *******************************************************************************************

class ParserCurrent : public Parser
{
public:
    double pgamma;

    void setParserVariables(SceneMaterial *material);
};

class LocalPointValueCurrent : public LocalPointValue
{
public:
    LocalPointValueCurrent(const Point &point);
};

class SurfaceIntegralValueCurrent : public SurfaceIntegralValue
{
public:
    SurfaceIntegralValueCurrent();
};

class VolumeIntegralValueCurrent : public VolumeIntegralValue
{
public:
    VolumeIntegralValueCurrent();
};

class ViewScalarFilterCurrent : public ViewScalarFilter
{
public:
    ViewScalarFilterCurrent(Hermes::vector<MeshFunction *> sln,
                            std::string expression);
};

// *******************************************************************************************

class SceneBoundaryCurrent : public SceneBoundary
{
public:
    Value value;

    SceneBoundaryCurrent(const QString &name, PhysicFieldBC type, Value value);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneMaterialCurrent : public SceneMaterial
{
public:
    Value conductivity;

    SceneMaterialCurrent(const QString &name, Value conductivity);

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class SceneBoundaryCurrentDialog : public SceneBoundaryDialog
{
    Q_OBJECT

public:
    SceneBoundaryCurrentDialog(SceneBoundaryCurrent *boundary, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    QLabel *lblValueUnit;
    QComboBox *cmbType;
    ValueLineEdit *txtValue;

private slots:
    void doTypeChanged(int index);
};

class SceneMaterialCurrentDialog : public SceneMaterialDialog
{
    Q_OBJECT

public:
    SceneMaterialCurrentDialog(SceneMaterialCurrent *material, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    ValueLineEdit *txtConductivity;
};

#endif // HERMES_CURRENT_H
