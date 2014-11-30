// This plugin is part of Agros2D.
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

#include "memory_monitor.h"

#include "util.h"
#include "util/system_utils.h"

MemoryMonitor::MemoryMonitor()
{
    // memory usage
    m_memoryTimer = new QTimer(this);
    m_memoryTimer->start(1000);

    m_memoryTimer->connect(m_memoryTimer, SIGNAL(timeout()), this, SLOT(refreshMemory()));
}

MemoryMonitor::~MemoryMonitor()
{
    delete m_memoryTimer;
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
    return (SystemUtils::getCurrentRSS() / 1024 / 1024);
}

int MemoryMonitor::lastMemoryUsage() const
{
    return (m_memoryUsage.isEmpty()) ? 0 : m_memoryUsage.last();
}

int MemoryMonitor::appTime() const
{
    return (m_memoryTime.isEmpty()) ? 0 : m_memoryTime.last();
}
