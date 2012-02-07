#ifndef COUPLING_H
#define COUPLING_H
#include "util.h"
#include "hermes2d.h"

#include <rapidxml.cpp>
#include <rapidxml_utils.cpp>

class FieldInfo;
class ParserFormExpression;

//namespace Hermes{
//namespace Module{

struct Coupling{
    // id
    std::string id;
    // name
    std::string name;
    // description
    std::string description;

    FieldInfo* sourceField;
    FieldInfo* objectField;

    Coupling(CoordinateType coordinateType);
    ~Coupling();

    // weak forms
    Hermes::vector<ParserFormExpression *> weakform_matrix_volume;
    Hermes::vector<ParserFormExpression *> weakform_vector_volume;


    void read(std::string filename);
    void clear();
private:

    //TODO temporary, should be 2 (for source and object), or some other way
    AnalysisType m_analysisType;

    CoordinateType m_coordinateType;
};

class CouplingInfo
{
public:
    CouplingInfo(QString couplingId = "");

    inline Coupling *coupling() const { return m_coupling; }

    QString couplingId() { return m_couplingId; }
    CouplingType couplingType() { return m_couplingType; }
    void setCouplingType(CouplingType couplingType);

private:
    /// module
    Coupling *m_coupling;

    /// pointer to problem info
    ProblemInfo *m_parent;

    /// unique coupling info
    QString m_couplingId;

    /// coupling type
    CouplingType m_couplingType;

};

//}
//}

#endif // COUPLING_H
