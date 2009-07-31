#ifndef HARMONICMAGNETIC_H
#define HARMONICMAGNETIC_H

#include "util.h"
#include "scene.h"
#include "hermes2d.h"
#include "solverdialog.h"
#include "solver_umfpack.h"

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

SolutionArray *harmonicmagnetic_main(SolverDialog *solverDialog,
                                     const char *fileName,
                                     HarmonicMagneticEdge *edge,
                                     HarmonicMagneticLabel *label);

#endif // HARMONICMAGNETIC_H
