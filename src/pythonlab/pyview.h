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

#ifndef PYTHONLABVIEW_H
#define PYTHONLABVIEW_H

#include "util.h"
#include "util/global.h"

#include "scene.h"
#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

class Solution;
class SceneViewPreprocessor;
class SceneViewMesh;
class SceneViewPost2D;
class SceneViewPost3D;
class PostHermes;

// view
struct PyView
{
    void saveImageToFile(char *file, int width, int height);
};

// view config
struct PyViewConfig
{
    // field
    void setField(char *fieldid);
    inline char* getField() const { return const_cast<char*>(Agros2D::scene()->activeViewField()->fieldId().toStdString().c_str()); }

    // time step
    void setActiveTimeStep(int timeStep);
    inline int getActiveTimeStep() const { return Agros2D::scene()->activeTimeStep(); }

    // adaptivity step
    void setActiveAdaptivityStep(int adaptivityStep);
    inline int getActiveAdaptivityStep() const { return Agros2D::scene()->activeAdaptivityStep(); }

    // solution type
    void setActiveSolutionType(char *solutionType);
    inline char* getActiveSolutionType() const { return const_cast<char*>(solutionTypeToStringKey(Agros2D::scene()->activeSolutionType()).toStdString().c_str()); }

    // grid
    void setGridShow(bool show);
    inline bool getGridShow() const { return Agros2D::problem()->configView()->showGrid; }
    void setGridStep(double step);
    inline double getGridStep() const { return Agros2D::problem()->configView()->gridStep; }

    // axes
    void setAxesShow(bool show);
    inline bool getAxesShow() const { return Agros2D::problem()->configView()->showAxes; }

    // rulers
    void setRulersShow(bool show);
    inline bool getRulersShow() const { return Agros2D::problem()->configView()->showRulers; }

    // todo: (Franta) font, size of nodes and edges and labels, colors
};

// view mesh
struct PyViewMesh
{
    void activate();

    // mesh
    void setInitialMeshViewShow(bool show);
    inline bool getInitialMeshViewShow() const { return Agros2D::problem()->configView()->showInitialMeshView; }
    void setSolutionMeshViewShow(bool show);
    inline bool getSolutionMeshViewShow() const { return Agros2D::problem()->configView()->showSolutionMeshView; }

    // polynomial order
    void setOrderViewShow(bool show);
    inline bool getOrderViewShow() const { return Agros2D::problem()->configView()->showOrderView; }
    void setOrderViewColorBar(bool show);
    inline bool getOrderViewColorBar() const { return Agros2D::problem()->configView()->showOrderColorBar; }
    void setOrderViewLabel(bool show);
    inline bool getOrderViewLabel() const { return Agros2D::problem()->configView()->orderLabel; }
    void setOrderViewPalette(char* palette);
    inline char* getOrderViewPalette() const { return const_cast<char*>(paletteOrderTypeToStringKey(Agros2D::problem()->configView()->orderPaletteOrderType).toStdString().c_str()); }
};

// post
struct PyViewPost
{
    // scalar view
    void setScalarViewVariable(char* var);
    inline char* getScalarViewVariable() const { return const_cast<char*>(Agros2D::problem()->configView()->scalarVariable.toStdString().c_str()); }
    void setScalarViewVariableComp(char* component);
    inline char* getScalarViewVariableComp() const { return const_cast<char*>(physicFieldVariableCompToStringKey(Agros2D::problem()->configView()->scalarVariableComp).toStdString().c_str()); }

    void setScalarViewPalette(char* palette);
    inline char* getScalarViewPalette() const { return const_cast<char*>(paletteTypeToStringKey(Agros2D::problem()->configView()->paletteType).toStdString().c_str()); }
    void setScalarViewPaletteQuality(char* quality);
    inline char* getScalarViewPaletteQuality() const { return const_cast<char*>(paletteQualityToStringKey(Agros2D::problem()->configView()->linearizerQuality).toStdString().c_str()); }
    void setScalarViewPaletteSteps(int steps);
    inline int getScalarViewPaletteSteps() const { return Agros2D::problem()->configView()->paletteSteps; }
    void setScalarViewPaletteFilter(bool filter);
    inline bool getScalarViewPaletteFilter() const { return Agros2D::problem()->configView()->paletteFilter; }

    void setScalarViewRangeLog(bool log);
    inline bool getScalarViewRangeLog() const { return Agros2D::problem()->configView()->scalarRangeLog; }
    void setScalarViewRangeBase(double base);
    inline double getScalarViewRangeBase() const { return Agros2D::problem()->configView()->scalarRangeBase; }

    void setScalarViewColorBar(bool show);
    inline bool getScalarViewColorBar() const { return Agros2D::problem()->configView()->showScalarColorBar; }
    void setScalarViewDecimalPlace(int place);
    inline int getScalarViewDecimalPlace() const { return Agros2D::problem()->configView()->scalarDecimalPlace; }

    void setScalarViewRangeAuto(bool autoRange);
    inline bool getScalarViewRangeAuto() const { return Agros2D::problem()->configView()->scalarRangeAuto; }
    void setScalarViewRangeMin(double min);
    inline double getScalarViewRangeMin() const { return Agros2D::problem()->configView()->scalarRangeMin; }
    void setScalarViewRangeMax(double max);
    inline double getScalarViewRangeMax() const { return Agros2D::problem()->configView()->scalarRangeMax; }
};

// post2d
struct PyViewPost2D
{
    void activate();

    // scalar
    void setScalarViewShow(bool show);
    inline bool getScalarViewShow() const { return Agros2D::problem()->configView()->showScalarView; }

    // contour
    void setContourShow(bool show);
    inline bool getContourShow() const { return Agros2D::problem()->configView()->showContourView; }
    void setContourCount(int count);
    inline int getContourCount() const { return Agros2D::problem()->configView()->contoursCount; }
    void setContourVariable(char* var);
    inline char* getContourVariable() const { return const_cast<char*>(Agros2D::problem()->configView()->contourVariable.toStdString().c_str()); }

    // vector
    void setVectorShow(bool show);
    inline bool getVectorShow() const { return Agros2D::problem()->configView()->showVectorView; }
    void setVectorCount(int count);
    inline int getVectorCount() const { return Agros2D::problem()->configView()->vectorCount; }
    void setVectorScale(double scale);
    inline int getVectorScale() const { return Agros2D::problem()->configView()->vectorScale; }
    void setVectorVariable(char* var);
    inline char* getVectorVariable() const { return const_cast<char*>(Agros2D::problem()->configView()->vectorVariable.toStdString().c_str()); }
    void setVectorProportional(bool show);
    inline bool getVectorProportional() const { return Agros2D::problem()->configView()->vectorProportional; }
    void setVectorColor(bool show);
    inline bool getVectorColor() const { return Agros2D::problem()->configView()->vectorColor; }
};

// post3d
struct PyViewPost3D
{
    void activate();

    // scalar view
    void setPost3DMode(char* mode);
    inline char* getPost3DMode() const { return const_cast<char*>(sceneViewPost3DModeToStringKey(Agros2D::problem()->configView()->showPost3D).toStdString().c_str()); }

};

#endif // PYTHONLABVIEW_H
