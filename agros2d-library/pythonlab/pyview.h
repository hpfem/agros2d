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

#include "pythonlab/pythonengine_agros.h"
#include "util/global.h"
#include "hermes2d/problem.h"
#include "hermes2d/problem_config.h"

struct PyViewClass
{
    inline int getBoolParameter(const std::string &parameter)
    {
        ProblemSetting::Type type = Agros2D::problem()->setting()->stringKeyToType(QString::fromStdString(parameter));
        return Agros2D::problem()->setting()->value(type).toBool();
    }

    inline int getIntParameter(const std::string &parameter)
    {
        ProblemSetting::Type type = Agros2D::problem()->setting()->stringKeyToType(QString::fromStdString(parameter));
        return Agros2D::problem()->setting()->value(type).toInt();
    }

    inline double getDoubleParameter(const std::string &parameter)
    {
        ProblemSetting::Type type = Agros2D::problem()->setting()->stringKeyToType(QString::fromStdString(parameter));
        return Agros2D::problem()->setting()->value(type).toDouble();
    }
};

struct PyView
{
    // save image
    void saveImageToFile(const std::string &file, int width, int height);

    // zoom
    void zoomBestFit();
    void zoomIn();
    void zoomOut();
    void zoomRegion(double x1, double y1, double x2, double y2);

    SceneViewCommon *currentSceneViewMode();
};

struct PyViewConfig : PyViewClass
{
    // grid
    void setGridShow(bool show) { setProblemSetting(ProblemSetting::View_ShowGrid, show); }
    inline bool getGridShow() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ShowGrid).toBool(); }

    void setGridStep(double step);
    inline double getGridStep() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_GridStep).toDouble(); }

    // axes
    void setAxesShow(bool show) { setProblemSetting(ProblemSetting::View_ShowAxes, show); }
    inline bool getAxesShow() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ShowAxes).toBool(); }

    // rulers
    void setRulersShow(bool show) { setProblemSetting(ProblemSetting::View_ShowRulers, show); }
    inline bool getRulersShow() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ShowRulers).toBool(); }

    // fonts
    void setFontFamily(ProblemSetting::Type type, const std::string &family);
    void setFontPointSize(ProblemSetting::Type type, int size);

    void setPostFontFamily(const std::string &family) { setFontFamily(ProblemSetting::View_PostFontFamily, family); }
    inline std::string getPostFontFamily() const
    {
        return Agros2D::problem()->setting()->value(ProblemSetting::View_PostFontFamily).toString().toStdString();
    }

    void setPostFontPointSize(int size) { setFontPointSize(ProblemSetting::View_PostFontPointSize, size); }
    inline int getPostFontPointSize() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_PostFontPointSize).toInt(); }

    void setRulersFontFamily(const std::string &family) { setFontFamily(ProblemSetting::View_RulersFontFamily, family); }
    inline std::string getRulersFontFamily() const
    {
        return Agros2D::problem()->setting()->value(ProblemSetting::View_RulersFontFamily).toString().toStdString();
    }

    void setRulersFontPointSize(int size) { setFontPointSize(ProblemSetting::View_RulersFontPointSize, size); }
    inline int getRulersFontPointSize() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_RulersFontFamily).toInt(); }

    void setProblemSetting(ProblemSetting::Type type, bool value);
};

struct PyViewMeshAndPost : PyViewClass
{
    // time step
    void setActiveTimeStep(int timeStep);
    inline int getActiveTimeStep() const { return currentPythonEngineAgros()->postHermes()->activeTimeStep(); }

    // adaptivity step
    void setActiveAdaptivityStep(int adaptivityStep);
    inline int getActiveAdaptivityStep() const { return currentPythonEngineAgros()->postHermes()->activeAdaptivityStep(); }

    // solution type
    void setActiveSolutionType(const std::string &solutionType);
    inline std::string getActiveSolutionType() const
    {
        return solutionTypeToStringKey(currentPythonEngineAgros()->postHermes()->activeAdaptivitySolutionType()).toStdString();
    }
};

struct PyViewMesh : PyViewMeshAndPost
{
    template <typename Type>
    void setParameter(const std::string &parameter, Type value)
    {
        checkExistingMesh();
        ProblemSetting::Type type = Agros2D::problem()->setting()->stringKeyToType(QString::fromStdString(parameter));

        if (!silentMode())
            Agros2D::problem()->setting()->setValue(type, value);
    }

    void checkExistingMesh();
    void setProblemSetting(ProblemSetting::Type type, bool value);

    void activate();

    // field
    void setField(const std::string &fieldId);
    inline std::string getField() const
    {
        return currentPythonEngineAgros()->postHermes()->activeViewField()->fieldId().toStdString();
    }

    // initial mesh
    void setInitialMeshViewShow(bool show) { setProblemSetting(ProblemSetting::View_ShowInitialMeshView, show); }
    inline bool getInitialMeshViewShow() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ShowInitialMeshView).toBool(); }

    // solution mesh
    void setSolutionMeshViewShow(bool show) { setProblemSetting(ProblemSetting::View_ShowSolutionMeshView, show); }
    inline bool getSolutionMeshViewShow() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ShowSolutionMeshView).toBool(); }

    // polynomial order
    void setOrderViewShow(bool show) { setProblemSetting(ProblemSetting::View_ShowOrderView, show); }
    inline bool getOrderViewShow() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ShowOrderView).toBool(); }

    // order pallete
    void setOrderViewPalette(const std::string & palette);
    inline std::string getOrderViewPalette() const
    {
        return paletteOrderTypeToStringKey((PaletteOrderType) Agros2D::problem()->setting()->value(ProblemSetting::View_OrderPaletteOrderType).toInt()).toStdString();
    }
};

struct PyViewPost : PyViewMeshAndPost
{
    template <typename Type>
    void setParameter(const std::string &parameter, Type value)
    {
        checkExistingSolution();
        ProblemSetting::Type type = Agros2D::problem()->setting()->stringKeyToType(QString::fromStdString(parameter));

        if (!silentMode())
            Agros2D::problem()->setting()->setValue(type, value);
    }

    void checkExistingSolution();
    void setProblemSetting(ProblemSetting::Type type, bool value);

    // field
    void setField(const std::string &fieldId);
    inline std::string getField() const
    {
        return currentPythonEngineAgros()->postHermes()->activeViewField()->fieldId().toStdString();
    }

    // scalar view variable
    void setScalarViewVariable(const std::string &var);
    inline std::string getScalarViewVariable() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariable).toString().toStdString(); }
    void setScalarViewVariableComp(const std::string &component);
    inline std::string getScalarViewVariableComp() const
    {
        return physicFieldVariableCompToStringKey((PhysicFieldVariableComp) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarVariableComp).toInt()).toStdString();
    }

    // scalar view palete
    void setScalarViewPalette(const std::string &palette);
    inline std::string getScalarViewPalette() const
    {
        return paletteTypeToStringKey((PaletteType) Agros2D::problem()->setting()->value(ProblemSetting::View_PaletteType).toInt()).toStdString();
    }
    void setScalarViewPaletteQuality(const std::string &quality);
    inline std::string getScalarViewPaletteQuality() const
    {
        return paletteQualityToStringKey((PaletteQuality) Agros2D::problem()->setting()->value(ProblemSetting::View_LinearizerQuality).toInt()).toStdString();
    }
};

struct PyViewPost2D : PyViewPost
{
    void activate();

    // scalar view
    void setScalarViewShow(bool show) { setProblemSetting(ProblemSetting::View_ShowScalarView, show); }
    inline bool getScalarViewShow() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ShowScalarView).toBool(); }

    // contour view
    void setContourShow(bool show) { setProblemSetting(ProblemSetting::View_ShowContourView, show); }
    inline bool getContourShow() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ShowContourView).toBool(); }
    void setContourVariable(const std::string &var);
    inline std::string getContourVariable() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ContourVariable).toString().toStdString(); }

    // vector view
    void setVectorShow(bool show) { setProblemSetting(ProblemSetting::View_ShowVectorView, show); }
    inline bool getVectorShow() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_ShowVectorView).toBool(); }
    void setVectorVariable(const std::string &var);
    inline std::string getVectorVariable() const { return Agros2D::problem()->setting()->value(ProblemSetting::View_VectorVariable).toString().toStdString(); }
    void setVectorType(const std::string &type);
    inline std::string getVectorType() const
    {
        return vectorTypeToStringKey((VectorType) Agros2D::problem()->setting()->value(ProblemSetting::View_VectorType).toInt()).toStdString();
    }
    void setVectorCenter(const std::string &center);
    inline std::string getVectorCenter() const
    {
        return vectorCenterToStringKey((VectorCenter) Agros2D::problem()->setting()->value(ProblemSetting::View_VectorCenter).toInt()).toStdString();
    }
};

struct PyViewPost3D : PyViewPost
{
    void activate();

    // mode
    void setPost3DMode(const std::string &mode);
    inline std::string getPost3DMode() const
    {
        return sceneViewPost3DModeToStringKey((SceneViewPost3DMode) Agros2D::problem()->setting()->value(ProblemSetting::View_ScalarView3DMode).toInt()).toStdString();
    }

};

struct PyViewParticleTracing
{
    void activate();
};

#endif // PYTHONLABVIEW_H
