#ifndef HERMES_FIELD_H
#define HERMES_FIELD_H

#include <QDomDocument>

#include <Python.h>

#include "util.h"
#include "hermes2d.h"
#include "solver_umfpack.h"

#include "localvalueview.h"
#include "surfaceintegralview.h"
#include "volumeintegralview.h"

#include "scenemarker.h"
#include "solverdialog.h"

class LocalPointValue;
class VolumeIntegralValue;
class SurfaceIntegralValue;

class SceneEdgeMarker;
class SceneLabelMarker;
class SceneViewSettings;

class SolutionArray;
class SolverThread;

struct HermesField : public QObject
{
    Q_OBJECT
public:
    PhysicField physicField;
    HermesField() { physicField = PHYSICFIELD_UNDEFINED; }

    virtual int numberOfSolution() = 0;
    virtual bool hasFrequency() = 0;
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
    virtual SceneLabelMarker *newLabelMarker() = 0;
    virtual SceneLabelMarker *newLabelMarker(PyObject *self, PyObject *args) = 0;

    virtual QList<SolutionArray *> *solve(SolverThread *solverThread) = 0;

    virtual PhysicFieldVariable contourPhysicFieldVariable() = 0;
    virtual PhysicFieldVariable scalarPhysicFieldVariable() = 0;
    virtual PhysicFieldVariableComp scalarPhysicFieldVariableComp() = 0;
    virtual PhysicFieldVariable vectorPhysicFieldVariable() = 0;

    virtual void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable) = 0;
    virtual void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable) = 0;

    virtual void showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue) = 0;
    virtual void showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue) = 0;
    virtual void showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue) = 0;
};

HermesField *hermesFieldFactory(PhysicField physicField);


// custom forms

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

#endif // HERMES_FIELD_H
