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

#ifndef FIELD_H
#define FIELD_H

#include "util.h"
#include "value.h"
#include "sceneedge.h"
#include "scenelabel.h"

namespace Module
{
    struct ModuleAgros;
}

namespace XMLProblem
{
    class field_config;
}

class ProblemConfig;
class CouplingInfo;
class LocalForceValue;
class PluginInterface;
class Value;

class AGROS_LIBRARY_API FieldInfo : public QObject
{
    Q_OBJECT

public:
    FieldInfo(QString fieldId = "", const AnalysisType analysisType = AnalysisType_Undefined);
    ~FieldInfo();

    void clear();

    inline PluginInterface *plugin() const { assert(m_plugin); return m_plugin; }

    QString fieldId() const { return m_fieldId; }

    inline MeshSharedPtr initialMesh() const { return m_initialMesh; }
    inline void clearInitialMesh() { m_initialMesh = MeshSharedPtr();}
    void setInitialMesh(MeshSharedPtr mesh);

    enum Type
    {
        Unknown,
        NonlinearTolerance,
        NonlinearConvergenceMeasurement,
        NewtonDampingType,
        NewtonDampingCoeff,
        NewtonReuseJacobian,
        NewtonSufImprovForJacobianReuse,
        NewtonSufImprov,
        NewtonMaxStepsReuseJacobian,
        NewtonStepsToIncreaseDF,
        PicardAndersonAcceleration,
        PicardAndersonBeta,
        PicardAndersonNumberOfLastVectors,
        SpaceNumberOfRefinements,
        SpacePolynomialOrder,
        AdaptivitySteps,
        AdaptivityTolerance,
        AdaptivityTransientBackSteps,
        AdaptivityTransientRedoneEach,
        AdaptivityStoppingCriterion,
        AdaptivityThreshold,
        AdaptivityErrorCalculator,
        AdaptivityUseAniso,
        AdaptivityFinerReference,
        AdaptivityOrderIncrease,
        AdaptivitySpaceRefinement,
        TransientTimeSkip,
        TransientInitialCondition,
        LinearSolverIterMethod,
        LinearSolverIterPreconditioner,
        LinearSolverIterToleranceAbsolute,
        LinearSolverIterIters,
        TimeUnit
    };

    // analysis type
    AnalysisType analysisType() const { return m_analysisType; }
    void setAnalysisType(const AnalysisType analysisType);

    // linearity
    inline LinearityType linearityType() const {return m_linearityType; }
    void setLinearityType(const LinearityType lt) { m_linearityType = lt; emit changed(); }

    QList<LinearityType> availableLinearityTypes(AnalysisType at) const;

    // adaptivity
    inline AdaptivityType adaptivityType() const { return m_adaptivityType; }
    void setAdaptivityType(const AdaptivityType at) { m_adaptivityType = at; emit changed(); }

    // matrix
    inline Hermes::MatrixSolverType matrixSolver() const { return m_matrixSolver; }
    void setMatrixSolver(const Hermes::MatrixSolverType matrixSolver) { m_matrixSolver = matrixSolver; emit changed(); }

    // number of solutions
    inline int numberOfSolutions() const { return m_numberOfSolutions; }

    const QMap<SceneEdge *, int> edgesRefinement() { return m_edgesRefinement; }
    int edgeRefinement(SceneEdge *edge);
    void setEdgeRefinement(SceneEdge *edge, int refinement) { m_edgesRefinement[edge] = refinement; }
    void removeEdgeRefinement(SceneEdge *edge) { m_edgesRefinement.remove(edge); }

    const QMap<SceneLabel *, int> labelsRefinement() { return m_labelsRefinement; }
    int labelRefinement(SceneLabel *label);
    void setLabelRefinement(SceneLabel *label, int refinement) { m_labelsRefinement[label] = refinement; }
    void removeLabelRefinement(SceneLabel *label) { m_labelsRefinement.remove(label); }

    const QMap<SceneLabel *, int> labelsPolynomialOrder() { return m_labelsPolynomialOrder; }
    int labelPolynomialOrder(SceneLabel *label);
    void setLabelPolynomialOrder(SceneLabel *label, int order) { m_labelsPolynomialOrder[label] = order; }
    void removeLabelPolynomialOrder(SceneLabel *label) { m_labelsPolynomialOrder.remove(label); }

    void load(XMLProblem::field_config *configxsd);
    void save(XMLProblem::field_config *configxsd);

    inline QString typeToStringKey(Type type) { return m_settingKey[type]; }
    inline Type stringKeyToType(const QString &key) { return m_settingKey.key(key); }
    inline QStringList stringKeys() { return m_settingKey.values(); }

    inline QVariant value(Type type) const { return m_setting[type]; }
    inline void setValue(Type type, int value, bool emitChanged = true) {  m_setting[type] = value; if (emitChanged) emit changed(); }
    inline void setValue(Type type, double value, bool emitChanged = true) {  m_setting[type] = value; emit changed(); if (emitChanged) emit changed(); }
    inline void setValue(Type type, bool value, bool emitChanged = true) {  m_setting[type] = value; emit changed(); if (emitChanged) emit changed(); }
    inline void setValue(Type type, const std::string &value, bool emitChanged = true) { setValue(type, QString::fromStdString(value), emitChanged); }
    inline void setValue(Type type, const QString &value, bool emitChanged = true) { m_setting[type] = value; emit changed(); if (emitChanged) emit changed(); }
    inline void setValue(Type type, const QStringList &value, bool emitChanged = true) { m_setting[type] = value; emit changed(); if (emitChanged) emit changed(); }

    inline QVariant defaultValue(Type type) {  return m_settingDefault[type]; }

    // refine mesh
    void refineMesh(MeshSharedPtr mesh, bool refineGlobal, bool refineTowardsEdge, bool refineArea);

    // name
    QString name() const;

    // description
    QString description() const;

    // deformable shape
    bool hasDeformableShape() const;

    // latex equation
    QString equation() const;

    // constants
    QMap<QString, double> constants() const;

    // macros
    QMap<QString, QString> macros() const;

    QMap<AnalysisType, QString> analyses() const;

    // spaces
    QMap<int, Module::Space> spaces() const;

    // material type
    QList<Module::MaterialTypeVariable> materialTypeVariables() const;
    // variable by name
    bool materialTypeVariableContains(const QString &id) const;
    Module::MaterialTypeVariable materialTypeVariable(const QString &id) const;

    // boundary conditions
    QList<Module::BoundaryType> boundaryTypes() const;
    // default boundary condition
    Module::BoundaryType boundaryTypeDefault() const;
    // variable by name
    bool boundaryTypeContains(const QString &id) const;
    Module::BoundaryType boundaryType(const QString &id) const;
    Module::BoundaryTypeVariable boundaryTypeVariable(const QString &id) const;

    // force
    Module::Force force() const;

    // error calculators
    QList<Module::ErrorCalculator> errorCalculators() const;

    // material and boundary user interface
    Module::DialogUI materialUI() const;
    Module::DialogUI boundaryUI() const;

    // local point variables
    QList<Module::LocalVariable> localPointVariables() const;
    // view scalar and vector variables
    QList<Module::LocalVariable> viewScalarVariables() const;
    QList<Module::LocalVariable> viewVectorVariables() const;
    // surface integrals
    QList<Module::Integral> surfaceIntegrals() const;
    // volume integrals
    QList<Module::Integral> volumeIntegrals() const;

    // variable by name
    Module::LocalVariable localVariable(const QString &id) const;
    Module::Integral surfaceIntegral(const QString &id) const;
    Module::Integral volumeIntegral(const QString &id) const;

    // default variables
    Module::LocalVariable defaultViewScalarVariable() const;
    Module::LocalVariable defaultViewVectorVariable() const;

    QList<LinearityType> availableLinearityTypes() const {return m_availableLinearityTypes;}

signals:
    void changed();

private:
    /// plugin
    PluginInterface *m_plugin;

    /// pointer to problem info, whose this object is a "subfield"
    ProblemConfig *m_parent;

    /// unique field info
    QString m_fieldId;

    // initial mesh
    MeshSharedPtr m_initialMesh;

    // analysis type
    AnalysisType m_analysisType;
    // number of solutions cache
    int m_numberOfSolutions;

    // linearity
    LinearityType m_linearityType;
    QList<LinearityType> m_availableLinearityTypes;

    // adaptivity
    AdaptivityType m_adaptivityType;

    // matrix solver
    Hermes::MatrixSolverType m_matrixSolver;

    // TODO: (Franta) gmsh
    QMap<SceneEdge *, int> m_edgesRefinement;
    QMap<SceneLabel *, int> m_labelsRefinement;
    QMap<SceneLabel *, int> m_labelsPolynomialOrder;

    QMap<Type, QVariant> m_setting;
    QMap<Type, QVariant> m_settingDefault;
    QMap<Type, QString> m_settingKey;

    void setDefaultValues();
    void setStringKeys();

    // help functions extracting parts of xml
};

XMLModule::linearity_option volumeLinearityOption(XMLModule::module* module, AnalysisType analysisType, LinearityType linearityType);

ostream& operator<<(ostream& output, FieldInfo& id);

class Field
{
public:
    Field(FieldInfo* fieldInfo);
    bool solveInitVariables();

    FieldInfo* fieldInfo() { return m_fieldInfo; }

    void addCouplingInfo(CouplingInfo *couplingInfo) { m_couplingInfos.append(couplingInfo); }
    QList<CouplingInfo* > couplingInfos() { return m_couplingInfos; }

    // mesh
    void setMeshInitial(MeshSharedPtr meshInitial);

private:
    QList<CouplingInfo* > m_couplingInfos;
    FieldInfo* m_fieldInfo;
};

#endif // FIELD_H
