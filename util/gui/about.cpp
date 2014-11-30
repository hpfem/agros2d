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

#include "about.h"

#include "../util.h"
#include "../util/checkversion.h"
#include "../util/system_utils.h"

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent)
{
    setWindowModality(Qt::ApplicationModal);
    setModal(true);

    setWindowIcon(icon("agros2d"));
    setWindowTitle(tr("About..."));

    createControls();

    setMinimumSize(450, 550);
    setMaximumSize(sizeHint());
}

AboutDialog::~AboutDialog()
{

}

void AboutDialog::createControls()
{
    QTabWidget *tab = new QTabWidget();
    tab->addTab(createAgros2D(), icon(""), tr("Agros2D"));
    tab->addTab(createDealii(), tr("deal.ii"));
    tab->addTab(createLibraries(), tr("Libraries"));
    tab->addTab(createLicense(), tr("License"));
    tab->addTab(createSysinfo(), tr("System Informations"));

    QPushButton *buttonClose = new QPushButton(tr("Close"));
    QPushButton *buttonCheckForNewVersion = new QPushButton(tr("Check version"));

    QHBoxLayout *buttonBox = new QHBoxLayout();
    buttonBox->addStretch();
    buttonBox->addWidget(buttonCheckForNewVersion);
    buttonBox->addWidget(buttonClose);

    connect(buttonClose, SIGNAL(clicked()), this, SLOT(close()));
    connect(buttonCheckForNewVersion, SIGNAL(clicked()), this, SLOT(checkVersion()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tab);
    // layout->addStretch();
    layout->addLayout(buttonBox);

    setLayout(layout);
}

QWidget *AboutDialog::createAgros2D()
{
    QLabel *labelContent = new QLabel(tr("<h3>Agros2D %1</h3>"
                                         "Agros2D is a multiplatform multiphysics application for the solution of partial differential equations (PDE) "
                                         "based on the <a href=\"http://dealii.org/\">deal.ii</a> library using higher order finite element method "
                                         "(<i>hp</i>-FEM) with automatic adaptivity.<br/><br/>"
                                         "Web page: <a href=\"http://agros2d.org/\">http://agros2d.org/</a><br/>"
                                         "Facebook: <a href=\"http://www.facebook.com/pages/Agros2D/132524130149770?sk=info\">http://www.facebook.com/pages/Agros2D/...</a><br/>"
                                         "<br/>"
                                         "<b>Authors:</b>"
                                         "<p>"
                                         "<b>Pavel Karban</b> - main developer (University of West Bohemia, Pilsen, Czech Republic)<br/>"
                                         "<b>František Mach</b> - developer, Python script (University of West Bohemia, Pilsen, Czech Republic)<br/>"
                                         "<b>Pavel Kůs</b> - developer, coupled problems, time domain adaptivity (University of West Bohemia, Pilsen, Czech Republic)<br/>"
                                         "<b>David Pánek</b> - developer (University of West Bohemia, Pilsen, Czech Republic)<br/>"
                                         "<b>Lukáš Korous</b> - developer (University of West Bohemia, Pilsen, Czech Republic)<br/>"
                                         "<b>Lukáš Koudela</b> - developer - RF module (University of West Bohemia, Pilsen, Czech Republic)<br/>"
                                         "<b>Bartosz Sawicki</b> - Polish translation (Warsaw University of Technology, Warsaw)<br/>"
                                         "<b>Václav Kotlan</b> - German translation (University of West Bohemia, Pilsen, Czech Republic)<br/>"
                                         "<b>Petr Kropík</b> - University of West Bohemia, Pilsen, Czech Republic<br/>"
                                         "<b>Denys Nikolayev</b> - Russion and French translation, University of West Bohemia, Pilsen, Czech Republic<br/>"
                                         "</p>")
                                      .arg(QApplication::applicationVersion()));
    labelContent->setWordWrap(true);
    labelContent->setOpenExternalLinks(true);


    QLabel *labelIcon = new QLabel();
    labelIcon->setPixmap(icon("agros2d").pixmap(64, 64));

    QGridLayout *layoutIcon = new QGridLayout();
    layoutIcon->addWidget(labelIcon, 0, 0, 1, 1, Qt::AlignTop);
    layoutIcon->addWidget(labelContent, 0, 1);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutIcon);
    layout->addStretch();

    QWidget *widget = new QWidget();
    widget->setLayout(layout);

    return widget;
}

QWidget *AboutDialog::createDealii()
{
    QLabel *labelContent = new QLabel(tr("<h3>dealii</h3>"
                                         "Web page: <a href=\"http://dealii.org/\">http://dealii.org/</a><br/><br/>"
                                         "<br/>"
                                         "<b>Authors:</b>"
                                         "<p>"
                                         "<b>Principal developers</b><br/>"
                                         "Wolfgang Bangerth, Texas A&M University, TX, USA<br/>"
                                         "Timo Heister, Clemson University, SC, USA<br/>"
                                         "Guido Kanschat, Universität Heidelberg, Germany<br/>"
                                         "Matthias Maier, Universität Heidelberg, Germany<br/>"
                                         "<br/>"
                                         "<b>Developers</b><br/>"
                                         "Luca Heltai, SISSA, Trieste, Italy<br/>"
                                         "Martin Kronbichler, Technische Universität München, Germany<br/>"
                                         "Bruno Turcksin, Texas A&M University, TX, USA<br/>"
                                         "Toby D. Young, Polish Academy of Sciences, Poland<br/>"
                                         "</p>"
                                         ));
    labelContent->setWordWrap(true);
    labelContent->setOpenExternalLinks(true);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(labelContent);
    layout->addStretch();

    QWidget *widget = new QWidget();
    widget->setLayout(layout);

    return widget;
}

QWidget *AboutDialog::createLibraries()
{
    QLabel *labelContent = new QLabel(tr("<h3>Libraries</h3>"
                                         "<b>Digia Qt:</b> Digia Qt (<a href=\"http://qt.digia.com/\">Qt - Cross-platform application and UI development framework</a>)<br/>"
                                         "<b>QCustomChart:</b> QCustomChart (<a href=\"http://www.workslikeclockwork.com/index.php/components/qt-plotting-widget/\">Qt Plotting Widget</a>)<br/>"
                                         "<b>Python:</b> Python Programming Language (<a href=\"http://www.python.org\">Python</a>)<br/>"
                                         "<b>libdxfrw:</b> LibreCAD DXF library (<a href=\"http://sourceforge.net/projects/libdxfrw/\">libdxfrw</a>)<br/>"
                                         "<b>ctemplate:</b> Simple but powerful template language for C++ (<a href=\"http://code.google.com/p/ctemplate/\">ctemplate</a>)<br/>"
                                         "<b>matio:</b> MAT File I/O Library (<a href=\"http://sourceforge.net/projects/matio/\">matio</a>)<br/>"
                                         "<b>poly2tri:</b> A 2D constrained Delaunay triangulation library (<a href=\"http://code.google.com/p/poly2tri/\">poly2tri</a>)<br/>"
                                         "<b>stb_truetype.h:</b> TrueType processing library - Sean Barrett / RAD Game Tools (<a href=\"http://nothings.org/\">stb</a>)<br/>"
                                         "<b>Triangle:</b> Jonathan Richard Shewchuk (<a href=\"http://www.cs.cmu.edu/~quake/triangle.html\">Triangle</a>)<br/>"
                                         "<b>Gmsh:</b> A three-dimensional finite element mesh generator (<a href=\"http://geuz.org/gmsh\">Gmsh</a>)<br/>"
                                         "<b>UMFPACK:</b> unsymmetric multifrontal sparse LU factorization package (<a href=\"http://www.cise.ufl.edu/research/sparse/umfpack/\">UMFPACK</a>)<br/>"
                                         "<b>MUMPS:</b> A MUltifrontal Massively Parallel sparse direct Solver (<a href=\"http://graal.ens-lyon.fr/MUMPS/\">MUMPS</a>)<br/>"
                                         "<b>SuperLU:</b> Direct solver for large sparse systems of linear equations (<a href=\"http://crd-legacy.lbl.gov/~xiaoye/SuperLU/\">SuperLU</a>)<br/>"
                                         "<b>PARALUTION:</b> Sparse iterative solvers on multi-core CPU and GPU devices (<a href=\"http://www.paralution.com/\">PARALUTION</a>)<br/>"
                                         ));
    labelContent->setWordWrap(true);
    labelContent->setOpenExternalLinks(true);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(labelContent);
    layout->addStretch();

    QWidget *widget = new QWidget();
    widget->setLayout(layout);

    return widget;
}

QWidget *AboutDialog::createLicense()
{
    QLabel *labelContent = new QLabel(tr("<h3>License:</h3>"
                                         "<p>Agros is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.</p><p>Agros is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.</p><p>You should have received a copy of the GNU General Public License along with Agros. If not, see <a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>."
                                         ));
    labelContent->setWordWrap(true);
    labelContent->setOpenExternalLinks(true);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(labelContent);
    layout->addStretch();

    QWidget *widget = new QWidget();
    widget->setLayout(layout);

    return widget;
}

QWidget *AboutDialog::createSysinfo()
{
    QGridLayout *layout = new QGridLayout();
    layout->addWidget(new QLabel(tr("OS:")), 0, 0);
    layout->addWidget(new QLabel(SystemUtils::operatingSystem()), 0, 1);
    layout->addWidget(new QLabel(""), 9, 0);
    layout->addWidget(new QLabel(tr("Processor:")), 10, 0);
    layout->addWidget(new QLabel(SystemUtils::cpuType()), 10, 1);
    layout->addWidget(new QLabel(tr("Number of threads:")), 11, 0);
    layout->addWidget(new QLabel(QString::number(SystemUtils::numberOfThreads())), 11, 1);
    layout->addWidget(new QLabel(tr("Memory:")), 12, 0);
    layout->addWidget(new QLabel(QString("%1 MB").arg(SystemUtils::totalMemorySize() / 1024 / 1024)), 12, 1);
    layout->addWidget(new QLabel(tr("Screen resolution:")), 13, 0);
    layout->addWidget(new QLabel(QString("%1 x %2").
                                 arg(QApplication::desktop()->screenGeometry().width()).
                                 arg(QApplication::desktop()->screenGeometry().height())), 13, 1);
    layout->setRowStretch(20, 1);

    QWidget *widget = new QWidget();
    widget->setLayout(layout);

    return widget;
}

void AboutDialog::checkVersion()
{
    checkForNewVersion();
}
