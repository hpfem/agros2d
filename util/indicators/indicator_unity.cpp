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

#ifdef WITH_UNITY
#include <unity/unity/unity.h>
#endif

#include "indicator_unity.h"
#include "../util.h"

namespace Unity
{

#ifdef WITH_UNITY

static UnityLauncherEntry *entry;
void init()
{
    entry = unity_launcher_entry_get_for_desktop_file("agros2d.desktop");
}

bool isRunning()
{
    UnityInspector *inspector = unity_inspector_get_default();
    return unity_inspector_get_unity_running(inspector);
}

void openProgress()
{
    // qDebug() << "unity open";
    unity_launcher_entry_set_progress_visible(entry, true);
}

void closeProgress()
{
    // qDebug() << "unity close";
    unity_launcher_entry_set_progress_visible(entry, false);
}

void setProgress(double value)
{
    unity_launcher_entry_set_progress(entry, value);
}

#else

void init() {}
bool isRunning() { return false; }
void openProgress() {}
void closeProgress() {}
void setProgress(double value) {}

#endif
}
