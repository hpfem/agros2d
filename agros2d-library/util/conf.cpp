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

#include "config.h"
#include "scene.h"

#include "hermes2d/module.h"
#include "util/constants.h"

Config::Config()
{
    // set xml schemas dir
    Hermes::Hermes2D::Hermes2DApi.set_text_param_value(Hermes::Hermes2D::xmlSchemasDirPath, QString("%1/resources/xsd").arg(datadir()).toStdString());

    load();
}

Config::~Config()
{
    save();
}

void Config::load()
{
    QSettings settings;

    // std log
    showLogStdOut = settings.value("SceneViewSettings/LogStdOut", false).toBool();

    // general
    guiStyle = settings.value("General/GUIStyle").toString();
    language = settings.value("General/Language", QLocale::system().name()).toString();

    collaborationServerURL = settings.value("General/CollaborationServerURL", QString("http://agros2d.org/collaboration/")).toString();

    checkVersion = settings.value("General/CheckVersion", true).toBool();
    lineEditValueShowResult = settings.value("General/LineEditValueShowResult", false).toBool();

    // development
    saveMatrixRHS = settings.value("Solution/SaveMatrixAndRHS", SAVEMATRIXANDRHS).toBool();
    dumpFormat = (Hermes::Algebra::EMatrixDumpFormat) settings.value("Solution/FormatMatrixAndRHS", DF_MATLAB_SPARSE).toInt();

    // cache size
    cacheSize = settings.value("Solution/CacheSize", CACHE_SIZE).toInt();

    // solver cache
    useSolverCache = settings.value("Solution/UseSolverCache", USER_SOLVER_CACHE).toBool();

    // number of threads
    numberOfThreads = settings.value("Parallel/NumberOfThreads", omp_get_max_threads()).toInt();
    if (numberOfThreads > omp_get_max_threads())
        numberOfThreads = omp_get_max_threads();    
    Hermes::HermesCommonApi.set_integral_param_value(Hermes::numThreads, numberOfThreads);
}

void Config::save()
{
    QSettings settings;

    // std log
    settings.setValue("SceneViewSettings/LogStdOut", showLogStdOut);

    // general
    settings.setValue("General/GUIStyle", guiStyle);
    settings.setValue("General/Language", language);

    settings.setValue("General/CollaborationServerURL", collaborationServerURL);

    settings.setValue("General/CheckVersion", checkVersion);
    settings.setValue("General/LineEditValueShowResult", lineEditValueShowResult);

    // development
    settings.setValue("Solution/SaveMatrixAndRHS", saveMatrixRHS);
    settings.setValue("Solution/FormatMatrixAndRHS", dumpFormat);

    // cache size
    settings.setValue("Solution/CacheSize", cacheSize);

    // solver cache
    settings.setValue("Solution/UseSolverCache", useSolverCache);

    // number of threads
    settings.setValue("Parallel/NumberOfThreads", numberOfThreads);
    Hermes::HermesCommonApi.set_integral_param_value(Hermes::numThreads, numberOfThreads);
}
