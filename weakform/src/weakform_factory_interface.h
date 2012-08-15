#ifndef WEAKFORM_FACTORY_INTERFACE_H
#define WEAKFORM_FACTORY_INTERFACE_H
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

#include <QtPlugin>

#include "util.h"
#include "hermes2d.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"

#include "hermes2d/localpoint.h"
#include "hermes2d/volumeintegral.h"
#include "hermes2d/surfaceintegral.h"

#include "hermes2d/marker.h"
#include "hermes2d/localpoint.h"
#include "hermes2d/weakform_parser.h"

template <typename Scalar>
class WeakFormFactoryInterface
{
public:

    virtual ~WeakFormFactoryInterface() {}

    virtual Hermes::Hermes2D::MatrixFormVol<Scalar> *MatrixFormVol(const std::string &problemId, int i, int j,
                                                                   const std::string &area, Hermes::Hermes2D::SymFlag sym,
                                                                   SceneMaterial *materialSource, Material *materialTarget, int offsetI, int offsetJ ) = 0;
    virtual Hermes::Hermes2D::VectorFormVol<Scalar> *VectorFormVol(const std::string &problemId, int i, int j,
                                                                   const std::string &area, SceneMaterial *materialSource, Material *materialTarget, int offsetI, int offsetJ) = 0;

    virtual Hermes::Hermes2D::MatrixFormSurf<Scalar> *MatrixFormSurf(const std::string &problemId, int i, int j,
                                                                     const std::string &area, SceneBoundary *boundary, int offsetI, int offsetJ) = 0;


    virtual Hermes::Hermes2D::VectorFormSurf<Scalar> *VectorFormSurf(const std::string &problemId, int i, int j,
                                                                     const std::string &area, SceneBoundary *boundary, int offsetI, int offsetJ) = 0;

    virtual Hermes::Hermes2D::ExactSolutionScalar<Scalar> *ExactSolution(const std::string &problemId, int i,Hermes::Hermes2D::Mesh *mesh, Boundary *boundary) = 0;
};

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(WeakFormFactoryInterface<Scalar>,"com.trolltech.PlugAndPaint.BrushInterface/1.0")
QT_END_NAMESPACE

#endif // WEAKFORM_FACTORY_INTERFACE_H
