#include "localvalueview.h"

LocalPointValueView::LocalPointValueView(Scene *scene, QWidget *parent): QDockWidget(tr("Local Values"), parent)
{
    m_scene = scene;

    setMinimumWidth(280);
    setObjectName("LocalPointValueView");

    createActions();
    createMenu();

    trvWidget = new QTreeWidget();
    trvWidget->setHeaderHidden(true);
    trvWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    trvWidget->setMouseTracking(true);
    trvWidget->setColumnCount(3);
    trvWidget->setColumnWidth(0, 150);
    trvWidget->setColumnWidth(1, 80);
    trvWidget->setColumnWidth(2, 20);

    connect(trvWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));

    setWidget(trvWidget);
}

void LocalPointValueView::createActions()
{
    actPoint = new QAction(icon("scene-node"), tr("Local point value"), this);
    connect(actPoint, SIGNAL(triggered()), this, SLOT(doPoint()));
}

void LocalPointValueView::createMenu()
{
    mnuInfo = new QMenu(this);

    mnuInfo->addAction(actPoint);
}

void LocalPointValueView::doPoint()
{
    Point point;
    SceneNode *node = new SceneNode(point);
    if (node->showDialog(m_scene, this) == QDialog::Accepted)
    {
        doShowPoint(localPointValueFactory(node->point, m_scene));
    }
    delete node;
}

void LocalPointValueView::doContextMenu(const QPoint &pos)
{
    mnuInfo->exec(QCursor::pos());
}

void LocalPointValueView::doShowPoint(LocalPointValue *localPointValue)
{
    trvWidget->clear();

    // point
    QTreeWidgetItem *pointNode = new QTreeWidgetItem(trvWidget);
    pointNode->setText(0, tr("Point"));
    pointNode->setExpanded(true);

    addValue(pointNode, m_scene->projectInfo().labelX() + ":", tr("%1").arg(localPointValue->point.x, 0, 'f', 5), tr("m"));
    addValue(pointNode, m_scene->projectInfo().labelY() + ":", tr("%1").arg(localPointValue->point.y, 0, 'f', 5), tr("m"));

    trvWidget->insertTopLevelItem(0, pointNode);

    if (localPointValue->scene()->sceneSolution()->sln())
    {
        if (LocalPointValueElectrostatic *localPointValueElectrostatic = dynamic_cast<LocalPointValueElectrostatic *>(localPointValue))
            showElectrostatic(localPointValueElectrostatic);
        if (LocalPointValueMagnetostatic *localPointValueMagnetostatic = dynamic_cast<LocalPointValueMagnetostatic *>(localPointValue))
            showMagnetostatic(localPointValueMagnetostatic);
        if (LocalPointValueHeat *localPointValueHeat = dynamic_cast<LocalPointValueHeat *>(localPointValue))
            showHeat(localPointValueHeat);
        if (LocalPointValueCurrent *localPointValueCurrent = dynamic_cast<LocalPointValueCurrent *>(localPointValue))
            showCurrent(localPointValueCurrent);
        if (LocalPointValueElasticity *localPointValueElasticity = dynamic_cast<LocalPointValueElasticity *>(localPointValue))
            showElasticity(localPointValueElasticity);
    }
}

void LocalPointValueView::showElectrostatic(LocalPointValueElectrostatic *localPointValueElectrostatic)
{
    // electrostatic
    QTreeWidgetItem *electrostaticNode = new QTreeWidgetItem(trvWidget);
    electrostaticNode->setText(0, tr("Electrostatic field"));
    electrostaticNode->setExpanded(true);

    // Permittivity
    addValue(electrostaticNode, tr("Permittivity:"), tr("%1").arg(localPointValueElectrostatic->permittivity, 0, 'f', 2), tr(""));

    // Charge Density
    addValue(electrostaticNode, tr("Charge density:"), tr("%1").arg(localPointValueElectrostatic->charge_density, 0, 'e', 3), tr("C/m3"));

    // Potential
    addValue(electrostaticNode, tr("Potential:"), tr("%1").arg(localPointValueElectrostatic->potential, 0, 'f', 2), tr("V"));

    // Electric Field
    QTreeWidgetItem *itemElectricField = new QTreeWidgetItem(electrostaticNode);
    itemElectricField->setText(0, tr("Electric field"));
    itemElectricField->setExpanded(true);

    addValue(itemElectricField, tr("Ex:"), tr("%1").arg(localPointValueElectrostatic->E.x, 0, 'f', 5), tr("V/m"));
    addValue(itemElectricField, tr("Ey:"), tr("%1").arg(localPointValueElectrostatic->E.y, 0, 'f', 5), tr("V/m"));
    addValue(itemElectricField, tr("E:"), tr("%1").arg(localPointValueElectrostatic->E.magnitude(), 0, 'f', 5), tr("V/m"));

    // Electric Displacement
    QTreeWidgetItem *itemElectricDisplacement = new QTreeWidgetItem(electrostaticNode);
    itemElectricDisplacement->setText(0, tr("Electric displacement"));
    itemElectricDisplacement->setExpanded(true);

    addValue(itemElectricDisplacement, tr("Dx:"), tr("%1").arg(localPointValueElectrostatic->D.x, 0, 'e', 3), tr("C/m2"));
    addValue(itemElectricDisplacement, tr("Dy:"), tr("%1").arg(localPointValueElectrostatic->D.y, 0, 'e', 3), tr("C/m2"));
    addValue(itemElectricDisplacement, tr("D:"), tr("%1").arg(localPointValueElectrostatic->D.magnitude(), 0, 'e', 3), tr("C/m2"));

    // Energy density
    addValue(electrostaticNode, tr("Energy density:"), tr("%1").arg(localPointValueElectrostatic->we, 0, 'e', 3), tr("J/m3"));
}

void LocalPointValueView::showMagnetostatic(LocalPointValueMagnetostatic *localPointValueMagnetostatic)
{
    // electrostatic
    QTreeWidgetItem *magnetostaticNode = new QTreeWidgetItem(trvWidget);
    magnetostaticNode->setText(0, tr("Magnetostatic field"));
    magnetostaticNode->setExpanded(true);

    // Permittivity
    addValue(magnetostaticNode, tr("Permeability:"), tr("%1").arg(localPointValueMagnetostatic->permeability, 0, 'f', 2), tr(""));

    // Charge Density
    addValue(magnetostaticNode, tr("Current density:"), tr("%1").arg(localPointValueMagnetostatic->current_density, 0, 'e', 3), tr("A/m2"));

    // Potential
    addValue(magnetostaticNode, tr("Vector potential:"), tr("%1").arg(localPointValueMagnetostatic->potential, 0, 'e', 2), tr("Wb/m"));

    // Flux Density
    QTreeWidgetItem *itemFluxDensity = new QTreeWidgetItem(magnetostaticNode);
    itemFluxDensity->setText(0, tr("Flux density"));
    itemFluxDensity->setExpanded(true);

    addValue(itemFluxDensity, tr("Bx:"), tr("%1").arg(localPointValueMagnetostatic->B.x, 0, 'f', 5), tr("T"));
    addValue(itemFluxDensity, tr("By:"), tr("%1").arg(localPointValueMagnetostatic->B.y, 0, 'f', 5), tr("T"));
    addValue(itemFluxDensity, tr("B:"), tr("%1").arg(localPointValueMagnetostatic->B.magnitude(), 0, 'f', 5), tr("T"));

    // Magnetic Field
    QTreeWidgetItem *itemMagneticField = new QTreeWidgetItem(magnetostaticNode);
    itemMagneticField->setText(0, tr("Magnetic field"));
    itemMagneticField->setExpanded(true);

    addValue(itemMagneticField, tr("Hx:"), tr("%1").arg(localPointValueMagnetostatic->H.x, 0, 'e', 3), tr("A/m"));
    addValue(itemMagneticField, tr("Hy:"), tr("%1").arg(localPointValueMagnetostatic->H.y, 0, 'e', 3), tr("A/m"));
    addValue(itemMagneticField, tr("H:"), tr("%1").arg(localPointValueMagnetostatic->H.magnitude(), 0, 'e', 3), tr("A/m"));

    // Energy density
    addValue(magnetostaticNode, tr("Energy density:"), tr("%1").arg(localPointValueMagnetostatic->wm, 0, 'e', 3), tr("J/m3"));
}

void LocalPointValueView::showHeat(LocalPointValueHeat *localPointValueHeat)
{
    // heat
    QTreeWidgetItem *heatNode = new QTreeWidgetItem(trvWidget);
    heatNode->setText(0, tr("Heat transfer"));
    heatNode->setExpanded(true);

    // Thermal Conductivity
    addValue(heatNode, tr("Thermal conductivity:"), tr("%1").arg(localPointValueHeat->thermal_conductivity, 0, 'f', 2), tr("W/m.K"));

    // Volume Heat
    addValue(heatNode, tr("Volume heat:"), tr("%1").arg(localPointValueHeat->volume_heat, 0, 'e', 3), tr("W/m3"));

    // Temperature
    addValue(heatNode, tr("Temperature:"), tr("%1").arg(localPointValueHeat->temperature, 0, 'f', 2), tr("deg."));

    // Heat Flux
    QTreeWidgetItem *itemHeatFlux = new QTreeWidgetItem(heatNode);
    itemHeatFlux->setText(0, tr("Heat flux"));
    itemHeatFlux->setExpanded(true);

    addValue(itemHeatFlux, tr("Fx:"), tr("%1").arg(localPointValueHeat->F.x, 0, 'e', 3), tr("W/m2"));
    addValue(itemHeatFlux, tr("Fy:"), tr("%1").arg(localPointValueHeat->F.y, 0, 'e', 3), tr("W/m2"));
    addValue(itemHeatFlux, tr("F:"), tr("%1").arg(localPointValueHeat->F.magnitude(), 0, 'e', 3), tr("W/m2"));

    // Temperature Gradient
    QTreeWidgetItem *itemTemperatureGradient = new QTreeWidgetItem(heatNode);
    itemTemperatureGradient->setText(0, tr("Temperature gradient"));
    itemTemperatureGradient->setExpanded(true);

    addValue(itemTemperatureGradient, tr("Gx:"), tr("%1").arg(localPointValueHeat->G.x, 0, 'f', 5), tr("K/m"));
    addValue(itemTemperatureGradient, tr("Gy:"), tr("%1").arg(localPointValueHeat->G.y, 0, 'f', 5), tr("K/m"));
    addValue(itemTemperatureGradient, tr("G:"), tr("%1").arg(localPointValueHeat->G.magnitude(), 0, 'f', 5), tr("K/m"));
}

void LocalPointValueView::showCurrent(LocalPointValueCurrent *localPointValueCurrent)
{
    // current field
    QTreeWidgetItem *currentNode = new QTreeWidgetItem(trvWidget);
    currentNode->setText(0, tr("Current field"));
    currentNode->setExpanded(true);

    // Conductivity
    addValue(currentNode, tr("Conductivity:"), tr("%1").arg(localPointValueCurrent->conductivity, 0, 'e', 3), tr("S/m"));

    // Potential
    addValue(currentNode, tr("Potential:"), tr("%1").arg(localPointValueCurrent->potential, 0, 'f', 2), tr("V"));

    // Electric Field
    QTreeWidgetItem *itemElectricField = new QTreeWidgetItem(currentNode);
    itemElectricField->setText(0, tr("Electric field"));
    itemElectricField->setExpanded(true);

    addValue(itemElectricField, tr("Ex:"), tr("%1").arg(localPointValueCurrent->E.x, 0, 'f', 5), tr("V/m"));
    addValue(itemElectricField, tr("Ey:"), tr("%1").arg(localPointValueCurrent->E.y, 0, 'f', 5), tr("V/m"));
    addValue(itemElectricField, tr("E:"), tr("%1").arg(localPointValueCurrent->E.magnitude(), 0, 'f', 5), tr("V/m"));

    // Current Density
    QTreeWidgetItem *itemCurrentDensity = new QTreeWidgetItem(currentNode);
    itemCurrentDensity->setText(0, tr("Current density"));
    itemCurrentDensity->setExpanded(true);

    addValue(itemCurrentDensity, tr("Jx:"), tr("%1").arg(localPointValueCurrent->J.x, 0, 'e', 3), tr("A/m2"));
    addValue(itemCurrentDensity, tr("Jy:"), tr("%1").arg(localPointValueCurrent->J.y, 0, 'e', 3), tr("A/m2"));
    addValue(itemCurrentDensity, tr("J:"), tr("%1").arg(localPointValueCurrent->J.magnitude(), 0, 'e', 3), tr("A/m2"));

    // Energy density
    addValue(currentNode, tr("Power losses dens.:"), tr("%1").arg(localPointValueCurrent->losses, 0, 'e', 3), tr("W/m3"));
}

void LocalPointValueView::showElasticity(LocalPointValueElasticity *localPointValueElasticity)
{
    // elasticity
    QTreeWidgetItem *heatNode = new QTreeWidgetItem(trvWidget);
    heatNode->setText(0, tr("Elasticity"));
    heatNode->setExpanded(true);

    // Young modulus
    addValue(heatNode, tr("Young modulus:"), tr("%1").arg(localPointValueElasticity->young_modulus, 0, 'e', 3), tr("Pa"));

    // Poisson ratio
    addValue(heatNode, tr("Poisson ratio:"), tr("%1").arg(localPointValueElasticity->poisson_ratio, 0, 'f', 3), tr(""));

    // Von Mises stress
    addValue(heatNode, tr("Von Mises stress:"), tr("%1").arg(localPointValueElasticity->von_mises_stress, 0, 'e', 3), tr("Pa"));
}

void LocalPointValueView::addValue(QTreeWidgetItem *parent, QString name, QString text, QString unit)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, name);
    item->setText(1, text);
    item->setTextAlignment(1, Qt::AlignRight);
    item->setText(2, unit + " ");
    item->setTextAlignment(2, Qt::AlignLeft);
}

LocalPointValue::LocalPointValue(Point &point, Scene *scene)
{
    this->m_scene = scene;
    this->point = point;
}

// ****************************************************************************************************************

LocalPointValueElectrostatic::LocalPointValueElectrostatic(Point &point, Scene *scene) : LocalPointValue(point, scene)
{
    charge_density = 0;
    permittivity = 0;

    potential = 0;
    E = Point();
    D = Point();
    we = 0;

    if (scene->sceneSolution()->sln())
    {
        PointValue value = scene->sceneSolution()->pointValue(point);       

        if (value.marker != NULL)
        {
            // potential
            potential = value.value;

            // electric field
            E = value.derivative * (-1);

            SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(value.marker);

            charge_density = marker->charge_density;
            permittivity = marker->permittivity;

            // electric displacement
            D = E * (marker->permittivity * EPS0);

            // energy density
            we = 0.5 * E.magnitude() * D.magnitude();
        }
    }
}

double LocalPointValueElectrostatic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_POTENTIAL:
        {
            return potential;
        }
        break;
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_ELECTRICFIELD:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return E.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return E.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return E.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_DISPLACEMENT:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return D.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return D.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return D.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_ENERGY_DENSITY:
        {
            return we;
        }
        break;
    case PHYSICFIELDVARIABLE_ELECTROSTATIC_PERMITTIVITY:
        {
            return permittivity;
        }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueElectrostatic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        break;
    }
}

QStringList LocalPointValueElectrostatic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(point.x, 0, 'e', 5) <<
            QString("%1").arg(point.y, 0, 'e', 5) <<
            QString("%1").arg(potential, 0, 'e', 5) <<
            QString("%1").arg(E.x, 0, 'e', 5) <<
            QString("%1").arg(E.y, 0, 'e', 5) <<
            QString("%1").arg(E.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(D.x, 0, 'e', 5) <<
            QString("%1").arg(D.y, 0, 'e', 5) <<
            QString("%1").arg(D.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(we, 0, 'e', 5) <<
            QString("%1").arg(permittivity, 0, 'f', 3);

    return QStringList(row);
}

// ****************************************************************************************************************

LocalPointValueMagnetostatic::LocalPointValueMagnetostatic(Point &point, Scene *scene) : LocalPointValue(point, scene)
{
    current_density = 0;
    permeability = 0;

    potential = 0;
    H = Point();
    B = Point();
    wm = 0;

    if (scene->sceneSolution()->sln())
    {
        PointValue value = scene->sceneSolution()->pointValue(point);
        if (value.marker != NULL)
        {
            // potential
            potential = value.value;

            // magnetic field
            Point der;
            der = value.derivative;

            if (m_scene->projectInfo().problemType == PROBLEMTYPE_PLANAR)
            {
                B.x =  der.y;
                B.y = -der.x;
            }
            else
            {
                B.x = -der.y;
                B.y =  der.x + ((point.x > 0.0) ? potential/point.x : 0.0);
            }

            SceneLabelMagnetostaticMarker *marker = dynamic_cast<SceneLabelMagnetostaticMarker *>(value.marker);

            current_density = marker->current_density;
            permeability = marker->permeability;

            // electric displacement
            H = B / (marker->permeability * MU0);

            // energy density
            wm = 0.5 * H.magnitude() * B.magnitude();
        }
    }
}

double LocalPointValueMagnetostatic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_VECTOR_POTENTIAL:
        {
            return potential;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_FLUX_DENSITY:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return B.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return B.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return B.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_MAGNETICFIELD:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return H.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return H.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return H.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_ENERGY_DENSITY:
        {
            return wm;
        }
        break;
    case PHYSICFIELDVARIABLE_MAGNETOSTATIC_PERMEABILITY:
        {
            return permeability;
        }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueMagnetostatic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        break;
    }
}

QStringList LocalPointValueMagnetostatic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(point.x, 0, 'e', 5) <<
            QString("%1").arg(point.y, 0, 'e', 5) <<
            QString("%1").arg(potential, 0, 'e', 5) <<
            QString("%1").arg(B.x, 0, 'e', 5) <<
            QString("%1").arg(B.y, 0, 'e', 5) <<
            QString("%1").arg(B.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(H.x, 0, 'e', 5) <<
            QString("%1").arg(H.y, 0, 'e', 5) <<
            QString("%1").arg(H.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(wm, 0, 'e', 5) <<
            QString("%1").arg(permeability, 0, 'f', 3);

    return QStringList(row);
}

// ****************************************************************************************************************

LocalPointValueHeat::LocalPointValueHeat(Point &point, Scene *scene) : LocalPointValue(point, scene)
{
    thermal_conductivity = 0;
    volume_heat = 0;

    temperature = 0;
    G = Point();
    F = Point();

    if (scene->sceneSolution()->sln())
    {
        PointValue value = scene->sceneSolution()->pointValue(point);
        if (value.marker != NULL)
        {
            // temperature
            temperature = value.value;

            // temperature gradient
            G = value.derivative * (-1);

            SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(value.marker);

            thermal_conductivity = marker->thermal_conductivity;
            volume_heat = marker->volume_heat;

            // heat flux
            F = G * marker->thermal_conductivity;
        }
    }
}

double LocalPointValueHeat::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE:
        {
            return temperature;
        }
        break;
    case PHYSICFIELDVARIABLE_HEAT_TEMPERATURE_GRADIENT:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return G.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return G.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return G.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_HEAT_FLUX:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return F.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return F.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return F.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_HEAT_CONDUCTIVITY:
        {
            return thermal_conductivity;
        }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueHeat::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        break;
    }
}

QStringList LocalPointValueHeat::variables()
{
    QStringList row;
    row <<  QString("%1").arg(point.x, 0, 'e', 5) <<
            QString("%1").arg(point.y, 0, 'e', 5) <<
            QString("%1").arg(temperature, 0, 'e', 5) <<
            QString("%1").arg(G.x, 0, 'e', 5) <<
            QString("%1").arg(G.y, 0, 'e', 5) <<
            QString("%1").arg(G.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(F.x, 0, 'e', 5) <<
            QString("%1").arg(F.y, 0, 'e', 5) <<
            QString("%1").arg(F.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(thermal_conductivity, 0, 'f', 3);

    return QStringList(row);
}

// ****************************************************************************************************************

LocalPointValueCurrent::LocalPointValueCurrent(Point &point, Scene *scene) : LocalPointValue(point, scene)
{
    conductivity = 0;

    potential = 0;
    J = Point();
    E = Point();
    losses = 0;

    if (scene->sceneSolution()->sln())
    {
        PointValue value = scene->sceneSolution()->pointValue(point);
        if (value.marker != NULL)
        {
            // potential
            potential = value.value;

            // magnetic field
            Point der;
            der = value.derivative;

            E.x =  der.y;
            E.y = -der.x;

            SceneLabelCurrentMarker *marker = dynamic_cast<SceneLabelCurrentMarker *>(value.marker);

            conductivity = marker->conductivity;

            // electric displacement
            J = E * marker->conductivity;

            // energy density
            losses = J.magnitude() * E.magnitude();
        }
    }
}

double LocalPointValueCurrent::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PHYSICFIELDVARIABLE_CURRENT_POTENTIAL:
        {
            return potential;
        }
        break;
    case PHYSICFIELDVARIABLE_CURRENT_ELECTRICFIELD:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return E.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return E.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return E.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_CURRENT_CURRENT_DENSITY:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return J.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return J.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return J.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_CURRENT_LOSSES:
        {
            return losses;
        }
        break;
    case PHYSICFIELDVARIABLE_CURRENT_CONDUCTIVITY:
        {
            return conductivity;
        }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueCurrent::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        break;
    }
}

QStringList LocalPointValueCurrent::variables()
{
    QStringList row;
    row <<  QString("%1").arg(point.x, 0, 'e', 5) <<
            QString("%1").arg(point.y, 0, 'e', 5) <<
            QString("%1").arg(potential, 0, 'e', 5) <<
            QString("%1").arg(J.x, 0, 'e', 5) <<
            QString("%1").arg(J.y, 0, 'e', 5) <<
            QString("%1").arg(J.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(E.x, 0, 'e', 5) <<
            QString("%1").arg(E.y, 0, 'e', 5) <<
            QString("%1").arg(E.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(losses, 0, 'e', 5) <<
            QString("%1").arg(conductivity, 0, 'f', 3);

    return QStringList(row);
}

// ****************************************************************************************************************

LocalPointValueElasticity::LocalPointValueElasticity(Point &point, Scene *scene) : LocalPointValue(point, scene)
{
    if (scene->sceneSolution()->sln())
    {
        von_mises_stress = 0;
        // G = Point();
        // F = Point();

        PointValue value = scene->sceneSolution()->pointValue(point);
        if (value.marker != NULL)
        {
            // Von Mises stress
            von_mises_stress = value.value;

            // temperature gradient
            // G = value.derivative * (-1);

            SceneLabelElasticityMarker *marker = dynamic_cast<SceneLabelElasticityMarker *>(value.marker);

            young_modulus = marker->young_modulus;
            poisson_ratio = marker->poisson_ratio;

            // heat flux
            // F = G * marker->thermal_conductivity;
        }
    }
}

double LocalPointValueElasticity::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PHYSICFIELDVARIABLE_ELASTICITY_VON_MISES_STRESS:
        {
            return von_mises_stress;
        }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueHeat::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        break;
    }
}

QStringList LocalPointValueElasticity::variables()
{
    QStringList row;
    row << QString("%1").arg(von_mises_stress, 0, 'e', 5);

    return QStringList(row);
}
// ***********************************************************************************************************************

LocalPointValue *localPointValueFactory(Point &point, Scene *scene)
{
    switch (scene->projectInfo().physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        return new LocalPointValueElectrostatic(point, scene);
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        return new LocalPointValueMagnetostatic(point, scene);
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        return new LocalPointValueHeat(point, scene);
        break;
    case PHYSICFIELD_CURRENT:
        return new LocalPointValueCurrent(point, scene);
        break;
    case PHYSICFIELD_ELASTICITY:
        return new LocalPointValueElasticity(point, scene);
        break;
    default:
        cerr << "Physical field '" + physicFieldStringKey(scene->projectInfo().physicField).toStdString() + "' is not implemented. LocalPointValueView *localPointValueFactory(Point &point, Scene *scene)" << endl;
        throw;
        break;
    }
}

QStringList localPointValueHeaderFactory(PhysicField physicField)
{
    QStringList headers;
    switch (physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        headers << "X" << "Y" << "Potential" << "Ex" << "Ey" << "E" << "Dx" << "Dy" << "D" << "Energy" << "Permittivity";
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        headers << "X" << "Y" << "Potential" << "Bx" << "By" << "B" << "Hx" << "Hy" << "H" << "Energy" << "Permeability";
        break;
    case PHYSICFIELD_CURRENT:
        headers << "X" << "Y" << "Potential" << "Jx" << "Jy" << "J" << "Ex" << "Ey" << "E" << "Losses" << "Conductivity";
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        headers << "X" << "Y" << "Temperature" << "Gx" << "Gy" << "G" << "Fx" << "Fy" << "F" << "Conductivity";
        break;
    case PHYSICFIELD_ELASTICITY:
        headers << "X" << "Y" << "Von Misses stress";
        break;
    default:
        cerr << "Physical field '" + physicFieldStringKey(physicField).toStdString() + "' is not implemented. localPointValueHeaderFactory(PhysicField physicField)" << endl;
        throw;
        break;
    }

    return QStringList(headers);
}
