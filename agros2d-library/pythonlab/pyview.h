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

#include "util/global.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

struct PyView
{
    // save image
    void saveImageToFile(const std::string &file, int width, int height);

    // zoom
    void zoomBestFit();
    void zoomIn();
    void zoomOut();
    void zoomRegion(double x1, double y1, double x2, double y2);
};

struct PyViewConfig
{
    // grid
    void setGridShow(bool show) { set(ProblemSetting::View_ShowGrid, show); }
    inline bool getGridShow() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ShowGrid).toBool(); }

    void setGridStep(double step);
    inline double getGridStep() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_GridStep).toDouble(); }

    // axes
    void setAxesShow(bool show) { set(ProblemSetting::View_ShowAxes, show); }
    inline bool getAxesShow() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ShowAxes).toBool(); }

    // rulers
    void setRulersShow(bool show) { set(ProblemSetting::View_ShowRulers, show); }
    inline bool getRulersShow() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ShowRulers).toBool(); }

private:
    void set(ProblemSetting::Type type, QVariant value);
};

struct PyViewMeshAndPost
{
    // field
    void setField(const std::string &fieldid);
    std::string getField() const;

    // time step
    void setActiveTimeStep(int timeStep);
    int getActiveTimeStep() const;

    // adaptivity step
    void setActiveAdaptivityStep(int adaptivityStep);
    int getActiveAdaptivityStep() const;

    // solution type
    void setActiveSolutionType(const std::string &solutionType);
    std::string getActiveSolutionType() const;
};

struct PyViewMesh
{
    void activate();

    // initial mesh
    void setInitialMeshViewShow(bool show) {set(ProblemSetting::View_ShowInitialMeshView, show); }
    inline bool getInitialMeshViewShow() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ShowInitialMeshView).toBool(); }

    // solution mesh
    void setSolutionMeshViewShow(bool show) {set(ProblemSetting::View_ShowSolutionMeshView, show); }
    inline bool getSolutionMeshViewShow() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ShowSolutionMeshView).toBool(); }

    // polynomial order
    void setOrderViewShow(bool show) {set(ProblemSetting::View_ShowOrderView, show); }
    inline bool getOrderViewShow() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ShowOrderView).toBool(); }

    // order color bar
    void setOrderViewColorBar(bool show) {set(ProblemSetting::View_ShowOrderColorBar, show); }
    inline bool getOrderViewColorBar() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ShowOrderColorBar).toBool(); }

    // order labels
    void setOrderViewLabel(bool show) {set(ProblemSetting::View_ShowOrderLabel, show); }
    inline bool getOrderViewLabel() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ShowOrderLabel).toBool(); }

    // order pallete
    void setOrderViewPalette(const std::string & palette);
    inline std::string getOrderViewPalette() const
    {
        return paletteOrderTypeToStringKey((PaletteOrderType) Agros2D::problem()->setting()->value(ProblemSetting::View_OrderPaletteOrderType).toInt()).toStdString();
    }

private:
    void set(ProblemSetting::Type type, QVariant value);
};

struct PyViewPost
{
    // scalar view variable
    void setScalarViewVariable(const std::string & var);
    inline std::string getScalarViewVariable() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString().toStdString(); }
    void setScalarViewVariableComp(const std::string & component);
    inline std::string getScalarViewVariableComp() const
    {
        return physicFieldVariableCompToStringKey((PhysicFieldVariableComp) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariableComp).toInt()).toStdString();
    }

    // scalar view palete
    void setScalarViewPalette(const std::string & palette);
    inline std::string getScalarViewPalette() const
    {
        return paletteTypeToStringKey((PaletteType) Agros2D::problem()->setting()->value(ProblemSetting::View_PaletteType).toInt()).toStdString();
    }
    void setScalarViewPaletteQuality(const std::string & quality);
    inline std::string getScalarViewPaletteQuality() const
    {
        return paletteQualityToStringKey((PaletteQuality) Agros2D::problem()->setting()->value(ProblemSetting::View_LinearizerQuality).toInt()).toStdString();
    }
    void setScalarViewPaletteSteps(int steps);
    inline int getScalarViewPaletteSteps() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_PaletteSteps).toInt(); }
    void setScalarViewPaletteFilter(bool filter);
    inline bool getScalarViewPaletteFilter() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_PaletteFilter).toBool(); }
    void setScalarViewColorBar(bool show);
    inline bool getScalarViewColorBar() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ShowScalarColorBar).toBool(); }
    void setScalarViewDecimalPlace(int place);
    inline int getScalarViewDecimalPlace() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarDecimalPlace).toInt(); }

    // scalar view range
    void setScalarViewRangeAuto(bool autoRange);
    inline bool getScalarViewRangeAuto() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeAuto).toBool(); }
    void setScalarViewRangeMin(double min);
    inline double getScalarViewRangeMin() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble(); }
    void setScalarViewRangeMax(double max);
    inline double getScalarViewRangeMax() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble(); }
    void setScalarViewRangeLog(bool log);
    inline bool getScalarViewRangeLog() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeLog).toBool(); }
    void setScalarViewRangeBase(double base);
    inline double getScalarViewRangeBase() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarRangeBase).toDouble(); }
};

struct PyViewPost2D
{
    void activate();

    // scalar view
    void setScalarViewShow(bool show) { set(ProblemSetting::View_ShowScalarView, show); }
    inline bool getScalarViewShow() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ShowScalarView).toBool(); }

    // contour view
    void setContourShow(bool show) { set(ProblemSetting::View_ShowContourView, show); }
    inline bool getContourShow() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ShowContourView).toBool(); }
    void setContourCount(int count);
    inline int getContourCount() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ContoursCount).toInt(); }
    void setContourVariable(const std::string & var);
    inline std::string getContourVariable() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ContourVariable).toString().toStdString(); }

    // vector view
    void setVectorShow(bool show) { set(ProblemSetting::View_ShowVectorView, show); }
    inline bool getVectorShow() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ShowVectorView).toBool(); }
    void setVectorCount(int count);
    inline int getVectorCount() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_VectorCount).toInt(); }
    void setVectorScale(double scale);
    inline double getVectorScale() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_VectorScale).toDouble(); }
    void setVectorVariable(const std::string & var);
    inline std::string getVectorVariable() const { return const_cast<char*>(Agros2D::problem()->setting()->value(ProblemSetting::View_VectorVariable).toString().toStdString().c_str()); }
    void setVectorProportional(bool show) { set(ProblemSetting::View_VectorProportional, show); }
    inline bool getVectorProportional() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_VectorProportional).toBool(); }
    void setVectorColor(bool show) { set(ProblemSetting::View_VectorColor, show); }
    inline bool getVectorColor() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_VectorColor).toBool(); }

private:
    void set(ProblemSetting::Type type, QVariant value);
};

struct PyViewPost3D
{
    void activate();

    // mode
    void setPost3DMode(const std::string & mode);
    inline std::string getPost3DMode() const
    {
        return sceneViewPost3DModeToStringKey((SceneViewPost3DMode) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DMode).toInt()).toStdString();
    }

};

struct PyViewParticleTracing
{
    void activate();

    // void setParticleShow(bool show);
    // inline bool getParticleShow() const { return Agros2D::problem()->configView()->showParticleView; }
};

#endif // PYTHONLABVIEW_H
