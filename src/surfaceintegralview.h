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

#ifndef SURFACEINTEGRALVIEW_H
#define SURFACEINTEGRALVIEW_H

#include "util.h"
#include "gui.h"
#include "hermes2d.h"

class SurfaceIntegralValue
{
protected:
    Element *e;

    double *x;
    double *value;
    double *dudx, *dudy;
    double3 *pt;
    double3 *tan;

    bool boundary;

    void calculate();
    virtual void calculateVariables(int i) = 0;

public:
    double length;
    double surface;

    SurfaceIntegralValue();

    virtual QStringList variables() = 0;
};

class SurfaceIntegralValueView : public QDockWidget
{
    Q_OBJECT

public slots:
    void doShowSurfaceIntegral();

public:
    SurfaceIntegralValueView(QWidget *parent = 0);
    ~SurfaceIntegralValueView();

private:
    QTreeWidget *trvWidget;
};

#endif // SURFACEINTEGRALVIEW_H
