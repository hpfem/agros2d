#ifndef HARMONICMAGNETIC_H
#define HARMONICMAGNETIC_H

#include "util.h"
#include "solverdialog.h"
#include "hermes2d.h"
#include "solver_umfpack.h"

#include <iostream>

using namespace std;

struct SolutionArray;
class SolverDialog;

struct HarmonicMagneticEdge
{
    PhysicFieldBC type;
    double value;        
};

struct HarmonicMagneticLabel
{
    double current_density_real;
    double current_density_imag;
    double permeability;
    double conductivity;
};

static HarmonicMagneticEdge *harmonicmagneticEdge;
static HarmonicMagneticLabel *harmonicmagneticLabel;
static bool harmonicmagneticIsPlanar;

SolutionArray *harmonicmagnetic_main(SolverDialog *solverDialog,
                                     const char *fileName,
                                     HarmonicMagneticEdge *edge,
                                     HarmonicMagneticLabel *label,
                                     int numberOfRefinements,
                                     int polynomialOrder,
                                     int adaptivitySteps,
                                     double adaptivityTolerance,
                                     bool isPlanar);

#endif // HARMONICMAGNETIC_H
