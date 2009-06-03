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
    actPoint = new QAction(getIcon("scenenode"), tr("Local point value"), this);
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

    addValue(pointNode, tr("x:"), tr("%1").arg(localPointValue->point.x, 0, 'f', 5), tr("m"));
    addValue(pointNode, tr("y:"), tr("%1").arg(localPointValue->point.y, 0, 'f', 5), tr("m"));

    trvWidget->insertTopLevelItem(0, pointNode);

    if (localPointValue->scene()->sceneSolution()->sln())
    {
        if (LocalPointValueElectrostatic *localPointValueElectrostatic = dynamic_cast<LocalPointValueElectrostatic *>(localPointValue))
            showElectrostatic(localPointValueElectrostatic);
        if (LocalPointValueMagnetostatic *localPointValueMagnetostatic = dynamic_cast<LocalPointValueMagnetostatic *>(localPointValue))
            showMagnetostatic(localPointValueMagnetostatic);
        if (LocalPointValueHeat *localPointValueHeat = dynamic_cast<LocalPointValueHeat *>(localPointValue))
            showHeat(localPointValueHeat);
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

QString LocalPointValue::toString()
{
    return QString::number(point.x, 'f', 5) + "; " + QString::number(point.y, 'f', 5);
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

QString LocalPointValueElectrostatic::toString()
{
    return QString::number(point.x, 'f', 5) + "; " + QString::number(point.y, 'f', 5);
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

QString LocalPointValueMagnetostatic::toString()
{
    return QString::number(point.x, 'f', 5) + "; " + QString::number(point.y, 'f', 5);
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

QString LocalPointValueHeat::toString()
{
    return QString::number(point.x, 'f', 5) + "; " + QString::number(point.y, 'f', 5);
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

QString LocalPointValueElasticity::toString()
{
    return QString::number(point.x, 'f', 5) + "; " + QString::number(point.y, 'f', 5);
}

// ***********************************************************************************************************************

LocalPointValue *localPointValueFactory(Point &point, Scene *scene)
{

    switch (scene->projectInfo().physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        // electrostatic
        return new LocalPointValueElectrostatic(point, scene);
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        // electrostatic
        return new LocalPointValueMagnetostatic(point, scene);
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        // heat transfer
        return new LocalPointValueHeat(point, scene);
        break;
    case PHYSICFIELD_ELASTICITY:
        // elasticity transfer
        return new LocalPointValueElasticity(point, scene);
        break;
    default:
        cerr << "Physical field '" + scene->projectInfo().physicFieldString().toStdString() + "' is not implemented. LocalPointValueView *localPointValueFactory(Point &point, Scene *scene)" << endl;
        throw;
        break;
    }
}
