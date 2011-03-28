// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#ifndef LOCALVALUE_H
#define LOCALVALUE_H

#include "util.h"

class SLineEditValue;
class SceneLabelMarker;
class Solution;

struct PointValue
{
    PointValue()
    {
        this->value = 0.0;
        this->derivative = Point();
        this->marker = NULL;
    }

    PointValue(double value, Point derivative, SceneLabelMarker *marker)
    {
        this->value = value;
        this->derivative = derivative;
        this->marker = marker;
    }

    double value;
    Point derivative;
    SceneLabelMarker *marker;
};

class LocalPointValue
{
protected:
    double value;
    Point derivative;
    SceneLabelMarker *labelMarker;

    PointValue pointValue(Solution *sln, const Point &point);

public:
    Point point;

    LocalPointValue(const Point &point);

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
    QAction *actCopy;
    QMenu *mnuInfo;

    Point point;

    void createActions();
    void createMenu();

private slots:
    void doContextMenu(const QPoint &pos);
    void doPoint();
    void doCopyValue();
};

class LocalPointValueDialog : public QDialog
{
    Q_OBJECT
public:
    LocalPointValueDialog(Point point, QWidget *parent = 0);
    ~LocalPointValueDialog();

    Point point();

private:
    QDialogButtonBox *buttonBox;

    SLineEditValue *txtPointX;
    SLineEditValue *txtPointY;

private slots:
    void evaluated(bool isError);
};

#endif // LOCALVALUE_H
