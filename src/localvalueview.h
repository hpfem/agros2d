#ifndef LOCALVALUE_H
#define LOCALVALUE_H

#include "util.h"
#include "gui.h"

class LocalPointValue
{
public:
    Point point;

    LocalPointValue(Point &point);

    virtual double variableValue(PhysicFieldVariable physicFieldVariable, PhysicFieldVariableComp physicFieldVariableComp) = 0;
    virtual QStringList variables() = 0;
};

class LocalPointValueView : public QDockWidget
{
    Q_OBJECT

public slots:
    void doShowPoint();
    void doShowPoint(const Point &point);

public:
    LocalPointValueView(QWidget *parent = 0);
    ~LocalPointValueView();

private:
    QTreeWidget *trvWidget;
    QAction *actPoint;
    QMenu *mnuInfo;

    Point point;

    void createActions();
    void createMenu();

private slots:
    void doPoint();
    void doContextMenu(const QPoint &pos);
};

class LocalPointValueDialog : public QDialog
{
    Q_OBJECT
public:
    LocalPointValueDialog(Point point, QWidget *parent = 0);
    ~LocalPointValueDialog();

    Point point();

private:
    SLineEdit *txtPointX;
    SLineEdit *txtPointY;
};

#endif // LOCALVALUE_H
