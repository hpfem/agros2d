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

#ifndef MEMORY_MONITOR_H
#define MEMORY_MONITOR_H

#include "util.h"

class AGROS_LIBRARY_API MemoryMonitor : public QObject
{
    Q_OBJECT

public:
    MemoryMonitor();
    ~MemoryMonitor();

    int currentMemoryUsage() const;
    int lastMemoryUsage() const;
    int appTime() const;
    inline QList<int> memoryTime() const { return m_memoryTime; }
    inline QList<int> memoryUsage() const { return m_memoryUsage; }

    inline void setInterval(int inter) { m_memoryTimer->setInterval(inter); }
    inline int interval() { return m_memoryTimer->interval(); }

signals:
    void refreshMemory(int usage);

private:
    QTimer *m_memoryTimer;
    QList<int> m_memoryTime;
    QList<int> m_memoryUsage;

private slots:
    void refreshMemory();
};


#endif /* MEMORY_MONITOR_H */
