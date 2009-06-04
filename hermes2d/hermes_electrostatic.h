#ifndef ELECTROSTATIC_H
#define ELECTROSTATIC_H

#include "util.h"
#include "scenehermes.h"
#include "hermes2d.h"
#include "solver_umfpack.h"

using namespace std;

struct SolutionArray;

struct ElectrostaticEdge
{
public:
    PhysicFieldBC type;
    double value;        
};

struct ElectrostaticLabel
{
    double charge_density;
    double permittivity;
};

static ElectrostaticEdge *electrostaticEdge;
static ElectrostaticLabel *electrostaticLabel;
static bool electrostaticIsPlanar;

SolutionArray *electrostatic_main(const char *fileName,
                                 ElectrostaticEdge *edge,
                                 ElectrostaticLabel *label,
                                 int numberOfRefinements,
                                 int polynomialOrder,
                                 int adaptivitySteps,
                                 double adaptivityTolerance,
                                 bool isPlanar);

#endif // ELECTROSTATIC_H
