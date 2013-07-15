// This plugin is part of Agros2D.
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

#include "memory_monitor.h"

#include "util.h"
#include "util/system_utils.h"

MemoryMonitor::MemoryMonitor()
{
    // memory usage
    m_memoryTimer = new QTimer();
    m_memoryTimer->start(1000);

    m_memoryTimer->connect(m_memoryTimer, SIGNAL(timeout()), this, SLOT(refreshMemory()));
}

void MemoryMonitor::refreshMemory()
{
    int memory = currentMemoryUsage();
    // append memory to map
    m_memoryTime.append(((m_memoryTime.isEmpty()) ? 0 : m_memoryTime.last()) + interval() / 1000);
    m_memoryUsage.append(memory);

    emit refreshMemory(memory);
}

int MemoryMonitor::currentMemoryUsage() const
{
    return (getCurrentRSS() / 1024 / 1024);
}

int MemoryMonitor::lastMemoryUsage() const
{
    return (m_memoryUsage.isEmpty()) ? 0 : m_memoryUsage.last();
}

int MemoryMonitor::appTime() const
{
    return (m_memoryTime.isEmpty()) ? 0 : m_memoryTime.last();
}
