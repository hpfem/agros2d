#include "surfaceintegralview.h"

SurfaceIntegralValueView::SurfaceIntegralValueView(QWidget *parent): QDockWidget(tr("Surface Integral"), parent)
{
    setMinimumWidth(280);
    setObjectName("SurfaceIntegralValueView");

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

void SurfaceIntegralValueView::doShowSurfaceIntegral(SurfaceIntegralValue *surfaceIntegralValue)
{
    trvWidget->clear();

    // point
    QTreeWidgetItem *pointGeometry = new QTreeWidgetItem(trvWidget);
    pointGeometry->setText(0, tr("Geometry"));
    pointGeometry->setExpanded(true);

    addValue(pointGeometry, tr("Length:"), tr("%1").arg(surfaceIntegralValue->length, 0, 'e', 3), tr("m"));
    addValue(pointGeometry, tr("Surface:"), tr("%1").arg(surfaceIntegralValue->surface, 0, 'e', 3), tr("m2"));

    trvWidget->insertTopLevelItem(0, pointGeometry);

    if (surfaceIntegralValue->scene()->sceneSolution()->sln())
    {
        if (SurfaceIntegralValueElectrostatic *surfaceIntegralValueElectrostatic = dynamic_cast<SurfaceIntegralValueElectrostatic *>(surfaceIntegralValue))
            showElectrostatic(surfaceIntegralValueElectrostatic);
        if (SurfaceIntegralValueMagnetostatic *surfaceIntegralValueMagnetostatic = dynamic_cast<SurfaceIntegralValueMagnetostatic *>(surfaceIntegralValue))
            showMagnetostatic(surfaceIntegralValueMagnetostatic);
        if (SurfaceIntegralValueHeat *surfaceIntegralValueHeat = dynamic_cast<SurfaceIntegralValueHeat *>(surfaceIntegralValue))
            showHeat(surfaceIntegralValueHeat);
    }
}

void SurfaceIntegralValueView::showElectrostatic(SurfaceIntegralValueElectrostatic *surfaceIntegralValueElectrostatic)
{
    // electrostatic
    QTreeWidgetItem *electrostaticNode = new QTreeWidgetItem(trvWidget);
    electrostaticNode->setText(0, tr("Electrostatic Field"));
    electrostaticNode->setExpanded(true);

    addValue(electrostaticNode, tr("Charge:"), tr("%1").arg(surfaceIntegralValueElectrostatic->surfaceCharge, 0, 'e', 3), tr("C"));
}

void SurfaceIntegralValueView::showMagnetostatic(SurfaceIntegralValueMagnetostatic *surfaceIntegralValueMagnetostatic)
{
    // electrostatic
    QTreeWidgetItem *magnetostaticNode = new QTreeWidgetItem(trvWidget);
    magnetostaticNode->setText(0, tr("Magnetostatic Field"));
    magnetostaticNode->setExpanded(true);

}

void SurfaceIntegralValueView::showHeat(SurfaceIntegralValueHeat *surfaceIntegralValueHeat)
{
    // heat
    QTreeWidgetItem *heatNode = new QTreeWidgetItem(trvWidget);
    heatNode->setText(0, tr("Heat Transfer"));
    heatNode->setExpanded(true);
}

void SurfaceIntegralValueView::addValue(QTreeWidgetItem *parent, QString name, QString text, QString unit)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, name);
    item->setText(1, text);
    item->setTextAlignment(1, Qt::AlignRight);
    item->setText(2, unit + " ");
    item->setTextAlignment(2, Qt::AlignLeft);
}

SurfaceIntegralValue::SurfaceIntegralValue(Scene *scene)
{
    this->m_scene = scene;

    length = 0;
    surface = 0;
    for (int i = 0; i<m_scene->edges.length(); i++)
    {
        if (m_scene->edges[i]->isSelected)
        {
            length += m_scene->sceneSolution()->surfaceIntegral(i, PHYSICFIELDINTEGRAL_SURFACE_LENGTH);
            surface += m_scene->sceneSolution()->surfaceIntegral(i, PHYSICFIELDINTEGRAL_SURFACE_SURFACE);
        }
    }
}

QString SurfaceIntegralValue::toString()
{
    return ""; // QString::number(point.x, 'f', 5) + "; " + QString::number(point.y, 'f', 5);
}

// ****************************************************************************************************************

SurfaceIntegralValueElectrostatic::SurfaceIntegralValueElectrostatic(Scene *scene) : SurfaceIntegralValue(scene)
{
    surfaceCharge = 0;

    if (scene->sceneSolution()->sln())
    {
        for (int i = 0; i<m_scene->edges.length(); i++)
        {
            if (m_scene->edges[i]->isSelected)
            {
                surfaceCharge += m_scene->sceneSolution()->surfaceIntegral(i, PHYSICFIELDINTEGRAL_SURFACE_ELECTROSTATIC_CHARGE_DENSITY);
            }
        }
    }
}

QString SurfaceIntegralValueElectrostatic::toString()
{
    return ""; // QString::number(point.x, 'f', 5) + "; " + QString::number(point.y, 'f', 5);
}

// ****************************************************************************************************************

SurfaceIntegralValueMagnetostatic::SurfaceIntegralValueMagnetostatic(Scene *scene) : SurfaceIntegralValue(scene)
{
    if (scene->sceneSolution()->sln())
    {

    }
}

QString SurfaceIntegralValueMagnetostatic::toString()
{
    return ""; // QString::number(point.x, 'f', 5) + "; " + QString::number(point.y, 'f', 5);
}

// ****************************************************************************************************************

SurfaceIntegralValueHeat::SurfaceIntegralValueHeat(Scene *scene) : SurfaceIntegralValue(scene)
{
    if (scene->sceneSolution()->sln())
    {

    }
}

QString SurfaceIntegralValueHeat::toString()
{
    return ""; // QString::number(point.x, 'f', 5) + "; " + QString::number(point.y, 'f', 5);
}

// ***********************************************************************************************************************

SurfaceIntegralValue *surfaceIntegralValueFactory(Scene *scene)
{
    switch (scene->projectInfo().physicField)
    {
    case PHYSICFIELD_ELECTROSTATIC:
        // electrostatic
        return new SurfaceIntegralValueElectrostatic(scene);
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        // electrostatic
        return new SurfaceIntegralValueMagnetostatic(scene);
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        // heat transfer
        return new SurfaceIntegralValueHeat(scene);
        break;
    default:
        cerr << "Physical field '" + scene->projectInfo().physicFieldString().toStdString() + "' is not implemented. SurfaceIntegralValue *surfaceIntegralValueFactory(Scene *scene)" << endl;
        throw;
        break;
    }
}
