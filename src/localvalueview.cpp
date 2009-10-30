#include "localvalueview.h"
#include "scene.h"

LocalPointValue::LocalPointValue(Point &point)
{
    this->point = point;
}

// *************************************************************************************************************************************

LocalPointValueView::LocalPointValueView(QWidget *parent): QDockWidget(tr("Local Values"), parent)
{
    QSettings settings;
    
    setMinimumWidth(280);
    setObjectName("LocalPointValueView");

    createActions();
    createMenu();

    trvWidget = new QTreeWidget();
    trvWidget->setHeaderHidden(false);
    trvWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    trvWidget->setMouseTracking(true);
    trvWidget->setColumnCount(3);
    trvWidget->setColumnWidth(0, settings.value("LocalPointValueView/TreeViewColumn0", 180).value<int>());
    trvWidget->setColumnWidth(1, settings.value("LocalPointValueView/TreeViewColumn1", 80).value<int>());
    trvWidget->setColumnWidth(2, settings.value("LocalPointValueView/TreeViewColumn2", 20).value<int>());

    QStringList labels;
    labels << tr("Label") << tr("Value") << tr("Unit");
    trvWidget->setHeaderLabels(labels);

    connect(trvWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));

    QToolButton *btnPoint = new QToolButton();
    btnPoint->setText(actPoint->text());
    btnPoint->setIcon(actPoint->icon());
    btnPoint->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    connect(btnPoint, SIGNAL(clicked()), this, SLOT(doPoint()));

    // main widget
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(trvWidget);
    layout->addWidget(btnPoint);
    layout->setContentsMargins(0, 0, 0, 7);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);    
    
    setWidget(widget);
}

LocalPointValueView::~LocalPointValueView()
{
    QSettings settings;
    settings.setValue("LocalPointValueView/TreeViewColumn0", trvWidget->columnWidth(0));
    settings.setValue("LocalPointValueView/TreeViewColumn1", trvWidget->columnWidth(1));
    settings.setValue("LocalPointValueView/TreeViewColumn2", trvWidget->columnWidth(2));
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
    
    LocalPointValueDialog localPointValueDialog(point);
    if (localPointValueDialog.exec() == QDialog::Accepted)
    {
        doShowPoint(localPointValueDialog.point());
    }
}

void LocalPointValueView::doContextMenu(const QPoint &pos)
{
    mnuInfo->exec(QCursor::pos());
}

void LocalPointValueView::doShowPoint(const Point &point)
{
    // store point
    this->point = point;
    doShowPoint();
}

void LocalPointValueView::doShowPoint()
{
    trvWidget->clear();

    // point
    QTreeWidgetItem *pointNode = new QTreeWidgetItem(trvWidget);
    pointNode->setText(0, tr("Point"));
    pointNode->setExpanded(true);

    addTreeWidgetItemValue(pointNode, Util::scene()->problemInfo()->labelX() + ":", QString("%1").arg(point.x, 0, 'f', 5), tr("m"));
    addTreeWidgetItemValue(pointNode, Util::scene()->problemInfo()->labelY() + ":", QString("%1").arg(point.y, 0, 'f', 5), tr("m"));

    trvWidget->insertTopLevelItem(0, pointNode);

    if (Util::scene()->sceneSolution()->isSolved())
        Util::scene()->problemInfo()->hermes()->showLocalValue(trvWidget, Util::scene()->problemInfo()->hermes()->localPointValue(point));
}

LocalPointValueDialog::LocalPointValueDialog(Point point, QWidget *parent) : QDialog(parent)
{
    setWindowIcon(icon("scene-node"));
    setWindowTitle(tr("Local point value"));

    setModal(true);

    txtPointX = new SLineEdit(QString::number(point.x), false);
    txtPointY = new SLineEdit(QString::number(point.y), false);

    QFormLayout *layoutPoint = new QFormLayout();
    layoutPoint->addRow(Util::scene()->problemInfo()->labelX() + " (m):", txtPointX);
    layoutPoint->addRow(Util::scene()->problemInfo()->labelY() + " (m):", txtPointY);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutPoint);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    resize(sizeHint());
    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

LocalPointValueDialog::~LocalPointValueDialog()
{
    delete txtPointX;
    delete txtPointY;
}

Point LocalPointValueDialog::point()
{
    return Point(txtPointX->value(), txtPointY->value());
}
