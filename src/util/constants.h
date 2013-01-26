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

#include "enums.h"

const double SPEEDOFLIGHT = 299792458;

// constants
const QString IMAGEROOT = QString("..%1resources_source%1images").arg(QDir::separator());
const QString XSDROOT = QString("%1resources%1xsd").arg(QDir::separator());
const QString MODULEROOT = QString("%1resources%1modules").arg(QDir::separator());
const QString COUPLINGROOT = QString("%1resources%1couplings").arg(QDir::separator());
const QString TEMPLATEROOT = QString("%1resources%1templates").arg(QDir::separator());

const QColor COLORBACKGROUND = QColor::fromRgb(255, 255, 255);
const QColor COLORGRID = QColor::fromRgb(200, 200, 200);
const QColor COLORCROSS = QColor::fromRgb(150, 150, 150);
const QColor COLORNODES = QColor::fromRgb(150, 0, 0);
const QColor COLOREDGES = QColor::fromRgb(0, 0, 150);
const QColor COLORLABELS = QColor::fromRgb(0, 150, 0);
const QColor COLORCONTOURS = QColor::fromRgb(0, 0, 0);
const QColor COLORVECTORS = QColor::fromRgb(0, 0, 0);
const QColor COLORINITIALMESH = QColor::fromRgb(250, 202, 119);
const QColor COLORSOLUTIONMESH = QColor::fromRgb(150, 70, 0);
const QColor COLORHIGHLIGHTED = QColor::fromRgb(250, 150, 0);
const QColor COLORSELECTED = QColor::fromRgb(150, 0, 0);
const QColor COLORCROSSED = QColor::fromRgb(255, 0, 0);
const QColor COLORNOTCONNECTED = QColor::fromRgb(255, 0, 0);

// workspace
const double GRIDSTEP = 0.05;
const bool SHOWGRID = true;
const bool SNAPTOGRID = true;

const QString RULERSFONT = "fontGoogleDroidMono10";
const QString POSTFONT = "fontGoogleDroidMono14";

const QString APPLICATION_FONTFAMILY = QApplication::font().family();
#ifdef Q_WS_WIN
    const int APPLICATION_FONTSIZE = QApplication::font().pointSize() + 4;
#endif
#ifdef Q_WS_X11
    const int APPLICATION_FONTSIZE = APPLICATION_FONTSIZE;
#endif
#ifdef Q_WS_MAC
    const int APPLICATION_FONTSIZE = APPLICATION_FONTSIZE;
#endif

const bool SHOWAXES = true;
const bool SHOWRULERS = true;
const bool ZOOMTOMOUSE = true;

const int GEOMETRYNODESIZE = 6;
const int GEOMETRYEDGEWIDTH = 2;
const int GEOMETRYLABELSIZE = 6;

// mesh
const bool SHOWINITIALMESHVIEW = true;
const bool SHOWSOLUTIONMESHVIEW = false;
const int MESHANGLESEGMENTSCOUNT = 3;
const bool MESHCURVILINEARELEMENTS = true;

// discrete saving
const bool SAVEMATRIXANDRHS = false;
const QString DISCRETEDIRECTORY = "discrete";

// post2d
const int PALETTEENTRIES = 256;
const int PALETTESTEPSMIN = 3;
const int PALETTESTEPSMAX = PALETTEENTRIES;
const int SCALARDECIMALPLACEMIN = 0;
const int SCALARDECIMALPLACEMAX = 10;
const int CONTOURSCOUNTMIN = 1;
const int CONTOURSCOUNTMAX = 100;
const double CONTOURSWIDTH = 1.0;
const double CONTOURSWIDTHMIN = 0.1;
const double CONTOURSWIDTHMAX = 5.0;
const int VECTORSCOUNTMIN = 1;
const int VECTORSCOUNTMAX = 500;
const double VECTORSSCALEMIN = 0.1;
const double VECTORSSCALEMAX = 20.0;

// post3d
const SceneViewPost3DMode SCALARSHOWPOST3D = SceneViewPost3DMode_None;

const bool SHOWCONTOURVIEW = false;
const int CONTOURSCOUNT = 15;

const PaletteType PALETTETYPE = Palette_Agros2D;
const bool PALETTEFILTER = false;
const int PALETTESTEPS = 30;

const bool SHOWVECTORVIEW = false;
const bool VECTORPROPORTIONAL = true;
const bool VECTORCOLOR = true;
const int VECTORCOUNT = 50;
const double VECTORSCALE = 0.6;
const VectorType VECTORTYPE = VectorType_Arrow;
const VectorCenter VECTORCENTER = VectorCenter_Tail;

const bool SHOWORDERVIEW = true;
const bool SHOWORDERCOLORBAR = true;
const PaletteOrderType ORDERPALETTEORDERTYPE = PaletteOrder_Hermes;
const bool ORDERLABEL = false;

// particle
const bool SHOWPARTICLEVIEW = false;
const bool PARTICLEINCLUDERELATIVISTICCORRECTION = true;
const int PARTICLENUMBEROFPARTICLES = 5;
const double PARTICLESTARTINGRADIUS = 0;
const double PARTICLESTARTX = 0;
const double PARTICLESTARTY = 0;
const double PARTICLESTARTVELOCITYX = 0;
const double PARTICLESTARTVELOCITYY = 0;
const double PARTICLECUSTOMFORCEX = 0;
const double PARTICLECUSTOMFORCEY = 0;
const double PARTICLECUSTOMFORCEZ = 0;
const double PARTICLEMASS = 9.109e-31; // mass of the electron
const double PARTICLECONSTANT = 1.602e-19; // charge of the electron
const bool PARTICLEREFLECTONDIFFERENTMATERIAL = true;
const bool PARTICLEREFLECTONBOUNDARY = false;
const double PARTICLECOEFFICIENTOFRESTITUTION = 0.0;
const double PARTICLEMAXIMUMRELATIVEERROR = 0.01;
const bool PARTICLECOLORBYVELOCITY = true;
const bool PARTICLESHOWPOINTS = false;
const int PARTICLEMAXIMUMNUMBEROFSTEPS = 500;
const double PARTICLEMINIMUMSTEP = 0;
const double PARTICLEDRAGDENSITY = 1.2041;
const double PARTICLEDRAGCOEFFICIENT = 0.0;
const double PARTICLEDRAGREFERENCEAREA = 1e-6;

// scalarview
const PaletteQuality PALETTEQUALITY = PaletteQuality_Normal;

const bool SHOWSCALARVIEW = true;
const bool SHOWSCALARCOLORBAR = true;
const bool SCALARFIELDRANGELOG = false;
const int SCALARFIELDRANGEBASE = 10;
const int SCALARDECIMALPLACE = 2;
const bool SCALARRANGEAUTO = true;
const double SCALARRANGEMIN = 0.0;
const double SCALARRANGEMAX = 1.0;

const bool VIEW3DLIGHTING = false;
const double VIEW3DANGLE = 240.0;
const bool VIEW3DBACKGROUND = true;
const double VIEW3DHEIGHT = 4.0;
const bool VIEW3DBOUNDINGBOX = true;

const bool DEFORMSCALAR = true;
const bool DEFORMCONTOUR = true;
const bool DEFORMVECTOR = true;

// adaptivity
const bool ADAPTIVITY_ISOONLY = false;
const double ADAPTIVITY_CONVEXP = 1.0;
const double ADAPTIVITY_THRESHOLD = 0.3;
const int ADAPTIVITY_STRATEGY = 0;
const int ADAPTIVITY_MESHREGULARITY = -1;
const Hermes::Hermes2D::ProjNormType ADAPTIVITY_PROJNORMTYPE = Hermes::Hermes2D::HERMES_H1_NORM;
const bool ADAPTIVITY_ANISO = true;
const bool ADAPTIVITY_FINER_REFERENCE_H_AND_P = false;

// command argument
const QString COMMANDS_TRIANGLE = "%1 -p -P -q31.0 -e -A -a -z -Q -I -n -o2 \"%2\"";
const QString COMMANDS_GMSH = "%1 -2 \"%2.geo\"";
const QString COMMANDS_BUILD_PLUGIN = "./agros2d_plugin_compiler.sh %1";

// max dofs
const int MAX_DOFS = 60e3;

// cache size
const int CACHE_SIZE = 10;

const int NOT_FOUND_SO_FAR = -999;

#endif // UTIL_CONSTANTS_H

