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

#include "pythonlab/pythonengine_agros.h"

#include "pythonlab/python_unittests.h"

UnitTestsWidget::UnitTestsWidget(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Unit tests"));
    setModal(true);

    // problem information
    lstLog = new QPlainTextEdit(this);
    lstLog->setReadOnly(true);

    trvTests = new QTreeWidget(this);
    trvTests->setMouseTracking(true);
    trvTests->setColumnCount(1);
    trvTests->setIndentation(15);
    trvTests->setIconSize(QSize(24, 24));
    trvTests->setHeaderHidden(true);
    trvTests->setMinimumWidth(320);

    // connect(trvTests, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doItemDoubleClicked(QTreeWidgetItem *, int)));
    // connect(trvTests, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    QHBoxLayout *layoutSurface = new QHBoxLayout();
    layoutSurface->addWidget(trvTests);
    layoutSurface->addWidget(lstLog, 1);

    QWidget *widget = new QWidget();
    widget->setLayout(layoutSurface);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(widget, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    QSettings settings;
    restoreGeometry(settings.value("UnitTestsWidget/Geometry", saveGeometry()).toByteArray());

    readTests();
}

UnitTestsWidget::~UnitTestsWidget()
{
    QSettings settings;
    settings.setValue("UnitTestsWidget/Geometry", saveGeometry());
}

void UnitTestsWidget::doReject()
{
    accept();
}

void UnitTestsWidget::readTests()
{
    trvTests->clear();

    QFont fnt = trvTests->font();
    fnt.setBold(true);

    // run expression
    currentPythonEngine()->runExpression(QString("import test_suite; agros2d_tests = []; agros2d.agros2d_find_all_tests(test_suite, agros2d_tests)"));

    // extract values
    PyObject *result = PyDict_GetItemString(currentPythonEngine()->dict(), "agros2d_tests");
    if (result)
    {
        Py_INCREF(result);
        for (int i = 0; i < PyList_Size(result); i++)
        {
            PyObject *list = PyList_GetItem(result, i);
            Py_INCREF(list);

            QString name = PyString_AsString(PyList_GetItem(list, 0));
            QString module = PyString_AsString(PyList_GetItem(list, 1));

            QTreeWidgetItem *classItem = new QTreeWidgetItem(trvTests);
            classItem->setText(0, name);
            classItem->setFont(0, fnt);
            classItem->setExpanded(true);

            PyObject *tests = PyList_GetItem(list, 2);
            Py_INCREF(tests);
            for (int j = 0; j < PyList_Size(tests); j++)
            {
                PyObject *item = PyList_GetItem(tests, j);
                Py_INCREF(item);

                QString test = PyString_AsString(item);

                QTreeWidgetItem *testItem = new QTreeWidgetItem(classItem);
                testItem->setText(0, test);
                testItem->setData(0, Qt::UserRole, module);
                testItem->setCheckState(0, Qt::Unchecked);

                Py_XDECREF(item);
            }

            Py_XDECREF(tests);
            Py_XDECREF(list);

        }
        Py_XDECREF(result);
    }

    // remove variables
    currentPythonEngine()->runExpression("del agros2d_tests");
}
