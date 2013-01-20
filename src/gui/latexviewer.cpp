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

#include "latexviewer.h"

#include "util.h"
#include "util/constants.h"

#include "ctemplate/template.h"

LaTeXViewer::LaTeXViewer(QWidget *parent, const QString &str) : QWebView(parent)
{
    setAcceptDrops(false);
    setContextMenuPolicy(Qt::NoContextMenu);
    setContentsMargins(0, 0, 0, 0);
    setMaximumHeight(70);
    setMaximumHeight(70);
    setMinimumWidth(350);
    page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

    connect(this, SIGNAL(loadFinished(bool)), SLOT(finishLoading(bool)));

    setLatex(str);
}

QSize LaTeXViewer::sizeHint() const
{
    return QSize(350, 70);
}

QString LaTeXViewer::latex()
{
    return m_latexSource;
}

void LaTeXViewer::setLatex(const QString &str)
{
    m_latexSource = str;

    QTimer::singleShot(0, this, SLOT(showLaTeX()));
}

void LaTeXViewer::showLaTeX()
{
    if (m_latexSource.isEmpty())
        return;

    setVisible(false);

    m_latexSource.replace("\\div", "\\mathrm{div}\\,");
    m_latexSource.replace("\\grad", "\\mathrm{grad}\\,");
    m_latexSource.replace("\\rot", "\\mathrm{rot}\\,");
    m_latexSource.replace("\\curl", "\\mathrm{curl}\\,");
    m_latexSource.replace("\\vec{", "\\mathbf{");
    m_latexSource.replace("\\faz{", "\\overline{");
    m_latexSource.replace("\\vecfaz{", "\\mathbf{\\overline ");
    m_latexSource.replace("\\mj", "\\mathrm{j}\\,");

    // template
    std::string info;
    ctemplate::TemplateDictionary problemInfo("latex");

    problemInfo.SetValue("BACKGROUND", QString("rgb(%1, %2, %3)")
                         .arg(palette().background().color().red())
                         .arg(palette().background().color().green())
                         .arg(palette().background().color().blue()).toStdString());
    problemInfo.SetValue("PANELS_DIRECTORY", datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels");
    problemInfo.SetValue("LATEX_SOURCE", m_latexSource.toStdString());

    ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/latex.tpl", ctemplate::DO_NOT_STRIP, &problemInfo, &info);

    writeStringContent(tempProblemDir() + "/latex.html", QString::fromStdString(info));

    // load page
    load(tempProblemDir() + "/latex.html");

}

void LaTeXViewer::finishLoading(bool ok)
{
    setVisible(ok);
}

