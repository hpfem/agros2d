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

#ifndef SCENEVIEW_POST_H
#define SCENEVIEW_POST_H

#include "util.h"
#include "sceneview_common.h"

template <typename Scalar> class SceneSolution;

class SceneViewPostInterface : public SceneViewCommon
{
    Q_OBJECT

public:
    SceneViewPostInterface(QWidget *parent = 0);


protected:
    SceneSolution<double> *m_sceneSolution;

    double m_texScale;
    double m_texShift;

    virtual int textureScalar() { return -1; }

    void paintScalarFieldColorBar(double min, double max);

    // palette
    const double *paletteColor(double x) const;
    const double *paletteColorOrder(int n) const;
    void paletteCreate(int texture);
    void paletteFilter(int texture);
    void paletteUpdateTexAdjust();

public slots:
    void timeStepChanged(bool showViewProgress = false);

protected slots:
    virtual void clearGLLists() {}
};

#endif // SCENEVIEW_POST_H
