#ifndef ELECTROSTATIC_H
#define ELECTROSTATIC_H

#include "util.h"
#include "scene.h"
#include "hermes2d.h"
#include "solverdialog.h"
#include "solver_umfpack.h"

struct SolutionArray;
class SolverThread;

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

SolutionArray *electrostatic_main(SolverThread *solverThread,
                                  const char *fileName,
                                  ElectrostaticEdge *edge,
                                  ElectrostaticLabel *label);

#endif // ELECTROSTATIC_H
