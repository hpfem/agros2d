// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#ifndef SYSTEM_UTILS_H
#define SYSTEM_UTILS_H

#include "../util.h"

struct AGROS_LIBRARY_API CleanExit
{
    CleanExit();

    static void exitQt(int sig);
};


namespace SystemUtils
{

// Returns the peak (maximum so far) resident set size (physical
// memory use) measured in bytes, or zero if the value cannot be
// determined on this OS.
AGROS_UTIL_API long getPeakRSS();

// Returns the current resident set size (physical memory use) measured
// in bytes, or zero if the value cannot be determined on this OS.
AGROS_UTIL_API long getCurrentRSS();

AGROS_UTIL_API bool isProcessRunning(int pid);

AGROS_UTIL_API QString cpuType();
AGROS_UTIL_API int numberOfThreads();
AGROS_UTIL_API size_t totalMemorySize();
AGROS_UTIL_API QString operatingSystem();

}

#endif // SYSTEM_UTILS_H
