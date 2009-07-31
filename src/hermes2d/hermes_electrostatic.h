#ifndef ELECTROSTATIC_H
#define ELECTROSTATIC_H

#include "util.h"
#include "scene.h"
#include "hermes2d.h"
#include "solverdialog.h"
#include "solver_umfpack.h"

struct SolutionArray;
class SolverDialog;

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

SolutionArray *electrostatic_main(SolverDialog *solverDialog,
                                  const char *fileName,
                                  ElectrostaticEdge *edge,
                                  ElectrostaticLabel *label);

#endif // ELECTROSTATIC_H
