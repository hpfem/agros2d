#include "volumeintegralview.h"

VolumeIntegralValueView::VolumeIntegralValueView(QWidget *parent): QDockWidget(tr("Volume Integral"), parent)
{
    setMinimumWidth(280);
    setObjectName("VolumeIntegralValueView");

    trvWidget = new QTreeWidget();
    trvWidget->setHeaderHidden(true);
    trvWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    trvWidget->setMouseTracking(true);
    trvWidget->setColumnCount(3);
    trvWidget->setColumnWidth(0, 150);
    trvWidget->setColumnWidth(1, 80);
    trvWidget->setColumnWidth(2, 20);

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
    addValue(pointGeometry, tr("Cross Section:"), tr("%1").arg(volumeIntegralValue->cross_section, 0, 'e', 3), tr("m2"));

    trvWidget->insertTopLevelItem(0, pointGeometry);

    if (volumeIntegralValue->scene()->sceneSolution()->sln())
    {
        if (VolumeIntegralValueElectrostatic2D *volumeIntegralValueElectrostatic2D = dynamic_cast<VolumeIntegralValueElectrostatic2D *>(volumeIntegralValue))
            showElectrostatic2D(volumeIntegralValueElectrostatic2D);
        if (VolumeIntegralValueMagnetostatic2D *volumeIntegralValueMagnetostatic2D = dynamic_cast<VolumeIntegralValueMagnetostatic2D *>(volumeIntegralValue))
            showMagnetostatic2D(volumeIntegralValueMagnetostatic2D);
        if (VolumeIntegralValueHeat2D *volumeIntegralValueHeat2D = dynamic_cast<VolumeIntegralValueHeat2D *>(volumeIntegralValue))
            showHeat2D(volumeIntegralValueHeat2D);
    }
}

void VolumeIntegralValueView::showElectrostatic2D(VolumeIntegralValueElectrostatic2D *volumeIntegralValueElectrostatic2D)
{
    // electrostatic
    QTreeWidgetItem *electrostaticNode = new QTreeWidgetItem(trvWidget);
    electrostaticNode->setText(0, tr("Electrostatic Field"));
    electrostaticNode->setExpanded(true);

    addValue(electrostaticNode, tr("Ex avg.:"), tr("%1").arg(volumeIntegralValueElectrostatic2D->averageElectricFieldX, 0, 'e', 3), tr("V/m"));
    addValue(electrostaticNode, tr("Ey avg.:"), tr("%1").arg(volumeIntegralValueElectrostatic2D->averageElectricFieldY, 0, 'e', 3), tr("V/m"));
    addValue(electrostaticNode, tr("E avg.:"), tr("%1").arg(volumeIntegralValueElectrostatic2D->averageElectricField, 0, 'e', 3), tr("V/m"));
    addValue(electrostaticNode, tr("Dx avg.:"), tr("%1").arg(volumeIntegralValueElectrostatic2D->averageDisplacementX, 0, 'e', 3), tr("C/m2"));
    addValue(electrostaticNode, tr("Dy avg.:"), tr("%1").arg(volumeIntegralValueElectrostatic2D->averageDisplacementY, 0, 'e', 3), tr("C/m2"));
    addValue(electrostaticNode, tr("D avg.:"), tr("%1").arg(volumeIntegralValueElectrostatic2D->averageDisplacement, 0, 'e', 3), tr("C/m2"));
    addValue(electrostaticNode, tr("Energy:"), tr("%1").arg(volumeIntegralValueElectrostatic2D->energy, 0, 'e', 3), tr("J"));
}

void VolumeIntegralValueView::showMagnetostatic2D(VolumeIntegralValueMagnetostatic2D *volumeIntegralValueMagnetostatic2D)
{
    // electrostatic
    QTreeWidgetItem *magnetostaticNode = new QTreeWidgetItem(trvWidget);
    magnetostaticNode->setText(0, tr("Magnetostatic Field"));
    magnetostaticNode->setExpanded(true);

    addValue(magnetostaticNode, tr("Hx avg.:"), tr("%1").arg(volumeIntegralValueMagnetostatic2D->averageMagneticFieldX, 0, 'e', 3), tr("A/m"));
    addValue(magnetostaticNode, tr("Hy avg.:"), tr("%1").arg(volumeIntegralValueMagnetostatic2D->averageMagneticFieldY, 0, 'e', 3), tr("A/m"));
    addValue(magnetostaticNode, tr("H: avg."), tr("%1").arg(volumeIntegralValueMagnetostatic2D->averageMagneticField, 0, 'e', 3), tr("A/m"));
    addValue(magnetostaticNode, tr("Bx avg.:"), tr("%1").arg(volumeIntegralValueMagnetostatic2D->averageFluxDensityX, 0, 'e', 3), tr("T"));
    addValue(magnetostaticNode, tr("By avg.:"), tr("%1").arg(volumeIntegralValueMagnetostatic2D->averageFluxDensityY, 0, 'e', 3), tr("T"));
    addValue(magnetostaticNode, tr("B avg.:"), tr("%1").arg(volumeIntegralValueMagnetostatic2D->averageFluxDensity, 0, 'e', 3), tr("T"));
    addValue(magnetostaticNode, tr("Energy:"), tr("%1").arg(volumeIntegralValueMagnetostatic2D->energy, 0, 'e', 3), tr("J"));
}

void VolumeIntegralValueView::showHeat2D(VolumeIntegralValueHeat2D *volumeIntegralValueHeat2D)
{
    // heat
    QTreeWidgetItem *heatNode = new QTreeWidgetItem(trvWidget);
    heatNode->setText(0, tr("Heat Transfer"));
    heatNode->setExpanded(true);

    addValue(heatNode, tr("Temperature:"), tr("%1").arg(volumeIntegralValueHeat2D->averageTemperature, 0, 'e', 3), tr("deg."));
    addValue(heatNode, tr("Gx avg.:"), tr("%1").arg(volumeIntegralValueHeat2D->averageTemperatureGradientX, 0, 'e', 3), tr("K/m"));
    addValue(heatNode, tr("Gy avg.:"), tr("%1").arg(volumeIntegralValueHeat2D->averageTemperatureGradientY, 0, 'e', 3), tr("K/m"));
    addValue(heatNode, tr("G avg.:"), tr("%1").arg(volumeIntegralValueHeat2D->averageTemperatureGradient, 0, 'e', 3), tr("K/m"));
    addValue(heatNode, tr("Fx avg.:"), tr("%1").arg(volumeIntegralValueHeat2D->averageHeatFluxX, 0, 'e', 3), tr("W/m2"));
    addValue(heatNode, tr("Fy avg.:"), tr("%1").arg(volumeIntegralValueHeat2D->averageHeatFluxY, 0, 'e', 3), tr("W/m2"));
    addValue(heatNode, tr("F avg.:"), tr("%1").arg(volumeIntegralValueHeat2D->averageHeatFlux, 0, 'e', 3), tr("W/m2"));
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

VolumeIntegralValue::VolumeIntegralValue(Scene *scene)
{
    this->m_scene = scene;

    cross_section = 0;
    volume = 0;
    for (int i = 0; i<m_scene->labels.length(); i++)
    {
        if (m_scene->labels[i]->isSelected)
        {
            cross_section += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_CROSSSECTION);
            volume += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_VOLUME);
        }
    }
}

QString VolumeIntegralValue::toString()
{
    return ""; // QString::number(point.x, 'f', 5) + "; " + QString::number(point.y, 'f', 5);
}

// ****************************************************************************************************************

VolumeIntegralValueElectrostatic2D::VolumeIntegralValueElectrostatic2D(Scene *scene) : VolumeIntegralValue(scene)
{
    if (scene->sceneSolution()->sln())
    {
        averageElectricFieldX = 0;
        averageElectricFieldY = 0;
        averageElectricField = 0;
        averageDisplacementX = 0;
        averageDisplacementY = 0;
        averageDisplacement = 0;
        energy = 0;
        for (int i = 0; i<m_scene->labels.length(); i++)
        {
            if (m_scene->labels[i]->isSelected)
            {
                averageElectricFieldX += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ELECTRICFIELD_X);
                averageElectricFieldY += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ELECTRICFIELD_Y);
                averageElectricField += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ELECTRICFIELD);
                averageDisplacementX += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_DISPLACEMENT_X);
                averageDisplacementY += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_DISPLACEMENT_Y);
                averageDisplacement += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_DISPLACEMENT);
                energy += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_ELECTROSTATIC_ENERGY_DENSITY);
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

QString VolumeIntegralValueElectrostatic2D::toString()
{
    return ""; // QString::number(point.x, 'f', 5) + "; " + QString::number(point.y, 'f', 5);
}

// ****************************************************************************************************************

VolumeIntegralValueMagnetostatic2D::VolumeIntegralValueMagnetostatic2D(Scene *scene) : VolumeIntegralValue(scene)
{
    if (scene->sceneSolution()->sln())
    {
        averageMagneticFieldX = 0;
        averageMagneticFieldY = 0;
        averageMagneticField = 0;
        averageFluxDensityX = 0;
        averageFluxDensityY = 0;
        averageFluxDensity = 0;
        energy = 0;
        for (int i = 0; i<m_scene->labels.length(); i++)
        {
            if (m_scene->labels[i]->isSelected)
            {
                averageMagneticFieldX += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_MAGNETICFIELD_X);
                averageMagneticFieldY += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_MAGNETICFIELD_Y);
                averageMagneticField += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_MAGNETICFIELD);
                averageFluxDensityX += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_FLUX_DENSITY_X);
                averageFluxDensityY += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_FLUX_DENSITY_Y);
                averageFluxDensity += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_FLUX_DENSITY);
                energy += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_MAGNETOSTATIC_ENERGY_DENSITY);
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

QString VolumeIntegralValueMagnetostatic2D::toString()
{
    return ""; // QString::number(point.x, 'f', 5) + "; " + QString::number(point.y, 'f', 5);
}

// ****************************************************************************************************************

VolumeIntegralValueHeat2D::VolumeIntegralValueHeat2D(Scene *scene) : VolumeIntegralValue(scene)
{
    if (scene->sceneSolution()->sln())
    {
        averageTemperature = 0;
        averageTemperatureGradientX = 0;
        averageTemperatureGradientY = 0;
        averageTemperatureGradient = 0;
        averageHeatFluxX = 0;
        averageHeatFluxY = 0;
        averageHeatFlux = 0;
        for (int i = 0; i<m_scene->labels.length(); i++)
        {
            if (m_scene->labels[i]->isSelected)
            {
                averageTemperature += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE);
                averageTemperatureGradientX += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE_GRADIENT_X);
                averageTemperatureGradientY += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE_GRADIENT_Y);
                averageTemperatureGradient += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HEAT_TEMPERATURE_GRADIENT);
                averageHeatFluxX += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HEAT_FLUX_X);
                averageHeatFluxY += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HEAT_FLUX_Y);
                averageHeatFlux += m_scene->sceneSolution()->volumeIntegral(i, PHYSICFIELDINTEGRAL_VOLUME_HEAT_FLUX);
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

QString VolumeIntegralValueHeat2D::toString()
{
    return ""; // QString::number(point.x, 'f', 5) + "; " + QString::number(point.y, 'f', 5);
}

// ***********************************************************************************************************************

VolumeIntegralValue *volumeIntegralValueFactory(Scene *scene)
{
    switch (scene->projectInfo().physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        // electrostatic
        return new VolumeIntegralValueElectrostatic2D(scene);
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        // electrostatic
        return new VolumeIntegralValueMagnetostatic2D(scene);
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        // heat transfer
        return new VolumeIntegralValueHeat2D(scene);
        break;
    default:
        cerr << "Physical field '" + scene->projectInfo().physicFieldString().toStdString() + "' is not implemented. VolumeIntegralValue *volumeIntegralValueFactory(Scene *scene)" << endl;
        throw;
        break;
    }
}
