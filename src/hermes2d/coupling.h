#ifndef COUPLING_H
#define COUPLING_H
#include "util.h"
#include "hermes2d.h"

#include <rapidxml.cpp>
#include <rapidxml_utils.cpp>

class FieldInfo;
class ProblemInfo;
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
    FieldInfo* targetField;

    Coupling(CoordinateType coordinateType, CouplingType couplingType);
    ~Coupling();

    // weak forms
    Hermes::vector<ParserFormExpression *> weakform_matrix_volume;
    Hermes::vector<ParserFormExpression *> weakform_vector_volume;

    mu::Parser *getParser();

    void read(std::string filename);
    void clear();

    inline CoordinateType get_coordinate_type() const { return m_coordinateType; }

private:
    CouplingType m_couplingType;
    CoordinateType m_coordinateType;
};

// available couplings
std::map<std::string, std::string> availableCouplings();


// coupling factory
Coupling *couplingFactory(FieldInfo* sourceField, FieldInfo* targetField, CoordinateType problem_type, CouplingType coupling_type,
                                           std::string filename_custom = "");


class CouplingInfo
{
public:
    CouplingInfo(ProblemInfo* parent, QString couplingId = "");

    inline Coupling *coupling() const { return m_coupling; }

    QString couplingId() { return m_couplingId; }
    CouplingType couplingType() { return m_couplingType; }
    void setCouplingType(CouplingType couplingType);

    FieldInfo* sourceFieldInfo();
    FieldInfo* targetFIeldInfo();

    /// weakforms
    WeakFormsType weakFormsType;

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
