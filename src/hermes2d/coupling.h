#ifndef COUPLING_H
#define COUPLING_H
#include "util.h"
#include "hermes2d.h"

class FieldInfo;
class ProblemConfig;
class ParserFormExpression;

// namespace Module
// {

struct Coupling
{
    Coupling(const QString &couplingId,
             CoordinateType coordinateType, CouplingType couplingType,
             AnalysisType sourceFieldAnalysis, AnalysisType targetFieldAnalysis);
    ~Coupling();

    // id
    inline QString couplingId() const { return m_couplingId; }
    // name
    inline QString name() const { return m_name; }
    // description
    inline QString description() const { return m_description; }

    // constants
    inline QMap<QString, double> constants() const { return m_constants; }

    // weak forms
    inline QList<ParserFormExpression *> wfMatrixVolumeExpression() const { return m_wfMatrixVolumeExpression; }
    inline QList<ParserFormExpression *> wfVectorVolumeExpression() const { return m_wfVectorVolumeExpression; }

    mu::Parser *expressionParser();

    inline CoordinateType coordinateType() const { return m_coordinateType; }

private:
    CouplingType m_couplingType;
    CoordinateType m_coordinateType;

    AnalysisType m_sourceFieldAnalysis;
    AnalysisType m_targetFieldAnalysis;

    // id
    QString m_couplingId;
    // name
    QString m_name;
    // description
    QString m_description;

    // constants
    QMap<QString, double> m_constants;

    // weak forms
    QList<ParserFormExpression *> m_wfMatrixVolumeExpression;
    QList<ParserFormExpression *> m_wfVectorVolumeExpression;

    void read(const QString &filename);
    void clear();
};

bool isCouplingAvailable(FieldInfo* sourceField, FieldInfo* targetField);

// coupling factory
Coupling *couplingFactory(FieldInfo* sourceField, FieldInfo* targetField, CouplingType couplingType);

class CouplingInfo
{
public:
    CouplingInfo(FieldInfo* sourceField, FieldInfo* targetField);
    ~CouplingInfo();

    inline Coupling *coupling() const { return m_coupling; }

    QString couplingId() { if (m_coupling) return m_coupling->couplingId(); return "none"; }
    CouplingType couplingType() { return m_couplingType; }
    inline bool isHard() { return m_couplingType == CouplingType_Hard;}
    inline bool isWeak() { return m_couplingType == CouplingType_Weak;}
    void setCouplingType(CouplingType couplingType);

    inline FieldInfo* sourceField() {return m_sourceField; }
    inline FieldInfo* targetField() {return m_targetField; }

    bool isRelated(FieldInfo* fieldInfo) { return((fieldInfo == sourceField()) || (fieldInfo == targetField())); }

    /// weakforms
    WeakFormsType weakFormsType;

    /// reloads the Coupling ("module"). Should be called when couplingType or AnalysisType of either fields changes
    void reload();

    /// goes through field infos and adds/removes coupling infos accordingly
    static void synchronizeCouplings(const QMap<QString, FieldInfo *>& fieldInfos, QMap<QPair<FieldInfo*, FieldInfo* >, CouplingInfo* >& couplingInfos);

    LinearityType linearityType();

private:
    /// module
    Coupling *m_coupling;

    /// pointer to problem info
    ProblemConfig *m_problemInfo;

    FieldInfo* m_sourceField;
    FieldInfo* m_targetField;

    /// coupling type
    CouplingType m_couplingType;
};

//}


#endif // COUPLING_H
