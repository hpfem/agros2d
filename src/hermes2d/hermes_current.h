#ifndef HERMES_CURRENT_H
#define HERMES_CURRENT_H

#include "util.h"
#include "scene.h"
#include "hermes2d.h"
#include "solverdialog.h"
#include "solver_umfpack.h"

struct SolutionArray;
class SolverDialog;

struct CurrentEdge
{
public:
    PhysicFieldBC type;
    double value;
};

struct CurrentLabel
{
    double conductivity;
};

SolutionArray *current_main(SolverDialog *solverDialog,
                                  const char *fileName,
                                  CurrentEdge *edge,
                                  CurrentLabel *label);

#endif // HERMES_CURRENT_H
