#include "hermes_field.h"

#include "hermes_general.h"
#include "hermes_electrostatic.h"
#include "hermes_magnetic.h"
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
    case PHYSICFIELD_MAGNETIC:
        return new HermesMagnetic();
    case PHYSICFIELD_HEAT:
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
