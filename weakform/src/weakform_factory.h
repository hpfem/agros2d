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

#ifndef WEAKFORM_FACTORY_H
#define WEAKFORM_FACTORY_H

#include "util.h"
#include "hermes2d.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenemarker.h"

#include "hermes2d/localpoint.h"
#include "hermes2d/volumeintegral.h"
#include "hermes2d/surfaceintegral.h"

#include <rapidxml.cpp>
#include <rapidxml_utils.cpp>

#include "../src/hermes2d/localpoint.h"
#include "../src/hermes2d/weakform_parser.h"

template <typename Scalar>
Hermes::Hermes2D::MatrixFormVol<Scalar> *factoryMatrixFormVol(const std::string &problemId, int i, int j,
                                                              const std::string &area, Hermes::Hermes2D::SymFlag sym,
                                                              SceneMaterial *material)
{
     
    return NULL;
}

template <typename Scalar>
Hermes::Hermes2D::VectorFormVol<Scalar> *factoryVectorFormVol(const std::string &problemId, int i, int j,
                                                              const std::string &area, SceneMaterial *material)
{
     
    return NULL;
}

template <typename Scalar>
Hermes::Hermes2D::MatrixFormSurf<Scalar> *factoryMatrixFormSurf(const std::string &problemId, int i, int j,
                                                                const std::string &area, SceneBoundary *boundary)
{
     
    return NULL;
}

template <typename Scalar>
Hermes::Hermes2D::VectorFormSurf<Scalar> *factoryVectorFormSurf(const std::string &problemId, int i, int j,
                                                                const std::string &area, SceneBoundary *boundary)
{
     
    return NULL;
}

#endif
