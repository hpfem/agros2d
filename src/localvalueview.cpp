#include "localvalueview.h"

LocalPointValueView::LocalPointValueView(QWidget *parent): QDockWidget(tr("Local Values"), parent)
{
    setMinimumWidth(280);
    setObjectName("LocalPointValueView");

    createActions();
    createMenu();

    trvWidget = new QTreeWidget();
    trvWidget->setHeaderHidden(false);
    trvWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    trvWidget->setMouseTracking(true);
    trvWidget->setColumnCount(3);
    trvWidget->setColumnWidth(0, 180);
    trvWidget->setColumnWidth(1, 80);
    trvWidget->setColumnWidth(2, 20);

    QStringList labels;
    labels << tr("Label") << tr("Value") << tr("Unit");
    trvWidget->setHeaderLabels(labels);

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
    if (node->showDialog(this) == QDialog::Accepted)
    {
        doShowPoint(localPointValueFactory(node->point));
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

    addValue(pointNode, Util::scene()->problemInfo().labelX() + ":", tr("%1").arg(localPointValue->point.x, 0, 'f', 5), tr("m"));
    addValue(pointNode, Util::scene()->problemInfo().labelY() + ":", tr("%1").arg(localPointValue->point.y, 0, 'f', 5), tr("m"));

    trvWidget->insertTopLevelItem(0, pointNode);

    if (Util::scene()->sceneSolution()->sln())
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
        if (LocalPointValueHarmonicMagnetic *localPointValueHarmonicMagnetic = dynamic_cast<LocalPointValueHarmonicMagnetic *>(localPointValue))
            showHarmonicMagnetic(localPointValueHarmonicMagnetic);
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
    addValue(electrostaticNode, tr("Charge density:"), tr("%1").arg(localPointValueElectrostatic->charge_density, 0, 'e', 3), "C/m3");

    // Energy density
    addValue(electrostaticNode, tr("Energy density:"), tr("%1").arg(localPointValueElectrostatic->we, 0, 'e', 3), "J/m3");

    // Potential
    addValue(electrostaticNode, tr("Potential:"), tr("%1").arg(localPointValueElectrostatic->potential, 0, 'e', 3), "V");

    // Electric Field
    QTreeWidgetItem *itemElectricField = new QTreeWidgetItem(electrostaticNode);
    itemElectricField->setText(0, tr("Electric field"));
    itemElectricField->setExpanded(true);

    addValue(itemElectricField, "E" + Util::scene()->problemInfo().labelX().toLower() + ":", tr("%1").arg(localPointValueElectrostatic->E.x, 0, 'f', 5), "V/m");
    addValue(itemElectricField, "E" + Util::scene()->problemInfo().labelY().toLower() + ":", tr("%1").arg(localPointValueElectrostatic->E.y, 0, 'f', 5), "V/m");
    addValue(itemElectricField, "E", tr("%1").arg(localPointValueElectrostatic->E.magnitude(), 0, 'f', 5), "V/m");

    // Electric Displacement
    QTreeWidgetItem *itemElectricDisplacement = new QTreeWidgetItem(electrostaticNode);
    itemElectricDisplacement->setText(0, tr("Electric displacement"));
    itemElectricDisplacement->setExpanded(true);

    addValue(itemElectricDisplacement, "D" + Util::scene()->problemInfo().labelX().toLower() + ":", tr("%1").arg(localPointValueElectrostatic->D.x, 0, 'e', 3), "C/m2");
    addValue(itemElectricDisplacement, "D" + Util::scene()->problemInfo().labelY().toLower() + ":", tr("%1").arg(localPointValueElectrostatic->D.y, 0, 'e', 3), "C/m2");
    addValue(itemElectricDisplacement, "D:", tr("%1").arg(localPointValueElectrostatic->D.magnitude(), 0, 'e', 3), "C/m2");
}

void LocalPointValueView::showMagnetostatic(LocalPointValueMagnetostatic *localPointValueMagnetostatic)
{
    // electrostatic
    QTreeWidgetItem *magnetostaticNode = new QTreeWidgetItem(trvWidget);
    magnetostaticNode->setText(0, tr("Magnetostatic field"));
    magnetostaticNode->setExpanded(true);

    // Permeability
    addValue(magnetostaticNode, tr("Permeability:"), tr("%1").arg(localPointValueMagnetostatic->permeability, 0, 'f', 2), "");

    // Current Density
    addValue(magnetostaticNode, tr("Current density:"), tr("%1").arg(localPointValueMagnetostatic->current_density, 0, 'e', 3), "A/m2");

    // Energy density
    addValue(magnetostaticNode, tr("Energy density:"), tr("%1").arg(localPointValueMagnetostatic->wm, 0, 'e', 3), "J/m3");

    // Potential
    addValue(magnetostaticNode, tr("Vector potential:"), tr("%1").arg(localPointValueMagnetostatic->potential, 0, 'e', 3), "Wb/m");

    // Flux Density
    QTreeWidgetItem *itemFluxDensity = new QTreeWidgetItem(magnetostaticNode);
    itemFluxDensity->setText(0, tr("Flux density"));
    itemFluxDensity->setExpanded(true);

    addValue(itemFluxDensity, "B" + Util::scene()->problemInfo().labelX().toLower() + ":", tr("%1").arg(localPointValueMagnetostatic->B.x, 0, 'e', 3), "T");
    addValue(itemFluxDensity, "B" + Util::scene()->problemInfo().labelY().toLower() + ":", tr("%1").arg(localPointValueMagnetostatic->B.y, 0, 'e', 3), "T");
    addValue(itemFluxDensity, "B:", tr("%1").arg(localPointValueMagnetostatic->B.magnitude(), 0, 'e', 3), "T");

    // Magnetic Field
    QTreeWidgetItem *itemMagneticField = new QTreeWidgetItem(magnetostaticNode);
    itemMagneticField->setText(0, tr("Magnetic field"));
    itemMagneticField->setExpanded(true);

    addValue(itemMagneticField, "H" + Util::scene()->problemInfo().labelX().toLower() + ":", tr("%1").arg(localPointValueMagnetostatic->H.x, 0, 'e', 3), "A/m");
    addValue(itemMagneticField, "H" + Util::scene()->problemInfo().labelY().toLower() + ":", tr("%1").arg(localPointValueMagnetostatic->H.y, 0, 'e', 3), "A/m");
    addValue(itemMagneticField, "H", tr("%1").arg(localPointValueMagnetostatic->H.magnitude(), 0, 'e', 3), "A/m");
}

void LocalPointValueView::showHarmonicMagnetic(LocalPointValueHarmonicMagnetic *localPointValueHarmonicMagnetic)
{
    // harmonic magnetic
    QTreeWidgetItem *harmonicMagneticNode = new QTreeWidgetItem(trvWidget);
    harmonicMagneticNode->setText(0, tr("Harmonic magnetic field"));
    harmonicMagneticNode->setExpanded(true);

    // Permittivity
    addValue(harmonicMagneticNode, tr("Permeability:"), tr("%1").arg(localPointValueHarmonicMagnetic->permeability, 0, 'f', 2), "");

    // Current Density
    QTreeWidgetItem *itemCurrentDensity = new QTreeWidgetItem(harmonicMagneticNode);
    itemCurrentDensity->setText(0, tr("External current density"));
    itemCurrentDensity->setExpanded(true);

    addValue(itemCurrentDensity, tr("real:"), tr("%1").arg(localPointValueHarmonicMagnetic->current_density_real, 0, 'e', 3), "A/m2");
    addValue(itemCurrentDensity, tr("imag:"), tr("%1").arg(localPointValueHarmonicMagnetic->current_density_imag, 0, 'e', 3), "A/m2");
    addValue(itemCurrentDensity, tr("magnitude:"), tr("%1").arg(sqrt(sqr(localPointValueHarmonicMagnetic->current_density_real) + sqr(localPointValueHarmonicMagnetic->current_density_imag)), 0, 'e', 3), "A/m2");

    // Average power losses
    addValue(harmonicMagneticNode, tr("Average power losses dens.:"), tr("%1").arg(localPointValueHarmonicMagnetic->pj, 0, 'e', 3), "W/m3");

    // Energy density
    addValue(harmonicMagneticNode, tr("Energy density:"), tr("%1").arg(localPointValueHarmonicMagnetic->wm, 0, 'e', 3), "J/m3");

    // Potential
    QTreeWidgetItem *itemPotential = new QTreeWidgetItem(harmonicMagneticNode);
    itemPotential->setText(0, tr("Vector potential"));
    itemPotential->setExpanded(true);

    addValue(itemPotential, tr("real:"), tr("%1").arg(localPointValueHarmonicMagnetic->potential_real, 0, 'e', 3), "Wb/m");
    addValue(itemPotential, tr("imag:"), tr("%1").arg(localPointValueHarmonicMagnetic->potential_imag, 0, 'e', 3), "Wb/m");
    addValue(itemPotential, tr("magnitude:"), tr("%1").arg(sqrt(sqr(localPointValueHarmonicMagnetic->potential_real) + sqr(localPointValueHarmonicMagnetic->potential_imag)), 0, 'e', 3), "Wb/m");

    // Flux Density
    addValue(harmonicMagneticNode, "Flux density:", tr("%1").arg(sqrt(sqr(localPointValueHarmonicMagnetic->B_real.x) + sqr(localPointValueHarmonicMagnetic->B_imag.x) + sqr(localPointValueHarmonicMagnetic->B_real.y) + sqr(localPointValueHarmonicMagnetic->B_imag.y)), 0, 'e', 3), "T");

    // Flux Density - real
    QTreeWidgetItem *itemFluxDensityReal = new QTreeWidgetItem(harmonicMagneticNode);
    itemFluxDensityReal->setText(0, tr("Flux density - real"));
    itemFluxDensityReal->setExpanded(false);

    addValue(itemFluxDensityReal, "B" + Util::scene()->problemInfo().labelX().toLower() + ":", tr("%1").arg(localPointValueHarmonicMagnetic->B_real.x, 0, 'e', 3), "T");
    addValue(itemFluxDensityReal, "B" + Util::scene()->problemInfo().labelY().toLower() + ":", tr("%1").arg(localPointValueHarmonicMagnetic->B_real.y, 0, 'e', 3), "T");
    addValue(itemFluxDensityReal, "B:", tr("%1").arg(localPointValueHarmonicMagnetic->B_real.magnitude(), 0, 'e', 3), "T");

    // Flux Density - imag
    QTreeWidgetItem *itemFluxDensityImag = new QTreeWidgetItem(harmonicMagneticNode);
    itemFluxDensityImag->setText(0, tr("Flux density - imag"));
    itemFluxDensityImag->setExpanded(false);

    addValue(itemFluxDensityImag, "B" + Util::scene()->problemInfo().labelX().toLower() + ":", tr("%1").arg(localPointValueHarmonicMagnetic->B_imag.x, 0, 'e', 3), "T");
    addValue(itemFluxDensityImag, "B" + Util::scene()->problemInfo().labelY().toLower() + ":", tr("%1").arg(localPointValueHarmonicMagnetic->B_imag.y, 0, 'e', 3), "T");
    addValue(itemFluxDensityImag, "B:", tr("%1").arg(localPointValueHarmonicMagnetic->B_imag.magnitude(), 0, 'e', 3), "T");

    // Magnetic Field
    addValue(harmonicMagneticNode, "Magnetic field:", tr("%1").arg(sqrt(sqr(localPointValueHarmonicMagnetic->H_real.x) + sqr(localPointValueHarmonicMagnetic->H_imag.x) + sqr(localPointValueHarmonicMagnetic->H_real.y) + sqr(localPointValueHarmonicMagnetic->H_imag.y)), 0, 'e', 3), "A/m");

    // Magnetic Field - real
    QTreeWidgetItem *itemMagneticFieldReal = new QTreeWidgetItem(harmonicMagneticNode);
    itemMagneticFieldReal->setText(0, tr("Magnetic field - real"));
    itemMagneticFieldReal->setExpanded(false);

    addValue(itemMagneticFieldReal, "H" + Util::scene()->problemInfo().labelX().toLower() + ":", tr("%1").arg(localPointValueHarmonicMagnetic->H_real.x, 0, 'e', 3), "A/m");
    addValue(itemMagneticFieldReal, "H" + Util::scene()->problemInfo().labelY().toLower() + ":", tr("%1").arg(localPointValueHarmonicMagnetic->H_real.y, 0, 'e', 3), "A/m");
    addValue(itemMagneticFieldReal, "H", tr("%1").arg(localPointValueHarmonicMagnetic->H_real.magnitude(), 0, 'e', 3), "A/m");

    // Magnetic Field - imag
    QTreeWidgetItem *itemMagneticFieldImag = new QTreeWidgetItem(harmonicMagneticNode);
    itemMagneticFieldImag->setText(0, tr("Magnetic field - imag"));
    itemMagneticFieldImag->setExpanded(false);

    addValue(itemMagneticFieldImag, "H" + Util::scene()->problemInfo().labelX().toLower() + ":", tr("%1").arg(localPointValueHarmonicMagnetic->H_imag.x, 0, 'e', 3), "A/m");
    addValue(itemMagneticFieldImag, "H" + Util::scene()->problemInfo().labelY().toLower() + ":", tr("%1").arg(localPointValueHarmonicMagnetic->H_imag.y, 0, 'e', 3), "A/m");
    addValue(itemMagneticFieldImag, "H", tr("%1").arg(localPointValueHarmonicMagnetic->H_imag.magnitude(), 0, 'e', 3), "A/m");

    // Total current density
    addValue(harmonicMagneticNode, tr("Total current density:"), tr("%1").arg(sqrt(sqr(localPointValueHarmonicMagnetic->current_density_total_real) + sqr(localPointValueHarmonicMagnetic->current_density_total_imag)), 0, 'e', 3), "A/m2");

    QTreeWidgetItem *itemCurrentDensityTotal = new QTreeWidgetItem(harmonicMagneticNode);
    itemCurrentDensityTotal->setText(0, tr("Total current density"));
    itemCurrentDensityTotal->setExpanded(false);

    addValue(itemCurrentDensityTotal, tr("real:"), tr("%1").arg(localPointValueHarmonicMagnetic->current_density_total_real, 0, 'e', 3), "A/m2");
    addValue(itemCurrentDensityTotal, tr("imag:"), tr("%1").arg(localPointValueHarmonicMagnetic->current_density_total_imag, 0, 'e', 3), "A/m2");

    // Induced current density
    addValue(harmonicMagneticNode, tr("Induced current density:"), tr("%1").arg(sqrt(sqr(localPointValueHarmonicMagnetic->current_density_induced_real) + sqr(localPointValueHarmonicMagnetic->current_density_induced_imag)), 0, 'e', 3), "A/m2");

    QTreeWidgetItem *itemCurrentDensityInduced = new QTreeWidgetItem(harmonicMagneticNode);
    itemCurrentDensityInduced->setText(0, tr("Induced current density"));
    itemCurrentDensityInduced->setExpanded(false);

    addValue(itemCurrentDensityInduced, tr("real:"), tr("%1").arg(localPointValueHarmonicMagnetic->current_density_induced_real, 0, 'e', 3), "A/m2");
    addValue(itemCurrentDensityInduced, tr("imag:"), tr("%1").arg(localPointValueHarmonicMagnetic->current_density_induced_imag, 0, 'e', 3), "A/m2");

    // Magnetic Field
    addValue(harmonicMagneticNode, "Lorentz force:", tr("%1").arg(sqrt(sqr(localPointValueHarmonicMagnetic->FL_real.x) + sqr(localPointValueHarmonicMagnetic->FL_imag.x) + sqr(localPointValueHarmonicMagnetic->FL_real.y) + sqr(localPointValueHarmonicMagnetic->FL_imag.y)), 0, 'e', 3), "A/m");

    // Lorentz force - real
    QTreeWidgetItem *itemLorentzForceReal = new QTreeWidgetItem(harmonicMagneticNode);
    itemLorentzForceReal->setText(0, tr("Lorentz force - real"));
    itemLorentzForceReal->setExpanded(false);

    addValue(itemLorentzForceReal, "FL" + Util::scene()->problemInfo().labelX().toLower() + ":", tr("%1").arg(localPointValueHarmonicMagnetic->FL_real.x, 0, 'e', 3), "N/m3");
    addValue(itemLorentzForceReal, "FL" + Util::scene()->problemInfo().labelY().toLower() + ":", tr("%1").arg(localPointValueHarmonicMagnetic->FL_real.y, 0, 'e', 3), "N/m3");
    addValue(itemLorentzForceReal, "FL", tr("%1").arg(localPointValueHarmonicMagnetic->FL_real.magnitude(), 0, 'e', 3), "N/m3");

    // Lorentz force - imag
    QTreeWidgetItem *itemLorentzForceImag = new QTreeWidgetItem(harmonicMagneticNode);
    itemLorentzForceImag->setText(0, tr("Lorentz force - imag"));
    itemLorentzForceImag->setExpanded(false);

    addValue(itemLorentzForceImag, "FL" + Util::scene()->problemInfo().labelX().toLower() + ":", tr("%1").arg(localPointValueHarmonicMagnetic->FL_imag.x, 0, 'e', 3), "N/m3");
    addValue(itemLorentzForceImag, "FL" + Util::scene()->problemInfo().labelY().toLower() + ":", tr("%1").arg(localPointValueHarmonicMagnetic->FL_imag.y, 0, 'e', 3), "N/m3");
    addValue(itemLorentzForceImag, "FL", tr("%1").arg(localPointValueHarmonicMagnetic->FL_imag.magnitude(), 0, 'e', 3), "N/m3");
}

void LocalPointValueView::showHeat(LocalPointValueHeat *localPointValueHeat)
{
    // heat
    QTreeWidgetItem *heatNode = new QTreeWidgetItem(trvWidget);
    heatNode->setText(0, tr("Heat transfer"));
    heatNode->setExpanded(true);

    // Thermal Conductivity
    addValue(heatNode, tr("Thermal conductivity:"), tr("%1").arg(localPointValueHeat->thermal_conductivity, 0, 'f', 2), "W/m.K");

    // Volume Heat
    addValue(heatNode, tr("Volume heat:"), tr("%1").arg(localPointValueHeat->volume_heat, 0, 'e', 3), "W/m3");

    // Temperature
    addValue(heatNode, tr("Temperature:"), tr("%1").arg(localPointValueHeat->temperature, 0, 'f', 2), "deg.");

    // Heat Flux
    QTreeWidgetItem *itemHeatFlux = new QTreeWidgetItem(heatNode);
    itemHeatFlux->setText(0, tr("Heat flux"));
    itemHeatFlux->setExpanded(true);

    addValue(itemHeatFlux, "F" + Util::scene()->problemInfo().labelX().toLower() + ":", tr("%1").arg(localPointValueHeat->F.x, 0, 'e', 3), "W/m2");
    addValue(itemHeatFlux, "F" + Util::scene()->problemInfo().labelY().toLower() + ":", tr("%1").arg(localPointValueHeat->F.y, 0, 'e', 3), "W/m2");
    addValue(itemHeatFlux, "F:", tr("%1").arg(localPointValueHeat->F.magnitude(), 0, 'e', 3), "W/m2");

    // Temperature Gradient
    QTreeWidgetItem *itemTemperatureGradient = new QTreeWidgetItem(heatNode);
    itemTemperatureGradient->setText(0, tr("Temperature gradient"));
    itemTemperatureGradient->setExpanded(true);

    addValue(itemTemperatureGradient, "G" + Util::scene()->problemInfo().labelX().toLower() + ":", tr("%1").arg(localPointValueHeat->G.x, 0, 'f', 5), "K/m");
    addValue(itemTemperatureGradient, "G" + Util::scene()->problemInfo().labelY().toLower() + ":", tr("%1").arg(localPointValueHeat->G.y, 0, 'f', 5), "K/m");
    addValue(itemTemperatureGradient, "G:", tr("%1").arg(localPointValueHeat->G.magnitude(), 0, 'f', 5), "K/m");
}

void LocalPointValueView::showCurrent(LocalPointValueCurrent *localPointValueCurrent)
{
    // current field
    QTreeWidgetItem *currentNode = new QTreeWidgetItem(trvWidget);
    currentNode->setText(0, tr("Current field"));
    currentNode->setExpanded(true);

    // Conductivity
    addValue(currentNode, tr("Conductivity:"), tr("%1").arg(localPointValueCurrent->conductivity, 0, 'e', 3), "S/m");

    // Energy density
    addValue(currentNode, tr("Power losses dens.:"), tr("%1").arg(localPointValueCurrent->losses, 0, 'e', 3), tr("W/m3"));

    // Potential
    addValue(currentNode, tr("Potential:"), tr("%1").arg(localPointValueCurrent->potential, 0, 'f', 2), "V");

    // Electric Field
    QTreeWidgetItem *itemElectricField = new QTreeWidgetItem(currentNode);
    itemElectricField->setText(0, tr("Electric field"));
    itemElectricField->setExpanded(true);

    addValue(itemElectricField, "E" + Util::scene()->problemInfo().labelX().toLower() + ":", tr("%1").arg(localPointValueCurrent->E.x, 0, 'f', 5), "V/m");
    addValue(itemElectricField, "E" + Util::scene()->problemInfo().labelY().toLower() + ":", tr("%1").arg(localPointValueCurrent->E.y, 0, 'f', 5), "V/m");
    addValue(itemElectricField, "E:", tr("%1").arg(localPointValueCurrent->E.magnitude(), 0, 'f', 5), "V/m");

    // Current Density
    QTreeWidgetItem *itemCurrentDensity = new QTreeWidgetItem(currentNode);
    itemCurrentDensity->setText(0, tr("Current density"));
    itemCurrentDensity->setExpanded(true);

    addValue(itemCurrentDensity, "J" + Util::scene()->problemInfo().labelX().toLower() + ":", tr("%1").arg(localPointValueCurrent->J.x, 0, 'e', 3), "A/m2");
    addValue(itemCurrentDensity, "J" + Util::scene()->problemInfo().labelY().toLower() + ":", tr("%1").arg(localPointValueCurrent->J.y, 0, 'e', 3), "A/m2");
    addValue(itemCurrentDensity, "J:", tr("%1").arg(localPointValueCurrent->J.magnitude(), 0, 'e', 3), "A/m2");
}

void LocalPointValueView::showElasticity(LocalPointValueElasticity *localPointValueElasticity)
{
    // elasticity
    QTreeWidgetItem *heatNode = new QTreeWidgetItem(trvWidget);
    heatNode->setText(0, tr("Elasticity"));
    heatNode->setExpanded(true);

    // Young modulus
    addValue(heatNode, tr("Young modulus:"), tr("%1").arg(localPointValueElasticity->young_modulus, 0, 'e', 3), "Pa");

    // Poisson ratio
    addValue(heatNode, tr("Poisson ratio:"), tr("%1").arg(localPointValueElasticity->poisson_ratio, 0, 'f', 3), "");

    // Von Mises stress
    addValue(heatNode, tr("Von Mises stress:"), tr("%1").arg(localPointValueElasticity->von_mises_stress, 0, 'e', 3), "Pa");
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

LocalPointValue::LocalPointValue(Point &point)
{
    this->point = point;
}

// ****************************************************************************************************************

LocalPointValueElectrostatic::LocalPointValueElectrostatic(Point &point) : LocalPointValue(point)
{
    charge_density = 0;
    permittivity = 0;

    potential = 0;
    E = Point();
    D = Point();
    we = 0;

    if (Util::scene()->sceneSolution()->sln())
    {
        PointValue value = Util::scene()->sceneSolution()->pointValue(point, Util::scene()->sceneSolution()->sln1());

        if (value.marker != NULL)
        {
            // potential
            potential = value.value;

            // electric field
            E = value.derivative * (-1);

            SceneLabelElectrostaticMarker *marker = dynamic_cast<SceneLabelElectrostaticMarker *>(value.marker);

            charge_density = marker->charge_density.number;
            permittivity = marker->permittivity.number;

            // electric displacement
            D = E * (marker->permittivity.number * EPS0);

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

LocalPointValueMagnetostatic::LocalPointValueMagnetostatic(Point &point) : LocalPointValue(point)
{
    current_density = 0;
    permeability = 0;

    potential = 0;
    H = Point();
    B = Point();
    wm = 0;

    if (Util::scene()->sceneSolution()->sln())
    {
        PointValue value = Util::scene()->sceneSolution()->pointValue(point, Util::scene()->sceneSolution()->sln1());
        if (value.marker != NULL)
        {
            // potential
            potential = value.value;

            // magnetic field
            Point der;
            der = value.derivative;

            if (Util::scene()->problemInfo().problemType == PROBLEMTYPE_PLANAR)
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

            current_density = marker->current_density.number;
            permeability = marker->permeability.number;

            // electric displacement
            H = B / (marker->permeability.number * MU0);

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

LocalPointValueHarmonicMagnetic::LocalPointValueHarmonicMagnetic(Point &point) : LocalPointValue(point)
{
    current_density_real = 0;
    current_density_imag = 0;
    permeability = 0;

    potential_real = 0;
    potential_imag = 0;
    H_real = Point();
    H_imag = Point();
    B_real = Point();
    B_imag = Point();
    wm = 0;

    if (Util::scene()->sceneSolution()->sln())
    {
        PointValue valueReal = Util::scene()->sceneSolution()->pointValue(point, Util::scene()->sceneSolution()->sln1());
        PointValue valueImag = Util::scene()->sceneSolution()->pointValue(point, Util::scene()->sceneSolution()->sln2());
        if (valueReal.marker != NULL)
        {
            // potential
            potential_real = valueReal.value;
            potential_imag = valueImag.value;

            // magnetic field
            Point derReal;
            derReal = valueReal.derivative;

            if (Util::scene()->problemInfo().problemType == PROBLEMTYPE_PLANAR)
            {
                B_real.x =  derReal.y;
                B_real.y = -derReal.x;
            }
            else
            {
                B_real.x = -derReal.y;
                B_real.y =  derReal.x + ((point.x > 0.0) ? potential_real/point.x : 0.0);
            }

            Point derImag;
            derImag = valueImag.derivative;

            if (Util::scene()->problemInfo().problemType == PROBLEMTYPE_PLANAR)
            {
                B_imag.x =  derImag.y;
                B_imag.y = -derImag.x;
            }
            else
            {
                B_imag.x = -derImag.y;
                B_imag.y =  derImag.x + ((point.x > 0.0) ? potential_imag/point.x : 0.0);
            }

            SceneLabelHarmonicMagneticMarker *marker = dynamic_cast<SceneLabelHarmonicMagneticMarker *>(valueReal.marker);
            permeability = marker->permeability.number;

            // current density
            current_density_real = marker->current_density_real.number;
            current_density_imag = marker->current_density_imag.number;

            // induced current density
            current_density_induced_real =   2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * potential_imag;
            current_density_induced_imag = - 2 * M_PI * Util::scene()->problemInfo().frequency * marker->conductivity.number * potential_real;

            // total current density
            current_density_total_real = current_density_real + current_density_induced_real;
            current_density_total_imag = current_density_imag + current_density_induced_imag;

            // electric displacement
            H_real = B_real / (marker->permeability.number * MU0);
            H_imag = B_imag / (marker->permeability.number * MU0);

            // Lorentz force
            FL_real.x = - (current_density_total_real*B_real.y - current_density_total_imag*B_imag.y);
            FL_real.y =   (current_density_total_real*B_real.x - current_density_total_imag*B_imag.x);
            FL_imag.x = - (current_density_total_imag*B_real.y + current_density_total_real*B_imag.y);
            FL_imag.y =   (current_density_total_imag*B_real.x + current_density_total_real*B_imag.x);

            // power losses
            pj = (marker->conductivity.number > 0.0) ?
                            0.5 / marker->conductivity.number * (sqr(current_density_total_real) + sqr(current_density_total_imag)) : 0.0;

            // energy density
            wm = 0.25 * (sqr(B_real.x) + sqr(B_real.y) + sqr(B_imag.x) + sqr(B_imag.y)) / (marker->permeability.number * MU0);
        }
    }
}

double LocalPointValueHarmonicMagnetic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariable)
    {
    case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_VECTOR_POTENTIAL:
        {
            return sqrt(sqr(potential_real) + sqr(potential_imag));
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_VECTOR_POTENTIAL_REAL:
        {
            return potential_real;
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_VECTOR_POTENTIAL_IMAG:
        {
            return potential_imag;
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_FLUX_DENSITY:
        {
           return sqrt(sqr(B_real.x) + sqr(B_imag.x) + sqr(B_real.y) + sqr(B_imag.y));
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_FLUX_DENSITY_REAL:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return B_real.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return B_real.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return B_real.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_FLUX_DENSITY_IMAG:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return B_imag.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return B_imag.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return B_imag.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_MAGNETICFIELD:
        {
           return sqrt(sqr(H_real.x) + sqr(H_imag.x) + sqr(H_real.y) + sqr(H_imag.y));
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_MAGNETICFIELD_REAL:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return H_real.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return H_real.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return H_real.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_MAGNETICFIELD_IMAG:
        {
            switch (physicFieldVariableComp)
            {
            case PHYSICFIELDVARIABLECOMP_X:
                return H_imag.x;
                break;
            case PHYSICFIELDVARIABLECOMP_Y:
                return H_imag.y;
                break;
            case PHYSICFIELDVARIABLECOMP_MAGNITUDE:
                return H_imag.magnitude();
                break;
            }
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL:
        {
            return sqrt(sqr(current_density_total_real) + sqr(current_density_total_imag));
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL_REAL:
        {
            return current_density_total_real;
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL_IMAG:
        {
            return current_density_total_imag;
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED:
        {
            return sqrt(sqr(current_density_induced_real) + sqr(current_density_induced_imag));
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED_REAL:
        {
            return current_density_induced_real;
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED_IMAG:
        {
            return current_density_induced_imag;
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_POWER_LOSSES:
        {
            return pj;
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_ENERGY_DENSITY:
        {
            return wm;
        }
        break;
    case PHYSICFIELDVARIABLE_HARMONIC_MAGNETIC_PERMEABILITY:
        {
            return permeability;
        }
        break;
    default:
        cerr << "Physical field variable '" + physicFieldVariableString(physicFieldVariable).toStdString() + "' is not implemented. LocalPointValueHarmonicMagnetic::variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp)" << endl;
        throw;
        break;
    }
}

QStringList LocalPointValueHarmonicMagnetic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(point.x, 0, 'e', 5) <<
            QString("%1").arg(point.y, 0, 'e', 5) <<
            QString("%1").arg(potential_real, 0, 'e', 5) <<
            QString("%1").arg(potential_imag, 0, 'e', 5) <<
            QString("%1").arg(sqr(potential_real) + sqr(potential_imag), 0, 'e', 5) <<
            QString("%1").arg(sqrt(sqr(B_real.x) + sqr(B_imag.x) + sqr(B_real.y) + sqr(B_imag.y)), 0, 'e', 5) <<
            QString("%1").arg(B_real.x, 0, 'e', 5) <<
            QString("%1").arg(B_real.y, 0, 'e', 5) <<
            QString("%1").arg(B_real.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(B_imag.x, 0, 'e', 5) <<
            QString("%1").arg(B_imag.y, 0, 'e', 5) <<
            QString("%1").arg(B_imag.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(sqrt(sqr(H_real.x) + sqr(H_imag.x) + sqr(H_real.y) + sqr(H_imag.y)), 0, 'e', 5) <<
            QString("%1").arg(H_real.x, 0, 'e', 5) <<
            QString("%1").arg(H_real.y, 0, 'e', 5) <<
            QString("%1").arg(H_real.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(H_imag.x, 0, 'e', 5) <<
            QString("%1").arg(H_imag.y, 0, 'e', 5) <<
            QString("%1").arg(H_imag.magnitude(), 0, 'e', 5) <<
            QString("%1").arg(current_density_induced_real, 0, 'e', 5) <<
            QString("%1").arg(current_density_induced_real, 0, 'e', 5) <<
            QString("%1").arg(sqrt(sqr(current_density_induced_real) + sqr(current_density_induced_imag)), 0, 'e', 5) <<
            QString("%1").arg(current_density_total_real, 0, 'e', 5) <<
            QString("%1").arg(current_density_total_real, 0, 'e', 5) <<
            QString("%1").arg(sqrt(sqr(current_density_total_real) + sqr(current_density_total_imag)), 0, 'e', 5) <<
            QString("%1").arg(pj, 0, 'e', 5) <<
            QString("%1").arg(wm, 0, 'e', 5) <<
            QString("%1").arg(permeability, 0, 'f', 3);

    return QStringList(row);
}

// ****************************************************************************************************************

LocalPointValueHeat::LocalPointValueHeat(Point &point) : LocalPointValue(point)
{
    thermal_conductivity = 0;
    volume_heat = 0;

    temperature = 0;
    G = Point();
    F = Point();

    if (Util::scene()->sceneSolution()->sln())
    {
        PointValue value = Util::scene()->sceneSolution()->pointValue(point, Util::scene()->sceneSolution()->sln1());
        if (value.marker != NULL)
        {
            // temperature
            temperature = value.value;

            // temperature gradient
            G = value.derivative * (-1);

            SceneLabelHeatMarker *marker = dynamic_cast<SceneLabelHeatMarker *>(value.marker);

            thermal_conductivity = marker->thermal_conductivity.number;
            volume_heat = marker->volume_heat.number;

            // heat flux
            F = G * marker->thermal_conductivity.number;
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

LocalPointValueCurrent::LocalPointValueCurrent(Point &point) : LocalPointValue(point)
{
    conductivity = 0;

    potential = 0;
    J = Point();
    E = Point();
    losses = 0;

    if (Util::scene()->sceneSolution()->sln())
    {
        PointValue value = Util::scene()->sceneSolution()->pointValue(point, Util::scene()->sceneSolution()->sln1());
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

            conductivity = marker->conductivity.number;

            // electric displacement
            J = E * marker->conductivity.number;

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

LocalPointValueElasticity::LocalPointValueElasticity(Point &point) : LocalPointValue(point)
{
    if (Util::scene()->sceneSolution()->sln())
    {
        von_mises_stress = 0;
        // G = Point();
        // F = Point();

        PointValue value = Util::scene()->sceneSolution()->pointValue(point, Util::scene()->sceneSolution()->sln1());
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

LocalPointValue *localPointValueFactory(Point &point)
{
    switch (Util::scene()->problemInfo().physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        return new LocalPointValueElectrostatic(point);
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        return new LocalPointValueMagnetostatic(point);
        break;
    case PHYSICFIELD_HARMONIC_MAGNETIC:
        return new LocalPointValueHarmonicMagnetic(point);
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        return new LocalPointValueHeat(point);
        break;
    case PHYSICFIELD_CURRENT:
        return new LocalPointValueCurrent(point);
        break;
    case PHYSICFIELD_ELASTICITY:
        return new LocalPointValueElasticity(point);
        break;
    default:
        cerr << "Physical field '" + physicFieldStringKey(Util::scene()->problemInfo().physicField).toStdString() + "' is not implemented. LocalPointValueView *localPointValueFactory(Point &point)" << endl;
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
    case PHYSICFIELD_HARMONIC_MAGNETIC:
        headers << "X" << "Y" << "Potential_real" << "Potential_imag" << "Potential"
                << "B" << "Bx_real" << "By_real" << "B_real" << "Bx_imag" << "By_imag" << "B_imag"
                << "H" << "Hx_real" << "Hy_real" << "H_real" << "Hx_imag" << "Hy_imag" << "H_imag"
                << "Ji_real" << "Ji_imag" << "Ji_real"
                << "J_real" << "J_imag" << "J_real"
                << "Losses" << "Energy" << "Permeability";
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
