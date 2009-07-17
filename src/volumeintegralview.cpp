#include "volumeintegralview.h"

VolumeIntegralValueView::VolumeIntegralValueView(QWidget *parent): QDockWidget(tr("Volume Integral"), parent)
{
    setMinimumWidth(280);
    setObjectName("VolumeIntegralValueView");

    trvWidget = new QTreeWidget();
    trvWidget->setHeaderHidden(false);
    trvWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    trvWidget->setMouseTracking(true);
    trvWidget->setColumnCount(3);
    trvWidget->setColumnWidth(0, 150);
    trvWidget->setColumnWidth(1, 80);
    trvWidget->setColumnWidth(2, 20);

    QStringList labels;
    labels << tr("Label") << tr("Number") << "Unit";
    trvWidget->setHeaderLabels(labels);

    setWidget(trvWidget);
}

void VolumeIntegralValueView::doShowVolumeIntegral(VolumeIntegralValue *volumeIntegralValue)
{
    trvWidget->clear();

    // point
    QTreeWidgetItem *pointGeometry = new QTreeWidgetItem(trvWidget);
    pointGeometry->setText(0, tr("Geometry"));
    pointGeometry->setExpanded(true);

    addValue(pointGeometry, tr("Volume:"), tr("%1").arg(volumeIntegralValue->volume, 0, 'e', 3), tr("m3"));
    addValue(pointGeometry, tr("Cross section:"), tr("%1").arg(volumeIntegralValue->crossSection, 0, 'e', 3), tr("m2"));

    trvWidget->insertTopLevelItem(0, pointGeometry);

    if (Util::scene()->sceneSolution()->isSolved())
    {
        if (VolumeIntegralValueElectrostatic *volumeIntegralValueElectrostatic = dynamic_cast<VolumeIntegralValueElectrostatic *>(volumeIntegralValue))
            showElectrostatic(volumeIntegralValueElectrostatic);
        if (VolumeIntegralValueMagnetostatic *volumeIntegralValueMagnetostatic = dynamic_cast<VolumeIntegralValueMagnetostatic *>(volumeIntegralValue))
            showMagnetostatic(volumeIntegralValueMagnetostatic);
        if (VolumeIntegralValueHarmonicMagnetic *volumeIntegralValueHarmonicMagnetic = dynamic_cast<VolumeIntegralValueHarmonicMagnetic *>(volumeIntegralValue))
            showHarmonicMagnetic(volumeIntegralValueHarmonicMagnetic);
        if (VolumeIntegralValueHeat *volumeIntegralValueHeat = dynamic_cast<VolumeIntegralValueHeat *>(volumeIntegralValue))
            showHeat(volumeIntegralValueHeat);
    }
}

void VolumeIntegralValueView::showElectrostatic(VolumeIntegralValueElectrostatic *volumeIntegralValueElectrostatic)
{
    // electrostatic
    QTreeWidgetItem *electrostaticNode = new QTreeWidgetItem(trvWidget);
    electrostaticNode->setText(0, tr("Electrostatic field"));
    electrostaticNode->setExpanded(true);

    addValue(electrostaticNode, tr("Ex avg.:"), tr("%1").arg(volumeIntegralValueElectrostatic->averageElectricFieldX, 0, 'e', 3), tr("V/m"));
    addValue(electrostaticNode, tr("Ey avg.:"), tr("%1").arg(volumeIntegralValueElectrostatic->averageElectricFieldY, 0, 'e', 3), tr("V/m"));
    addValue(electrostaticNode, tr("E avg.:"), tr("%1").arg(volumeIntegralValueElectrostatic->averageElectricField, 0, 'e', 3), tr("V/m"));
    addValue(electrostaticNode, tr("Dx avg.:"), tr("%1").arg(volumeIntegralValueElectrostatic->averageDisplacementX, 0, 'e', 3), tr("C/m2"));
    addValue(electrostaticNode, tr("Dy avg.:"), tr("%1").arg(volumeIntegralValueElectrostatic->averageDisplacementY, 0, 'e', 3), tr("C/m2"));
    addValue(electrostaticNode, tr("D avg.:"), tr("%1").arg(volumeIntegralValueElectrostatic->averageDisplacement, 0, 'e', 3), tr("C/m2"));
    addValue(electrostaticNode, tr("Energy:"), tr("%1").arg(volumeIntegralValueElectrostatic->energy, 0, 'e', 3), tr("J"));
}

void VolumeIntegralValueView::showMagnetostatic(VolumeIntegralValueMagnetostatic *volumeIntegralValueMagnetostatic)
{
    // electrostatic
    QTreeWidgetItem *magnetostaticNode = new QTreeWidgetItem(trvWidget);
    magnetostaticNode->setText(0, tr("Magnetostatic field"));
    magnetostaticNode->setExpanded(true);

    addValue(magnetostaticNode, tr("Hx avg.:"), tr("%1").arg(volumeIntegralValueMagnetostatic->averageMagneticFieldX, 0, 'e', 3), tr("A/m"));
    addValue(magnetostaticNode, tr("Hy avg.:"), tr("%1").arg(volumeIntegralValueMagnetostatic->averageMagneticFieldY, 0, 'e', 3), tr("A/m"));
    addValue(magnetostaticNode, tr("H: avg."), tr("%1").arg(volumeIntegralValueMagnetostatic->averageMagneticField, 0, 'e', 3), tr("A/m"));
    addValue(magnetostaticNode, tr("Bx avg.:"), tr("%1").arg(volumeIntegralValueMagnetostatic->averageFluxDensityX, 0, 'e', 3), tr("T"));
    addValue(magnetostaticNode, tr("By avg.:"), tr("%1").arg(volumeIntegralValueMagnetostatic->averageFluxDensityY, 0, 'e', 3), tr("T"));
    addValue(magnetostaticNode, tr("B avg.:"), tr("%1").arg(volumeIntegralValueMagnetostatic->averageFluxDensity, 0, 'e', 3), tr("T"));
    addValue(magnetostaticNode, tr("Energy:"), tr("%1").arg(volumeIntegralValueMagnetostatic->energy, 0, 'e', 3), tr("J"));
}

void VolumeIntegralValueView::showHarmonicMagnetic(VolumeIntegralValueHarmonicMagnetic *volumeIntegralValueHarmonicMagnetic)
{
    // harmonic magnetic
    QTreeWidgetItem *harmonicMagneticNode = new QTreeWidgetItem(trvWidget);
    harmonicMagneticNode->setText(0, tr("Harmonic magnetic field"));
    harmonicMagneticNode->setExpanded(true);

    // total current
    QTreeWidgetItem *itemCurrentTotal = new QTreeWidgetItem(harmonicMagneticNode);
    itemCurrentTotal->setText(0, tr("Total current"));
    itemCurrentTotal->setExpanded(true);

    addValue(itemCurrentTotal, tr("real:"), tr("%1").arg(volumeIntegralValueHarmonicMagnetic->currentTotalReal, 0, 'e', 3), "A");
    addValue(itemCurrentTotal, tr("imag:"), tr("%1").arg(volumeIntegralValueHarmonicMagnetic->currentTotalImag, 0, 'e', 3), "A");
    addValue(itemCurrentTotal, tr("magnitude:"), tr("%1").arg(sqrt(sqr(volumeIntegralValueHarmonicMagnetic->currentTotalReal) + sqr(volumeIntegralValueHarmonicMagnetic->currentTotalImag)), 0, 'e', 3), "A");

    // induced current
    QTreeWidgetItem *itemCurrentInduced = new QTreeWidgetItem(harmonicMagneticNode);
    itemCurrentInduced->setText(0, tr("Induced current"));
    itemCurrentInduced->setExpanded(true);

    addValue(itemCurrentInduced, tr("real:"), tr("%1").arg(volumeIntegralValueHarmonicMagnetic->currentInducedReal, 0, 'e', 3), "A");
    addValue(itemCurrentInduced, tr("imag:"), tr("%1").arg(volumeIntegralValueHarmonicMagnetic->currentInducedImag, 0, 'e', 3), "A");
    addValue(itemCurrentInduced, tr("magnitude:"), tr("%1").arg(sqrt(sqr(volumeIntegralValueHarmonicMagnetic->currentInducedReal) + sqr(volumeIntegralValueHarmonicMagnetic->currentInducedImag)), 0, 'e', 3), "A");

    addValue(harmonicMagneticNode, tr("Power losses avg.:"), tr("%1").arg(volumeIntegralValueHarmonicMagnetic->powerLosses, 0, 'e', 3), tr("W"));
    addValue(harmonicMagneticNode, tr("Energy avg.:"), tr("%1").arg(volumeIntegralValueHarmonicMagnetic->energy, 0, 'e', 3), tr("J"));
}

void VolumeIntegralValueView::showHeat(VolumeIntegralValueHeat *volumeIntegralValueHeat)
{
    // heat
    QTreeWidgetItem *heatNode = new QTreeWidgetItem(trvWidget);
    heatNode->setText(0, tr("Heat transfer"));
    heatNode->setExpanded(true);

    addValue(heatNode, tr("Temperature:"), tr("%1").arg(volumeIntegralValueHeat->averageTemperature, 0, 'e', 3), tr("deg."));
    addValue(heatNode, tr("Gx avg.:"), tr("%1").arg(volumeIntegralValueHeat->averageTemperatureGradientX, 0, 'e', 3), tr("K/m"));
    addValue(heatNode, tr("Gy avg.:"), tr("%1").arg(volumeIntegralValueHeat->averageTemperatureGradientY, 0, 'e', 3), tr("K/m"));
    addValue(heatNode, tr("G avg.:"), tr("%1").arg(volumeIntegralValueHeat->averageTemperatureGradient, 0, 'e', 3), tr("K/m"));
    addValue(heatNode, tr("Fx avg.:"), tr("%1").arg(volumeIntegralValueHeat->averageHeatFluxX, 0, 'e', 3), tr("W/m2"));
    addValue(heatNode, tr("Fy avg.:"), tr("%1").arg(volumeIntegralValueHeat->averageHeatFluxY, 0, 'e', 3), tr("W/m2"));
    addValue(heatNode, tr("F avg.:"), tr("%1").arg(volumeIntegralValueHeat->averageHeatFlux, 0, 'e', 3), tr("W/m2"));
}

void VolumeIntegralValueView::addValue(QTreeWidgetItem *parent, QString name, QString text, QString unit)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, name);
    item->setText(1, text);
    item->setTextAlignment(1, Qt::AlignRight);
    item->setText(2, unit + " ");
    item->setTextAlignment(2, Qt::AlignLeft);
}

VolumeIntegralValue::VolumeIntegralValue()
{
    crossSection = 0;
    volume = 0;
    for (int i = 0; i<Util::scene()->labels.length(); i++)
    {
        if (Util::scene()->labels[i]->isSelected)
        {
            crossSection += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_CROSSSECTION);
            volume += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_VOLUME);
        }
    }
}

// ****************************************************************************************************************

VolumeIntegralValueElectrostatic::VolumeIntegralValueElectrostatic() : VolumeIntegralValue()
{
    if (Util::scene()->sceneSolution()->isSolved())
    {
        averageElectricFieldX = 0;
        averageElectricFieldY = 0;
        averageElectricField = 0;
        averageDisplacementX = 0;
        averageDisplacementY = 0;
        averageDisplacement = 0;
        energy = 0;
        for (int i = 0; i<Util::scene()->labels.length(); i++)
        {
            if (Util::scene()->labels[i]->isSelected)
            {
                averageElectricFieldX += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ELECTRICFIELD_X);
                averageElectricFieldY += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ELECTRICFIELD_Y);
                averageElectricField += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ELECTRICFIELD);
                averageDisplacementX += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_DISPLACEMENT_X);
                averageDisplacementY += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_DISPLACEMENT_Y);
                averageDisplacement += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_DISPLACEMENT);
                energy += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ENERGY_DENSITY);
            }
        }

        if (volume > 0)
        {
            averageElectricFieldX /= volume;
            averageElectricFieldY /= volume;
            averageElectricField /= volume;
            averageDisplacementX /= volume;
            averageDisplacementY /= volume;
            averageDisplacement /= volume;
        }
    }
}

QStringList VolumeIntegralValueElectrostatic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
            QString("%1").arg(crossSection, 0, 'e', 5) <<
            QString("%1").arg(averageElectricFieldX, 0, 'e', 5) <<
            QString("%1").arg(averageElectricFieldY, 0, 'e', 5) <<
            QString("%1").arg(averageElectricField, 0, 'e', 5) <<
            QString("%1").arg(averageDisplacementX, 0, 'e', 5) <<
            QString("%1").arg(averageDisplacementY, 0, 'e', 5) <<
            QString("%1").arg(averageDisplacement, 0, 'e', 5) <<
            QString("%1").arg(energy, 0, 'e', 5);
    return QStringList(row);
}

// ****************************************************************************************************************

VolumeIntegralValueMagnetostatic::VolumeIntegralValueMagnetostatic() : VolumeIntegralValue()
{
    if (Util::scene()->sceneSolution()->isSolved())
    {
        averageMagneticFieldX = 0;
        averageMagneticFieldY = 0;
        averageMagneticField = 0;
        averageFluxDensityX = 0;
        averageFluxDensityY = 0;
        averageFluxDensity = 0;
        energy = 0;
        for (int i = 0; i<Util::scene()->labels.length(); i++)
        {
            if (Util::scene()->labels[i]->isSelected)
            {
                averageMagneticFieldX += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_MAGNETICFIELD_X);
                averageMagneticFieldY += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_MAGNETICFIELD_Y);
                averageMagneticField += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_MAGNETICFIELD);
                averageFluxDensityX += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_FLUX_DENSITY_X);
                averageFluxDensityY += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_FLUX_DENSITY_Y);
                averageFluxDensity += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_FLUX_DENSITY);
                energy += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_ENERGY_DENSITY);
            }
        }

        if (volume > 0)
        {
            averageMagneticFieldX /= volume;
            averageMagneticFieldY /= volume;
            averageMagneticField /= volume;
            averageFluxDensityX /= volume;
            averageFluxDensityY /= volume;
            averageFluxDensity /= volume;
        }
    }
}

QStringList VolumeIntegralValueMagnetostatic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
            QString("%1").arg(crossSection, 0, 'e', 5) <<
            QString("%1").arg(averageMagneticFieldX, 0, 'e', 5) <<
            QString("%1").arg(averageMagneticFieldY, 0, 'e', 5) <<
            QString("%1").arg(averageMagneticField, 0, 'e', 5) <<
            QString("%1").arg(averageFluxDensityX, 0, 'e', 5) <<
            QString("%1").arg(averageFluxDensityY, 0, 'e', 5) <<
            QString("%1").arg(averageFluxDensity, 0, 'e', 5) <<
            QString("%1").arg(energy, 0, 'e', 5);
    return QStringList(row);
}

// ****************************************************************************************************************

VolumeIntegralValueHarmonicMagnetic::VolumeIntegralValueHarmonicMagnetic() : VolumeIntegralValue()
{
    if (Util::scene()->sceneSolution()->isSolved())
    {
        currentInducedReal = 0;
        currentInducedImag = 0;
        currentTotalReal = 0;
        currentTotalImag = 0;
        powerLosses = 0;
        energy = 0;
        for (int i = 0; i<Util::scene()->labels.length(); i++)
        {
            if (Util::scene()->labels[i]->isSelected)
            {
                currentInducedReal += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED_REAL);
                currentInducedImag += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_CURRENT_DENSITY_INDUCED_IMAG);
                currentTotalReal += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL_REAL);
                currentTotalImag += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_CURRENT_DENSITY_TOTAL_IMAG);
                powerLosses += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_POWER_LOSSES);
                energy += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HARMONIC_MAGNETIC_ENERGY_DENSITY);
            }
        }

        if (volume > 0)
        {
        }
    }
}

QStringList VolumeIntegralValueHarmonicMagnetic::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
            QString("%1").arg(powerLosses, 0, 'e', 5) <<
            QString("%1").arg(energy, 0, 'e', 5);
    return QStringList(row);
}

// ****************************************************************************************************************

VolumeIntegralValueHeat::VolumeIntegralValueHeat() : VolumeIntegralValue()
{
    if (Util::scene()->sceneSolution()->isSolved())
    {
        averageTemperature = 0;
        averageTemperatureGradientX = 0;
        averageTemperatureGradientY = 0;
        averageTemperatureGradient = 0;
        averageHeatFluxX = 0;
        averageHeatFluxY = 0;
        averageHeatFlux = 0;
        for (int i = 0; i<Util::scene()->labels.length(); i++)
        {
            if (Util::scene()->labels[i]->isSelected)
            {
                averageTemperature += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE);
                averageTemperatureGradientX += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE_GRADIENT_X);
                averageTemperatureGradientY += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE_GRADIENT_Y);
                averageTemperatureGradient += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE_GRADIENT);
                averageHeatFluxX += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HEAT_FLUX_X);
                averageHeatFluxY += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HEAT_FLUX_Y);
                averageHeatFlux += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HEAT_FLUX);
            }
        }

        if (volume > 0)
        {
            averageTemperature /= volume;
            averageTemperatureGradientX /= volume;
            averageTemperatureGradientY /= volume;
            averageTemperatureGradient /= volume;
            averageHeatFluxX /= volume;
            averageHeatFluxY /= volume;
            averageHeatFlux /= volume;
        }
    }
}

QStringList VolumeIntegralValueHeat::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
            QString("%1").arg(crossSection, 0, 'e', 5) <<
            QString("%1").arg(averageTemperature, 0, 'e', 5) <<
            QString("%1").arg(averageTemperatureGradientX, 0, 'e', 5) <<
            QString("%1").arg(averageTemperatureGradientY, 0, 'e', 5) <<
            QString("%1").arg(averageTemperatureGradient, 0, 'e', 5) <<
            QString("%1").arg(averageHeatFluxX, 0, 'e', 5) <<
            QString("%1").arg(averageHeatFluxY, 0, 'e', 5) <<
            QString("%1").arg(averageHeatFlux, 0, 'e', 5);
    return QStringList(row);
}

// ****************************************************************************************************************

VolumeIntegralValueCurrent::VolumeIntegralValueCurrent() : VolumeIntegralValue()
{
    if (Util::scene()->sceneSolution()->isSolved())
    {
        averageElectricFieldX = 0;
        averageElectricFieldY = 0;
        averageElectricField = 0;
        averageCurrentDensityX = 0;
        averageCurrentDensityY = 0;
        averageCurrentDensity = 0;
        losses = 0;
        for (int i = 0; i<Util::scene()->labels.length(); i++)
        {
            if (Util::scene()->labels[i]->isSelected)
            {
                averageElectricFieldX += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_CURRENT_ELECTRICFIELD_X);
                averageElectricFieldY += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_CURRENT_ELECTRICFIELD_Y);
                averageElectricField += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_CURRENT_ELECTRICFIELD);
                averageCurrentDensityX += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_CURRENT_CURRENT_DENSITY_X);
                averageCurrentDensityY += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_CURRENT_CURRENT_DENSITY_Y);
                averageCurrentDensity += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_CURRENT_CURRENT_DENSITY);
                losses += Util::scene()->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_CURRENT_LOSSES);
            }
        }

        if (volume > 0)
        {
            averageElectricFieldX /= volume;
            averageElectricFieldY /= volume;
            averageElectricField /= volume;
            averageCurrentDensityX /= volume;
            averageCurrentDensityY /= volume;
            averageCurrentDensity /= volume;
        }
    }
}

QStringList VolumeIntegralValueCurrent::variables()
{
    QStringList row;
    row <<  QString("%1").arg(volume, 0, 'e', 5) <<
            QString("%1").arg(crossSection, 0, 'e', 5) <<
            QString("%1").arg(averageElectricFieldX, 0, 'e', 5) <<
            QString("%1").arg(averageElectricFieldY, 0, 'e', 5) <<
            QString("%1").arg(averageElectricField, 0, 'e', 5) <<
            QString("%1").arg(averageCurrentDensityX, 0, 'e', 5) <<
            QString("%1").arg(averageCurrentDensityY, 0, 'e', 5) <<
            QString("%1").arg(averageCurrentDensity, 0, 'e', 5);
    return QStringList(row);
}

// ***********************************************************************************************************************

VolumeIntegralValue *volumeIntegralValueFactory()
{
    switch (Util::scene()->projectInfo().physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        return new VolumeIntegralValueElectrostatic();
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        return new VolumeIntegralValueMagnetostatic();
        break;
    case PHYSICFIELD_HARMONIC_MAGNETIC:
        return new VolumeIntegralValueHarmonicMagnetic();
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        return new VolumeIntegralValueHeat();
        break;
    case PHYSICFIELD_CURRENT:
        return new VolumeIntegralValueCurrent();
        break;
    default:
        cerr << "Physical field '" + physicFieldStringKey(Util::scene()->projectInfo().physicField).toStdString() + "' is not implemented. VolumeIntegralValue *volumeIntegralValueFactory()" << endl;
        throw;
        break;
    }
}

QStringList volumeIntegralValueHeaderFactory(PhysicField physicField)
{
    QStringList headers;
    switch (physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        headers << "Volume" << "CrossSection" << "Ex_avg" << "Ey_avg" << "E_avg" << "Dx_avg" << "Dy_avg" << "D_avg" << "Energy";
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        headers << "Volume" << "CrossSection" << "Bx_avg" << "By_avg" << "B_avg" << "Hx_avg" << "Hy_avg" << "H_avg" << "Energy";
        break;
    case PHYSICFIELD_CURRENT:
        headers << "Volume" << "CrossSection" << "Jx_avg" << "Jy_avg" << "J_avg" << "Ex_avg" << "Ey_avg" << "E_avg" << "Losses";
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        headers << "Volume" << "CrossSection" << "Temperature" << "Gx_avg" << "Gy_avg" << "G_avg" << "Fx_avg" << "Fy_avg" << "F_avg";
        break;
    case PHYSICFIELD_ELASTICITY:
        headers << "Volume" << "CrossSection";
        break;
    default:
        cerr << "Physical field '" + physicFieldStringKey(physicField).toStdString() + "' is not implemented. volumeIntegralValueHeaderFactory(PhysicField physicField)" << endl;
        throw;
        break;
    }

    return QStringList(headers);
}
