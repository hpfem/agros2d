#ifndef FORM_INFO_H
#define FORM_INFO_H

#include "util.h"
#include "util/enums.h"

struct AGROS_LIBRARY_API FormInfo
{
    FormInfo() : id(""), i(0), j(0), sym_planar(HERMES_NONSYM), sym_axi(HERMES_NONSYM), variant(WeakFormVariant_Normal), coefficient(1) {}
    FormInfo(const QString &id, int i = 0, int j = 0, SymFlag sym_planar = HERMES_NONSYM, SymFlag sym_axi = HERMES_NONSYM)
        : id(id), i(i), j(j), sym_planar(sym_planar), sym_axi(sym_axi), variant(WeakFormVariant_Normal), coefficient(1), condition("") {}

    QString id;

    // position
    int i;
    int j;

    // symmetric flag
    SymFlag sym_planar;
    SymFlag sym_axi;

    QString expr_planar;
    QString expr_axi;
    WeakFormVariant variant;
    double coefficient;
    QString condition;

    SymFlag sym(CoordinateType coordinateType) { return (coordinateType == CoordinateType_Axisymmetric) ? sym_axi : sym_planar; }
};

#endif // FORM_INFO_H
