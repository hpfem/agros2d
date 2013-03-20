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

#ifndef GUI_GROUPBOX_H
#define GUI_GROUPBOX_H

#include <QGroupBox>

class CollapsableGroupBoxButton : public QGroupBox
{
    Q_OBJECT

public:
    CollapsableGroupBoxButton(QWidget *parent = 0);
    CollapsableGroupBoxButton(const QString &title, QWidget *parent = 0);

    inline bool isCollapsed() { return m_collapsed; }
    void setCollapsed(bool collapsed);

signals:
    void collapseEvent(bool collapsed);

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *);

protected:
    // collapsed state
    bool m_collapsed;

private:
    // click position
    QPoint m_clickPos;
};


class CollapsableGroupBox : public CollapsableGroupBoxButton
{
    Q_OBJECT

public:
    CollapsableGroupBox(QWidget *parent = 0);
    CollapsableGroupBox(const QString &title, QWidget *parent = 0);

private slots:
    void collapsed(bool collapsed);
};

#endif // GUI_GROUPBOX_H
