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

#include "example.h"

#include "util.h"

FormExample::FormExample(QWidget *parent) : FormInterface(parent)
{
    actShow = new QAction(tr("Example"), this);
    connect(actShow, SIGNAL(triggered()), this, SLOT(show()));
}

FormExample::~FormExample()
{
}

QAction *FormExample::action()
{
    return actShow;
}

int FormExample::show()
{
    return exec();
}

void FormExample::acceptForm()
{   
    accept();
}

void FormExample::rejectForm()
{    
    reject();
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(agros2d_form_example, FormExample)
#endif
