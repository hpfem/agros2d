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

#ifndef HERMES_ELASTICITY_H
#define HERMES_ELASTICITY_H

#include "util.h"
#include "hermes_field.h"

struct HermesElasticity : public HermesField
{
    Q_OBJECT
public:
    PhysicField physicField() const { return PhysicField_Elasticity; }

    inline int numberOfSolution() const { return 2; }
    bool hasSteadyState() const { return true; }
    bool hasHarmonic() const { return false; }
    bool hasTransient() const { return false; }

    void readEdgeMarkerFromDomElement(QDomElement *element);
    void writeEdgeMarkerToDomElement(QDomElement *element, SceneEdgeMarker *marker);
    void readLabelMarkerFromDomElement(QDomElement *element);
    void writeLabelMarkerToDomElement(QDomElement *element, SceneLabelMarker *marker);

    LocalPointValue *localPointValue(const Point &point);
    QStringList localPointValueHeader();

    SurfaceIntegralValue *surfaceIntegralValue();
    QStringList surfaceIntegralValueHeader();

    VolumeIntegralValue *volumeIntegralValue();
    QStringList volumeIntegralValueHeader();

    inline bool physicFieldBCCheck(PhysicFieldBC physicFieldBC) { return (physicFieldBC == PhysicFieldBC_Elasticity_Fixed ||
                                                                          physicFieldBC == PhysicFieldBC_Elasticity_Free); }
    inline bool physicFieldVariableCheck(PhysicFieldVariable physicFieldVariable) { return (physicFieldVariable == PhysicFieldVariable_Elasticity_VonMisesStress ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Elasticity_Displacement ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Elasticity_StrainXX ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Elasticity_StrainYY ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Elasticity_StrainZZ ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Elasticity_StrainXY ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Elasticity_StressXX ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Elasticity_StressYY ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Elasticity_StressZZ ||
                                                                                            physicFieldVariable == PhysicFieldVariable_Elasticity_StressXY); }

    SceneEdgeMarker *newEdgeMarker();
    SceneEdgeMarker *newEdgeMarker(PyObject *self, PyObject *args);
    SceneEdgeMarker *modifyEdgeMarker(PyObject *self, PyObject *args);
    SceneLabelMarker *newLabelMarker();
    SceneLabelMarker *newLabelMarker(PyObject *self, PyObject *args);
    SceneLabelMarker *modifyLabelMarker(PyObject *self, PyObject *args);

    QList<SolutionArray *> solve(ProgressItemSolve *progressItemSolve);

    inline PhysicFieldVariable contourPhysicFieldVariable() { return PhysicFieldVariable_Elasticity_Displacement; }
    inline PhysicFieldVariable scalarPhysicFieldVariable() { return PhysicFieldVariable_Elasticity_VonMisesStress; }
    inline PhysicFieldVariableComp scalarPhysicFieldVariableComp() { return PhysicFieldVariableComp_Scalar; }
    inline PhysicFieldVariable vectorPhysicFieldVariable() { return PhysicFieldVariable_Elasticity_Displacement; }

    void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable);

    void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable)
    {
        cmbFieldVariable->addItem(physicFieldVariableString(PhysicFieldVariable_Elasticity_Displacement), PhysicFieldVariable_Elasticity_Displacement);
    }

    void showLocalValue(QTreeWidget *trvWidget, LocalPointValue *localPointValue);
    void showSurfaceIntegralValue(QTreeWidget *trvWidget, SurfaceIntegralValue *surfaceIntegralValue);
    void showVolumeIntegralValue(QTreeWidget *trvWidget, VolumeIntegralValue *volumeIntegralValue);

    ViewScalarFilter *viewScalarFilter(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);

    void deformShape(double3* linVert, int count);
    void deformShape(double4* linVert, int count);
};

class LocalPointValueElasticity : public LocalPointValue
{
public:
    double young_modulus;
    double poisson_ratio;
    double von_mises_stress;
    double tresca_stress;
    double forceX;
    double forceY;
    double alpha;
    double temp;
    double temp_ref;
    Point d;
    double strain_x;
    double strain_y;
    double strain_z;
    double strain_xy;
    double stress_x;
    double stress_y;
    double stress_z;
    double stress_xy;

    LocalPointValueElasticity(const Point &point);
    double variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp);
    QStringList variables();
};

class SurfaceIntegralValueElasticity : public SurfaceIntegralValue
{
protected:
    void calculateVariables(int i);

public:
    SurfaceIntegralValueElasticity();

    QStringList variables();
};

class VolumeIntegralValueElasticity : public VolumeIntegralValue
{
protected:
    void calculateVariables(int i);
    void initSolutions();

public:
    VolumeIntegralValueElasticity();
    QStringList variables();
};

class ViewScalarFilterElasticity : public ViewScalarFilter
{
public:
    ViewScalarFilterElasticity(Hermes::vector<MeshFunction *> sln, PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp) :
            ViewScalarFilter(sln, physicFieldVariable, physicFieldVariableComp) {}

protected:
    void calculateVariable(int i);
};

class SceneEdgeElasticityMarker : public SceneEdgeMarker
{
public:
    PhysicFieldBC typeX;
    PhysicFieldBC typeY;
    Value forceX;
    Value forceY;
    Value displacementX;
    Value displacementY;

    QString script();
    QMap<QString, QString> data();
    SceneEdgeElasticityMarker(const QString &name, PhysicFieldBC typeX, PhysicFieldBC typeY,
                              Value forceX, Value forceY, Value displacementX, Value displacementY);

    int showDialog(QWidget *parent);
};

class SceneLabelElasticityMarker : public SceneLabelMarker
{
public:
    Value young_modulus;
    Value poisson_ratio;
    Value forceX;
    Value forceY;
    Value alpha;
    Value temp;
    Value temp_ref;

    SceneLabelElasticityMarker(const QString &name, Value young_modulus, Value poisson_ratio, Value forceX, Value forceY,
                               Value alpha, Value temp, Value temp_ref);

    // Lame constant
    inline double lambda() { return (young_modulus.number * poisson_ratio.number) / ((1.0 + poisson_ratio.number) * (1.0 - 2.0*poisson_ratio.number)); }
    inline double mu() { return young_modulus.number / (2.0*(1.0 + poisson_ratio.number)); }

    QString script();
    QMap<QString, QString> data();
    int showDialog(QWidget *parent);
};

class DSceneEdgeElasticityMarker : public DSceneEdgeMarker
{
    Q_OBJECT

public:
    DSceneEdgeElasticityMarker(SceneEdgeElasticityMarker *edgeEdgeElasticityMarker, QWidget *parent);

protected:
    void createContent();

    void load();
    bool save();

private:
    QLabel *lblEquationX;
    QLabel *lblEquationImageX;
    QLabel *lblEquationY;
    QLabel *lblEquationImageY;
    QComboBox *cmbTypeX;
    QComboBox *cmbTypeY;
    ValueLineEdit *txtForceX;
    ValueLineEdit *txtForceY;
    ValueLineEdit *txtDisplacementX;
    ValueLineEdit *txtDisplacementY;

private slots:
    void doTypeXChanged(int index);
    void doTypeYChanged(int index);
};

class DSceneLabelElasticityMarker : public DSceneLabelMarker
{
    Q_OBJECT

public:
    DSceneLabelElasticityMarker(QWidget *parent, SceneLabelElasticityMarker *labelElasticityMarker);

protected:
    void createContent();

    void load();
    bool save();

private:
    ValueLineEdit *txtYoungModulus;
    ValueLineEdit *txtPoissonNumber;
    ValueLineEdit *txtForceX;
    ValueLineEdit *txtForceY;
    ValueLineEdit *txtAlpha;
    ValueLineEdit *txtTemp;
    ValueLineEdit *txtTempRef;
};

#endif // HERMES_ELASTICITY_H
