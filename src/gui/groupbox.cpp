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

#include "groupbox.h"

#include "util.h"

CollapsableGroupBoxButton::CollapsableGroupBoxButton(QWidget *parent)
    : QGroupBox(parent), m_collapsed(false)
{   
}

CollapsableGroupBoxButton::CollapsableGroupBoxButton(const QString &title, QWidget *parent)
    : QGroupBox(title, parent), m_collapsed(false)
{
}

void CollapsableGroupBoxButton::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        QStyleOptionGroupBox option;
        initStyleOption(&option);
        QRect buttonArea(0, 0, 32, 20);
        buttonArea.moveTopRight(option.rect.adjusted(0, -2, -10, 0).topRight());
        if (buttonArea.contains(e->pos()))
        {
            m_clickPos = e->pos();
            return;
        }
    }
    QGroupBox::mousePressEvent(e);
}

void CollapsableGroupBoxButton::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && m_clickPos == e->pos())
        setCollapsed(!isCollapsed());
}

void CollapsableGroupBoxButton::paintEvent(QPaintEvent *)
{
    QStylePainter paint(this);
    QStyleOptionGroupBox option;
    initStyleOption(&option);
    paint.drawComplexControl(QStyle::CC_GroupBox, option);
    paint.drawItemPixmap(option.rect.adjusted(0, 2, -10, 0),
                         Qt::AlignTop | Qt::AlignRight,
                         QPixmap(m_collapsed ?
                                     icon("edit-right").pixmap(16, 16) :
                                     icon("edit-down").pixmap(16, 16)));
}

void CollapsableGroupBoxButton::setCollapsed(bool collapse)
{
    m_collapsed = collapse;

    if (isVisible())
    {
        // emit signal
        emit collapseEvent(collapse);

        // force paint
        repaint();
    }
}

// **************************************************************

CollapsableGroupBox::CollapsableGroupBox(QWidget *parent)
    : CollapsableGroupBoxButton(parent)
{
    connect(this, SIGNAL(collapseEvent(bool)), this, SLOT(collapsed(bool)));
}

CollapsableGroupBox::CollapsableGroupBox(const QString &title, QWidget *parent)
    : CollapsableGroupBoxButton(title, parent)
{
    connect(this, SIGNAL(collapseEvent(bool)), this, SLOT(collapsed(bool)));
}

void CollapsableGroupBox::collapsed(bool collapsed)
{
    foreach (QWidget *widget, findChildren<QWidget*>())
        widget->setVisible(!collapsed);
}
