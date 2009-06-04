#ifndef HEAT_H
#define HEAT_H

#include "util.h"
#include "scenehermes.h"
#include "hermes2d.h"
#include "solver_umfpack.h"

using namespace std;

struct SolutionArray;

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

static HeatEdge *heatEdge;
static HeatLabel *heatLabel;
static bool heatIsPlanar;

SolutionArray *heat_main(const char *fileName,
                        HeatEdge *edge,
                        HeatLabel *label,
                        int numberOfRefinements,
                        int polynomialOrder,
                        int adaptivitySteps,
                        double adaptivityTolerance,
                        bool isPlanar);

#endif // HEAT_H
