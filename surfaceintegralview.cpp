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

    trvWidget->insertTopLevelItem(0, pointGeometry);

    if (surfaceIntegralValue->scene()->sceneSolution()->sln())
    {
        if (SurfaceIntegralValueElectrostatic2D *surfaceIntegralValueElectrostatic2D = dynamic_cast<SurfaceIntegralValueElectrostatic2D *>(surfaceIntegralValue))
            showElectrostatic2D(surfaceIntegralValueElectrostatic2D);
        if (SurfaceIntegralValueMagnetostatic2D *surfaceIntegralValueMagnetostatic2D = dynamic_cast<SurfaceIntegralValueMagnetostatic2D *>(surfaceIntegralValue))
            showMagnetostatic2D(surfaceIntegralValueMagnetostatic2D);
        if (SurfaceIntegralValueHeat2D *surfaceIntegralValueHeat2D = dynamic_cast<SurfaceIntegralValueHeat2D *>(surfaceIntegralValue))
            showHeat2D(surfaceIntegralValueHeat2D);
    }
}

void SurfaceIntegralValueView::showElectrostatic2D(SurfaceIntegralValueElectrostatic2D *surfaceIntegralValueElectrostatic2D)
{
    // electrostatic
    QTreeWidgetItem *electrostaticNode = new QTreeWidgetItem(trvWidget);
    electrostaticNode->setText(0, tr("Electrostatic Field"));
    electrostaticNode->setExpanded(true);


    // insert electrostatic item
    trvWidget->insertTopLevelItem(0, electrostaticNode);
}

void SurfaceIntegralValueView::showMagnetostatic2D(SurfaceIntegralValueMagnetostatic2D *surfaceIntegralValueMagnetostatic2D)
{
    // electrostatic
    QTreeWidgetItem *magnetostaticNode = new QTreeWidgetItem(trvWidget);
    magnetostaticNode->setText(0, tr("Magnetostatic Field"));
    magnetostaticNode->setExpanded(true);

    // insert electrostatic item
    trvWidget->insertTopLevelItem(0, magnetostaticNode);
}

void SurfaceIntegralValueView::showHeat2D(SurfaceIntegralValueHeat2D *surfaceIntegralValueHeat2D)
{
    // heat
    QTreeWidgetItem *heatNode = new QTreeWidgetItem(trvWidget);
    heatNode->setText(0, tr("Heat Transfer"));
    heatNode->setExpanded(true);

    // insert heat transfer item
    trvWidget->insertTopLevelItem(0, heatNode);
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

    // cross_section area
    length = 0;
    for (int i = 0; i<m_scene->edges.length(); i++)
    {
        if (m_scene->edges[i]->isSelected)
        {
           length += m_scene->sceneSolution()->surfaceIntegral(i, PHYSICFIELDINTEGRAL_SURFACE_LENGTH);
        }
    }
}

QString SurfaceIntegralValue::toString()
{
    return ""; // QString::number(point.x, 'f', 5) + "; " + QString::number(point.y, 'f', 5);
}

// ****************************************************************************************************************

SurfaceIntegralValueElectrostatic2D::SurfaceIntegralValueElectrostatic2D(Scene *scene) : SurfaceIntegralValue(scene)
{
    if (scene->sceneSolution()->sln())
    {

    }
}

QString SurfaceIntegralValueElectrostatic2D::toString()
{
    return ""; // QString::number(point.x, 'f', 5) + "; " + QString::number(point.y, 'f', 5);
}

// ****************************************************************************************************************

SurfaceIntegralValueMagnetostatic2D::SurfaceIntegralValueMagnetostatic2D(Scene *scene) : SurfaceIntegralValue(scene)
{
    if (scene->sceneSolution()->sln())
    {

    }
}

QString SurfaceIntegralValueMagnetostatic2D::toString()
{
    return ""; // QString::number(point.x, 'f', 5) + "; " + QString::number(point.y, 'f', 5);
}

// ****************************************************************************************************************

SurfaceIntegralValueHeat2D::SurfaceIntegralValueHeat2D(Scene *scene) : SurfaceIntegralValue(scene)
{
    if (scene->sceneSolution()->sln())
    {

    }
}

QString SurfaceIntegralValueHeat2D::toString()
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
        return new SurfaceIntegralValueElectrostatic2D(scene);
        break;
    case PHYSICFIELD_MAGNETOSTATIC:
        // electrostatic
        return new SurfaceIntegralValueMagnetostatic2D(scene);
        break;
    case PHYSICFIELD_HEAT_TRANSFER:
        // heat transfer
        return new SurfaceIntegralValueHeat2D(scene);
        break;
    default:
        cerr << "Physical field '" + scene->projectInfo().physicFieldString().toStdString() + "' is not implemented. SurfaceIntegralValue *surfaceIntegralValueFactory(Scene *scene)" << endl;
        throw;
        break;
    }
}
