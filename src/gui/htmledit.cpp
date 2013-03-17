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

#include "htmledit.h"

#include "util.h"

HtmlValueEdit::HtmlValueEdit(QWidget *parent, const QString &str)
    : QWidget(parent)
{
    // create controls
    txtLineEdit = new QLineEdit(this);
    txtLineEdit->setToolTip(tr("This textedit allows using HTML entities."));
    txtLineEdit->setText(str);
    connect(txtLineEdit, SIGNAL(textChanged(QString)), this, SLOT(evaluate()));
    connect(txtLineEdit, SIGNAL(editingFinished()), this, SLOT(evaluate()));

    lblValue = new QLabel(this);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->addWidget(txtLineEdit, 1);
    layout->addWidget(lblValue, 0, Qt::AlignRight);

    setLayout(layout);

    evaluate();
}

bool HtmlValueEdit::evaluate()
{
    lblValue->setText(txtLineEdit->text());
    return true;
}

