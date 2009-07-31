#ifndef HEAT_H
#define HEAT_H

#include "util.h"
#include "scene.h"
#include "hermes2d.h"
#include "solverdialog.h"
#include "solver_umfpack.h"

struct SolutionArray;
class SolverDialog;

struct HeatEdge
{
    PhysicFieldBC type;
    double temperature;
    double heatFlux;
    double h;
    double externalTemperature;
};

struct HeatLabel
{
    double thermal_conductivity;
    double volume_heat;
};

SolutionArray *heat_main(SolverDialog *solverDialog,
                         const char *fileName,
                         HeatEdge *edge,
                         HeatLabel *label);

#endif // HEAT_H
