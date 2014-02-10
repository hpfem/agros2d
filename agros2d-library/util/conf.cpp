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
#include "util/global.h"

Config::Config()
{
    // set xml schemas dir
    Hermes::Hermes2D::Hermes2DApi.set_text_param_value(Hermes::Hermes2D::xmlSchemasDirPath, QString("%1/resources/xsd").arg(datadir()).toStdString());

    setStringKeys();
    setDefaultValues();

    clear();

    load();
}

Config::~Config()
{
    save();
}

void Config::clear()
{
    // set default values and types
    m_setting.clear();
    setDefaultValues();

    m_setting = m_settingDefault;
}

void Config::load()
{
    // default
    m_setting = m_settingDefault;

    QSettings settings;

    // std log
    m_setting[Config_LogStdOut] = settings.value(m_settingKey[Config_LogStdOut], m_settingDefault[Config_LogStdOut]).toBool();

    // general
    m_setting[Config_GUIStyle] = settings.value(m_settingKey[Config_GUIStyle], m_settingDefault[Config_GUIStyle]).toString();
    m_setting[Config_Locale] = settings.value(m_settingKey[Config_Locale], m_settingDefault[Config_Locale]).toString();

    m_setting[Config_ShowResults] = settings.value(m_settingKey[Config_ShowResults], m_settingDefault[Config_ShowResults]).toBool();

    // development
    m_setting[Config_LinearSystemSave] = settings.value(m_settingKey[Config_LinearSystemSave], m_settingDefault[Config_LinearSystemSave]).toBool();
    m_setting[Config_LinearSystemFormat] = (Hermes::Algebra::MatrixExportFormat) settings.value(m_settingKey[Config_LinearSystemFormat], m_settingDefault[Config_LinearSystemFormat]).toInt();

    // cache size
    m_setting[Config_CacheSize] = settings.value(m_settingKey[Config_CacheSize], m_settingDefault[Config_CacheSize]).toInt();

    // number of threads
    m_setting[Config_NumberOfThreads] = settings.value(m_settingKey[Config_NumberOfThreads], m_settingDefault[Config_NumberOfThreads]).toInt();
    if (m_setting[Config_NumberOfThreads].toInt() > omp_get_max_threads())
        m_setting[Config_NumberOfThreads] = omp_get_max_threads();
    Hermes::HermesCommonApi.set_integral_param_value(Hermes::numThreads, m_settingKey[Config_NumberOfThreads].toInt());

    m_setting[Config_ShowGrid] = settings.value(m_settingKey[Config_ShowGrid], m_settingDefault[Config_ShowGrid]).toBool();
    m_setting[Config_ShowRulers] = settings.value(m_settingKey[Config_ShowRulers], m_settingDefault[Config_ShowRulers]).toBool();
    m_setting[Config_ShowAxes] = settings.value(m_settingKey[Config_ShowAxes], m_settingDefault[Config_ShowAxes]).toBool();

    m_setting[Config_RulersFontFamily] = settings.value(m_settingKey[Config_RulersFontFamily], m_settingDefault[Config_RulersFontFamily]).toString();
    m_setting[Config_RulersFontPointSize] = settings.value(m_settingKey[Config_RulersFontPointSize], m_settingDefault[Config_RulersFontPointSize]).toInt();
    m_setting[Config_PostFontFamily] = settings.value(m_settingKey[Config_PostFontFamily], m_settingDefault[Config_PostFontFamily]).toString();
    m_setting[Config_PostFontPointSize] = settings.value(m_settingKey[Config_PostFontPointSize], m_settingDefault[Config_PostFontPointSize]).toInt();
}

void Config::save()
{
    QSettings settings;

    foreach (Type key, m_setting.keys())
        settings.setValue(m_settingKey[key], m_setting[key]);

    // number of threads
    Hermes::HermesCommonApi.set_integral_param_value(Hermes::numThreads, m_settingKey[Config_NumberOfThreads].toInt());
}

void Config::setStringKeys()
{
    m_settingKey[Config_LogStdOut] = "Config_LogStdOut";
    m_settingKey[Config_GUIStyle] = "Config_GUIStyle";
    m_settingKey[Config_Locale] = "Config_Locale";
    m_settingKey[Config_ShowResults] = "Config_ShowResults";
    m_settingKey[Config_LinearSystemFormat] = "Config_LinearSystemFormat";
    m_settingKey[Config_LinearSystemSave] = "Config_LinearSystemSave";
    m_settingKey[Config_CacheSize] = "Config_CacheSize";
    m_settingKey[Config_NumberOfThreads] = "Config_NumberOfThreads";
    m_settingKey[Config_ShowGrid] = "Config_ShowGrid";
    m_settingKey[Config_ShowRulers] = "Config_ShowRulers";
    m_settingKey[Config_ShowAxes] = "Config_ShowAxes";
    m_settingKey[Config_RulersFontFamily] = "Config_RulersFontFamily";
    m_settingKey[Config_RulersFontPointSize] = "Config_RulersFontPointSize";
    m_settingKey[Config_PostFontFamily] = "Config_PostFontFamily";
    m_settingKey[Config_PostFontPointSize] = "Config_PostFontPointSize";
}

void Config::setDefaultValues()
{
    m_settingDefault.clear();

    m_settingDefault[Config_LogStdOut] = false;
    m_settingDefault[Config_GUIStyle] = defaultGUIStyle();
    m_settingDefault[Config_Locale] = defaultLocale();
    m_settingDefault[Config_ShowResults] = false;
    m_settingDefault[Config_LinearSystemFormat] = EXPORT_FORMAT_MATLAB_MATIO;
    m_settingDefault[Config_LinearSystemSave] = false;
    m_settingDefault[Config_CacheSize] = 10;
    m_settingDefault[Config_NumberOfThreads] = omp_get_max_threads();;
    m_settingDefault[Config_ShowGrid] = true;
    m_settingDefault[Config_ShowRulers] = true;
    m_settingDefault[Config_ShowAxes] = true;
    m_settingDefault[Config_RulersFontFamily] = QString("Droid");
    m_settingDefault[Config_RulersFontPointSize] = 12;
    m_settingDefault[Config_PostFontFamily] = QString("Droid");
    m_settingDefault[Config_PostFontPointSize] = 16;
}
