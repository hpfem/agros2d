#ifndef COUPLING_H
#define COUPLING_H
#include "util.h"
#include "hermes2d.h"

#include <rapidxml.cpp>
#include <rapidxml_utils.cpp>

class FieldInfo;
class ParserFormMatrix;
class ParserFormVector;

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
    Hermes::vector<ParserFormMatrix *> weakform_matrix_volume;
    Hermes::vector<ParserFormVector *> weakform_vector_volume;


    void read(std::string filename);
    void clear();
private:

    //TODO temporary, should be 2 (for source and object), or some other way
    AnalysisType m_analysisType;

    CoordinateType m_coordinateType;
};

//}
//}

#endif // COUPLING_H
