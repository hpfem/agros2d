#include "hermes_field.h"

#include "hermes_general.h"
#include "hermes_electrostatic.h"
#include "hermes_magnetostatic.h"
#include "hermes_harmonicmagnetic.h"
#include "hermes_heat.h"
#include "hermes_current.h"
#include "hermes_elasticity.h"

HermesField *hermesFieldFactory(PhysicField physicField)
{
    switch (physicField)
    {
    case PHYSICFIELD_GENERAL:
        return new HermesGeneral();
    case PHYSICFIELD_ELECTROSTATIC:
        return new HermesElectrostatic();
    case PHYSICFIELD_MAGNETOSTATIC:
        return new HermesMagnetostatic();
    case PHYSICFIELD_HARMONIC_MAGNETIC:
        return new HermesHarmonicMagnetic();
    case PHYSICFIELD_HEAT_TRANSFER:
        return new HermesHeat();
    case PHYSICFIELD_CURRENT:
        return new HermesCurrent();
    case PHYSICFIELD_ELASTICITY:
        return new HermesElasticity();
    default:
        std::cerr << "Physical field '" + QString::number(physicField).toStdString() + "' is not implemented. hermesObjectFactory()" << endl;
        throw;
        break;
    }
}
