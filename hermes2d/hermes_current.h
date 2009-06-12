#ifndef HERMES_CURRENT_H
#define HERMES_CURRENT_H


#include "util.h"
#include "solverdialog.h"
#include "hermes2d.h"
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

static CurrentEdge *currentEdge;
static CurrentLabel *currentLabel;
static bool currentIsPlanar;

SolutionArray *current_main(SolverDialog *solverDialog,
                                  const char *fileName,
                                  CurrentEdge *edge,
                                  CurrentLabel *label,
                                  int numberOfRefinements,
                                  int polynomialOrder,
                                  int adaptivitySteps,
                                  double adaptivityTolerance,
                                  bool isPlanar);

#endif // HERMES_CURRENT_H
