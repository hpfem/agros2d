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

#ifndef UTIL_CONSTANTS_H
#define UTIL_CONSTANTS_H

#include <QtCore>
#include <QApplication>
#include <QFont>

const double SPEEDOFLIGHT = 299792458;

// constants
const QString IMAGEROOT = QString("..%1resources_source%1images").arg(QDir::separator());
const QString XSDROOT = QString("%1resources%1xsd").arg(QDir::separator());
const QString MODULEROOT = QString("%1resources%1modules").arg(QDir::separator());
const QString COUPLINGROOT = QString("%1resources%1couplings").arg(QDir::separator());
const QString TEMPLATEROOT = QString("%1resources%1templates").arg(QDir::separator());

// discrete saving
const bool SAVEMATRIXANDRHS = false;
const QString DISCRETEDIRECTORY = "discrete";

const bool EXPERIMENTALFEATURES = false;

const int PALETTEENTRIES = 256;
const double VECTORSSCALEMIN = 0.1;
const double VECTORSSCALEMAX = 20.0;
const int SCALARDECIMALPLACEMIN = 0;
const int SCALARDECIMALPLACEMAX = 10;
const double CONTOURSWIDTHMIN = 0.1;
const double CONTOURSWIDTHMAX = 5.0;
const int VECTORSCOUNTMIN = 1;
const int VECTORSCOUNTMAX = 500;
const int CONTOURSCOUNTMIN = 1;
const int CONTOURSCOUNTMAX = 100;
const int PALETTESTEPSMIN = 3;
const int PALETTESTEPSMAX = PALETTEENTRIES;

// command argument
const QString COMMANDS_BUILD_PLUGIN = "./agros2d_plugin_compiler.sh %1";

// cache size
const int CACHE_SIZE = 10;

// solver cache
const bool USER_SOLVER_CACHE = false;

const int NOT_FOUND_SO_FAR = -999;

const int GLYPH_M = 77;

const double COLORBACKGROUND[3] = { 255 / 255.0, 255 / 255.0, 255 / 255.0 };
const double COLORGRID[3] = { 200 / 255.0, 200 / 255.0, 200 / 255.0 };
const double COLORCROSS[3] = { 150 / 255.0, 150 / 255.0, 150 / 255.0 };
const double COLORNODE[3] = { 150 / 255.0, 0 / 255.0, 0 / 255.0 };
const double COLOREDGE[3] = { 0 / 255.0, 0 / 255.0, 150 / 255.0 };
const double COLORLABEL[3] = { 0 / 255.0, 150 / 255.0, 0 / 255.0 };
const double COLORSELECTED[3] = { 0 / 255.0, 0 / 255.0, 0 / 255.0 };
const double COLORHIGHLIGHTED[3] = { 250 / 255.0, 150 / 255.0, 0 / 255.0 };
const double COLORCROSSED[3] = { 255 / 255.0, 0 / 255.0, 0 / 255.0 };
const double COLORNOTCONNECTED[3] = { 255 / 255.0, 0 / 255.0, 0 / 255.0 };

const double COLORCONTOURS[3] = { 0 / 255.0, 0 / 255.0, 0 / 255.0 };
const double COLORVECTORS[3] = { 0 / 255.0, 0 / 255.0, 0 / 255.0 };
const double COLORINITIALMESH[3] = { 250 / 255.0, 202 / 255.0, 119 / 255.0 };
const double COLORSOLUTIONMESH[3] = { 150 / 255.0, 70 / 255.0, 0 / 255.0 };

const int LABELSIZE = 6;
const int EDGEWIDTH = 2;
const int NODESIZE = 6;

#endif // UTIL_CONSTANTS_H

