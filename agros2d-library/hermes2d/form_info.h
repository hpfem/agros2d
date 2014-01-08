#ifndef FORM_INFO_H
#define FORM_INFO_H

#include "util.h"
#include "util/enums.h"
#include "hermes2d.h"

struct AGROS_LIBRARY_API FormInfo
{
    FormInfo() : id(""), i(0), j(0), sym_planar(Hermes::Hermes2D::HERMES_NONSYM), sym_axi(Hermes::Hermes2D::HERMES_NONSYM), variant(WeakFormVariant_Normal), coefficient(1) {}
    FormInfo(const QString &id, int i = 0, int j = 0, Hermes::Hermes2D::SymFlag sym_planar = Hermes::Hermes2D::HERMES_NONSYM, Hermes::Hermes2D::SymFlag sym_axi = Hermes::Hermes2D::HERMES_NONSYM)
        : id(id), i(i), j(j), sym_planar(sym_planar), sym_axi(sym_axi), variant(WeakFormVariant_Normal), coefficient(1), condition("") {}

    QString id;

    // position
    int i;
    int j;

    // symmetric flag
    Hermes::Hermes2D::SymFlag sym_planar;
    Hermes::Hermes2D::SymFlag sym_axi;

    QString expr_planar;
    QString expr_axi;
    WeakFormVariant variant;
    double coefficient;
    QString condition;

    Hermes::Hermes2D::SymFlag sym(CoordinateType coordinateType) { return (coordinateType == CoordinateType_Axisymmetric) ? sym_axi : sym_planar; }
};

#endif // FORM_INFO_H
