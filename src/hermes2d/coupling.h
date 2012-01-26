#ifndef COUPLING_H
#define COUPLING_H
#include "util.h"
#include "hermes2d.h"

#include <rapidxml.cpp>
#include <rapidxml_utils.cpp>

class FieldInfo;
class ParserFormMatrix;
class ParserFormVector;

struct Coupling{
    // id
    std::string id;
    // name
    std::string name;
    // description
    std::string description;

    FieldInfo* sourceField;
    FieldInfo* influencedField;

    // weak forms
    Hermes::vector<ParserFormMatrix *> weakform_matrix_volume;
    Hermes::vector<ParserFormVector *> weakform_vector_volume;


    void read(std::string filename);
    void clear();
};


#endif // COUPLING_H
