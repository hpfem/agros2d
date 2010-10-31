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

#ifndef HERMES_FIELD_H
#define HERMES_FIELD_H

#include <QDomDocument>

#include "util.h"
#include "hermes2d.h"
#include "solver_umfpack.h"

#include "localvalueview.h"
#include "surfaceintegralview.h"
#include "volumeintegralview.h"

#include "scenemarker.h"
#include "progressdialog.h"

extern bool isPlanar;
extern AnalysisType analysisType;
extern double frequency;
extern double actualTime;
extern double timeStep;

class LocalPointValue;
class VolumeIntegralValue;
class SurfaceIntegralValue;

class SceneEdgeMarker;
class SceneLabelMarker;
class SceneViewSettings;

class SolutionArray;

class ViewScalarFilter;

struct HermesField : public QObject
{
    Q_OBJECT
public:
    PhysicField physicField;
    HermesField() { physicField = PhysicField_Undefined; }

    virtual int numberOfSolution() = 0;
    virtual bool hasHarmonic() = 0;
    virtual bool hasTransient() = 0;

    virtual void readEdgeMarkerFromDomElement(QDomElement *element) = 0;
    virtual void writeEdgeMarkerToDomElement(QDomElement *element, SceneEdgeMarker *marker) = 0;
    virtual void readLabelMarkerFromDomElement(QDomElement *element) = 0;
    virtual void writeLabelMarkerToDomElement(QDomElement *element, SceneLabelMarker *marker) = 0;

    virtual LocalPointValue *localPointValue(Point point) = 0;
    virtual QStringList localPointValueHeader() = 0;

    virtual SurfaceIntegralValue *surfaceIntegralValue() = 0;
    virtual QStringList surfaceIntegralValueHeader() = 0;

    virtual VolumeIntegralValue *volumeIntegralValue() = 0;
    virtual QStringList volumeIntegralValueHeader() = 0;

    virtual bool physicFieldBCCheck(PhysicFieldBC physicFieldBC) = 0;
    virtual bool physicFieldVariableCheck(PhysicFieldVariable physicFieldVariable) = 0;

    virtual SceneEdgeMarker *newEdgeMarker() = 0;
    virtual SceneEdgeMarker *newEdgeMarker(PyObject *self, PyObject *args) = 0;
    virtual SceneEdgeMarker *modifyEdgeMarker(PyObject *self, PyObject *args) = 0;
    virtual SceneLabelMarker *newLabelMarker() = 0;
    virtual SceneLabelMarker *newLabelMarker(PyObject *self, PyObject *args) = 0;
    virtual SceneLabelMarker *modifyLabelMarker(PyObject *self, PyObject *args) = 0;

    virtual QList<SolutionArray *> *solve(ProgressItemSolve *progressItemSolve) = 0;

    virtual PhysicFieldVariable contourPhysicFieldVariable() = 0;
    virtual PhysicFieldVariable scalarPhysicFieldVariable() = 0;
    virtual PhysicFieldVariableComp scalarPhysicFieldVariableComp() = 0;
    virtual PhysicFieldVariable vectorPhysicFieldVariable() = 0;

    virtual void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable) = 0;
    virtual void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable) = 0;

    virtual void showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue) = 0;
    virtual void showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue) = 0;
    virtual void showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue) = 0;

    virtual ViewScalarFilter *viewScalarFilter(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp) = 0;
};

HermesField *hermesFieldFactory(PhysicField physicField);

class ViewScalarFilter : public Filter
{
public:
    ViewScalarFilter(MeshFunction *sln1, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
    ViewScalarFilter(MeshFunction *sln1, MeshFunction *sln2, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
    ViewScalarFilter(MeshFunction *sln1, MeshFunction *sln2, MeshFunction *sln3, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);

    double get_pt_value(double x, double y, int item = H2D_FN_VAL);

protected:
    PhysicFieldVariable m_physicFieldVariable;
    PhysicFieldVariableComp m_physicFieldVariableComp;

    Node* node;

    double *dudx1, *dudy1, *dudx2, *dudy2, *dudx3, *dudy3;
    double *value1, *value2, *value3;
    double *x, *y;

    SceneLabelMarker *labelMarker;

    void precalculate(int order, int mask);
    virtual void calculateVariable(int i) = 0;
};

// read mesh
Mesh *readMeshFromFile(const QString &fileName);
void writeMeshFromFile(const QString &fileName, Mesh *mesh);

// solve
QList<SolutionArray *> *solveSolutioArray(ProgressItemSolve *progressItemSolve,
                                          void (*cbSpace)(Tuple<Space *>),
                                          void (*cbWeakForm)(WeakForm *, Tuple<Solution *>));

// custom forms **************************************************************************************************************************

template<typename Real, typename Scalar>
Scalar int_x_u_v(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e)
{
  Scalar result = 0;
  for (int i = 0; i < n; i++)
    result += wt[i] * e->x[i] * (u->val[i] * v->val[i]);
  return result;
}

template<typename Real, typename Scalar>
Scalar int_x_v(int n, double *wt, Func<Real> *v, Geom<Real> *e)
{
    Scalar result = 0;
    for (int i = 0; i < n; i++)
        result += wt[i] * e->x[i] * (v->val[i]);
    return result;
}

template<typename Real, typename Scalar>
Scalar int_x_grad_u_grad_v(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e)
{
    Scalar result = 0;
    for (int i = 0; i < n; i++)
        result += wt[i] * e->x[i] * (u->dx[i] * v->dx[i] + u->dy[i] * v->dy[i]);
    return result;
}

template<typename Real, typename Scalar>
Scalar int_u_dvdx_over_x(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e)
{
    Scalar result = 0;
    for (int i = 0; i < n; i++)
        result += wt[i] * (v->dx[i] * u->val[i]) / e->x[i];
    return result;
}

template<typename Real, typename Scalar>
Scalar int_dvdx(int n, double *wt, Func<Real> *v)
{
  Scalar result = 0;
  for (int i = 0; i < n; i++)
    result += wt[i] * (v->dx[i]);
  return result;
}

template<typename Real, typename Scalar>
Scalar int_dvdy(int n, double *wt, Func<Real> *v)
{
  Scalar result = 0;
  for (int i = 0; i < n; i++)
    result += wt[i] * (v->dy[i]);
  return result;
}


template<typename Real, typename Scalar>
Scalar int_velocity(int n, double *wt, Func<Real> *u, Func<Real> *v, Geom<Real> *e, double vx, double vy, double omega)
{
    Scalar result = 0;
    for (int i = 0; i < n; i++)
        result += wt[i] * u->val[i] * ((vx - e->y[i] * omega) * v->dx[i] + (vy + e->x[i] * omega) * v->dy[i]);
    return result;
}

template<typename Real, typename Scalar>
Scalar int_magnet(int n, double *wt, Func<Real> *v, double angle)
{
    Scalar result = 0;
    for (int i = 0; i < n; i++)
        result += wt[i] * (- sin(angle / 180.0 * M_PI) * v->dx[i] + cos(angle / 180.0 * M_PI) * v->dy[i]);
    return result;
}

#endif // HERMES_FIELD_H
