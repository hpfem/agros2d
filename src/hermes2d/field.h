#ifndef FIELD_H
#define FIELD_H

#include "util.h"

namespace Hermes
{
    namespace Module
    {
        struct ModuleAgros;
    }
}

class ProblemConfig;
class CouplingInfo;

class FieldInfo
{
public:
    FieldInfo(QString fieldId = "");
    ~FieldInfo();

    void clear();

    inline Hermes::Module::ModuleAgros *module() const { return m_module; }

    QString fieldId() { return m_fieldId; }
    AnalysisType analysisType() { return m_analysisType; }
    void setAnalysisType(AnalysisType analysisType);

    // linearity
    LinearityType linearityType;
    double nonlinearTolerance; // percent
    int nonlinearSteps;

    int numberOfRefinements;
    int polynomialOrder;

    AdaptivityType adaptivityType;
    int adaptivitySteps;
    double adaptivityTolerance; // percent

    // transient
    Value initialCondition;

    // weakforms
    WeakFormsType weakFormsType;
private:
    /// module
    Hermes::Module::ModuleAgros *m_module;

    /// pointer to problem info, whose this object is a "subfield"
    ProblemConfig *m_parent;

    /// unique field info
    QString m_fieldId;

    // analysis type
    AnalysisType m_analysisType;
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
