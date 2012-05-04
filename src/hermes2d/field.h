#ifndef FIELD_H
#define FIELD_H

#include "util.h"

namespace Module
{
    struct ModuleAgros;
}

class ProblemConfig;
class CouplingInfo;

class FieldInfo : public QObject
{
    Q_OBJECT

public:
    FieldInfo(QString fieldId = "", const AnalysisType analysisType = AnalysisType_Undefined);
    ~FieldInfo();

    void clear();

    inline Module::ModuleAgros *module() const { return m_module; }

    QString fieldId() const { return m_fieldId; }
    AnalysisType analysisType() const { return m_analysisType; }
    void setAnalysisType(const AnalysisType analysisType);

    inline LinearityType linearityType() const {return m_linearityType; }
    void setLinearityType(const LinearityType lt) { m_linearityType = lt; emit changed(); }

    inline double nonlinearTolerance() const { return m_nonlinearTolerance; }
    void setNonlinearTolerance(const double nt) { m_nonlinearTolerance = nt; emit changed(); }

    inline int nonlinearSteps() const { return m_nonlinearSteps; }
    void setNonlinearSteps(const int ns) { m_nonlinearSteps = ns; emit changed(); }

    inline int numberOfRefinements() const { return m_numberOfRefinements; }
    void setNumberOfRefinements(const int nr) {m_numberOfRefinements = nr; emit changed(); }

    inline int polynomialOrder() const { return m_polynomialOrder; }
    void setPolynomialOrder(const int po) { m_polynomialOrder = po; emit changed(); }

    inline AdaptivityType adaptivityType() const { return m_adaptivityType; }
    void setAdaptivityType(const AdaptivityType at) { m_adaptivityType = at; emit changed(); }

    inline int adaptivitySteps() const { return m_adaptivitySteps; }
    void setAdaptivitySteps(const int as) { m_adaptivitySteps = as; emit changed(); }

    inline double adaptivityTolerance() const { return m_adaptivityTolerance; }
    void setAdaptivityTolerance(const double at) { m_adaptivityTolerance = at; emit changed(); }

    inline Value initialCondition() const { return m_initialCondition; }
    void setInitialCondition(const Value& value) { m_initialCondition = value; emit changed(); }

    inline WeakFormsType weakFormsType() const { return m_weakFormsType; }
    void setWeakFormsType(const WeakFormsType wft) { m_weakFormsType = wft; emit changed(); }

    // module
    QString name();
    // description
    QString description();

signals:
    void changed();

private:
    /// module
    Module::ModuleAgros *m_module;

    /// pointer to problem info, whose this object is a "subfield"
    ProblemConfig *m_parent;

    /// unique field info
    QString m_fieldId;

    // analysis type
    AnalysisType m_analysisType;

    // linearity
    LinearityType m_linearityType;
    double m_nonlinearTolerance; // percent
    int m_nonlinearSteps;

    int m_numberOfRefinements;
    int m_polynomialOrder;

    AdaptivityType m_adaptivityType;
    int m_adaptivitySteps;
    double m_adaptivityTolerance; // percent

    // transient
    Value m_initialCondition;

    // weakforms
    WeakFormsType m_weakFormsType;
};

ostream& operator<<(ostream& output, FieldInfo& id);

class Field
{
public:
    Field(FieldInfo* fieldInfo);
    bool solveInitVariables();
    FieldInfo* fieldInfo() { return m_fieldInfo; }

    // mesh
    void setMeshInitial(Hermes::Hermes2D::Mesh *meshInitial);

public:
//private:
    QList<CouplingInfo* > m_couplingSources;
    FieldInfo* m_fieldInfo;

};

#endif // FIELD_H
