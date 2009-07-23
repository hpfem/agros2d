#ifndef MAGNETOSTATIC_H
#define MAGNETOSTATIC_H

#include "util.h"
#include "hermes2d.h"
#include "solverdialog.h"
#include "solver_umfpack.h"

struct SolutionArray;
class SolverDialog;

struct MagnetostaticEdge
{
    PhysicFieldBC type;
    double value;        
};

struct MagnetostaticLabel
{
    double current_density;
    double permeability;
};

static MagnetostaticEdge *magnetostaticEdge;
static MagnetostaticLabel *magnetostaticLabel;
static bool magnetostaticIsPlanar;

SolutionArray *magnetostatic_main(SolverDialog *solverDialog,
                                  const char *fileName,
                                  MagnetostaticEdge *edge,
                                  MagnetostaticLabel *label,
                                  int numberOfRefinements,
                                  int polynomialOrder,
                                  int adaptivitySteps,
                                  double adaptivityTolerance,
                                  bool isPlanar);

#endif // MAGNETOSTATIC_H
