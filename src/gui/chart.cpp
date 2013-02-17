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

#include "chart.h"

#include "scene.h"
#include "util.h"

#include "hermes2d/module.h"

#include "hermes2d/solver.h"
#include "hermes2d/field.h"
#include "hermes2d/problem.h"
#include "hermes2d/solutionstore.h"

#include <qwt_scale_map.h>
#include <qwt_symbol.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_panner.h>
#include <qwt_counter.h>
#include <qwt_legend.h>
#include <qwt_text.h>
#include <qwt_scale_engine.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_rescaler.h>
#include <qwt_plot_curve.h>

ChartBasic::ChartBasic(QWidget *parent) : QwtPlot(parent)
{
    //  chart style
    setAutoReplot(false);
    setTitle("");
    setMinimumSize(420, 260);

    // panning with the left mouse button
    // QwtPlotPanner *panner = new QwtPlotPanner(canvas());
    // panner->setMouseButton(Qt::MidButton);

    // zoom in/out with the wheel
    // QwtPlotMagnifier *magnifier = new QwtPlotMagnifier(canvas());
    // magnifier->setWheelFactor(1 / magnifier->wheelFactor());
    // magnifier->setMouseFactor(1 / magnifier->mouseFactor());

    // QwtPlotZoomer *zoomer = new QwtPlotZoomer(canvas());
    // zoomer->setRubberBandPen(QColor(Qt::red));
    // zoomer->setTrackerPen(Qt::NoPen);
    // zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
    // zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);

    // legend
    /*
    QwtLegend *legend = new QwtLegend;
    legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
    insertLegend(legend, QwtPlot::BottomLegend);
    */

    // canvas
    QPalette canvasPalette(Qt::white);
    canvasPalette.setColor(QPalette::Foreground, QColor(0, 0, 0));
    canvas()->setPalette(canvasPalette);
    canvas()->setFrameShape(QFrame::NoFrame);

    // grid
    m_grid = new QwtPlotGrid();
    m_grid->setMajPen(QPen(Qt::gray, 0, Qt::DotLine));
    m_grid->setMinPen(QPen(Qt::NoPen));
    m_grid->enableX(true);
    m_grid->enableY(true);
    m_grid->enableXMin(true);
    m_grid->enableYMin(true);
    m_grid->attach(this);

    // axes
    QFont fnt = QFont(QApplication::font().family(), QApplication::font().pointSize() - 1, QFont::Normal);
    setAxisFont(QwtPlot::xBottom, fnt);
    setAxisTitle(QwtPlot::xBottom, QwtText(" "));
    setAxisFont(QwtPlot::yLeft, fnt);
    setAxisTitle(QwtPlot::yLeft, QwtText(" "));

    // curve styles
    QwtSymbol sym;

    sym.setStyle(QwtSymbol::Cross);
    sym.setPen(QColor(Qt::black));
    sym.setSize(5);

    // curve
    m_curve = new QwtPlotCurve();
    m_curve->setRenderHint(QwtPlotCurve::RenderAntialiased);
    m_curve->setPen(QPen(Qt::darkBlue, 1.5));
    m_curve->setCurveAttribute(QwtPlotCurve::Inverted);
    m_curve->setYAxis(QwtPlot::yLeft);
    m_curve->attach(this);


    // chart picker
    QwtPlotPicker *pickerValue = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                                                   QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
                                                   canvas());
    pickerValue->setRubberBandPen(QColor(Qt::green));
    pickerValue->setRubberBand(QwtPicker::CrossRubberBand);
    pickerValue->setTrackerMode(QwtPicker::AlwaysOn);
    pickerValue->setTrackerPen(QColor(Qt::black));

    connect(pickerValue, SIGNAL(moved(const QPoint &)), SLOT(pickerValueMoved(const QPoint &)));

    setMinimumSize(sizeHint());
}

ChartBasic::~ChartBasic()
{
    delete m_curve;
}

void ChartBasic::saveImage(const QString &fileName)
{
    QSettings settings;
    QString dir = settings.value("General/LastImageDir").toString();

    QString selectedFilter = "png";
    QString fileNameTemp;
    if (fileName.isEmpty())
    {
        const QList<QByteArray> imageFormats = QImageWriter::supportedImageFormats();

        QStringList filter;
        filter += "PDF Documents (*.pdf)";
#ifndef QWT_NO_SVG
        filter += "SVG Documents (*.svg)";
#endif
        filter += "Postscript Documents (*.ps)";
        filter += "Image (*.png)";

        /*
        if (imageFormats.size() > 0)
        {
            QString imageFilter("Images (");
            for ( int i = 0; i < imageFormats.size(); i++ )
            {
                if ( i > 0 )
                    imageFilter += " ";
                imageFilter += "*.";
                imageFilter += imageFormats[i];
            }
            imageFilter += ")";

            filter += imageFilter;
        }
        */

        fileNameTemp = QFileDialog::getSaveFileName(this, tr("Export image to file"), dir, filter.join(";;"),
                                                    &selectedFilter);
        selectedFilter = selectedFilter.mid(selectedFilter.indexOf("*.") + 2,
                                            selectedFilter.length() - selectedFilter.indexOf("*.") - 3);

        QFileInfo fileInfo(fileNameTemp);
        if (!fileNameTemp.isEmpty() && fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastImageDir", fileInfo.absolutePath());
    }
    else
    {
        fileNameTemp = fileName;
    }

    if (!fileNameTemp.isEmpty())
    {
        QFileInfo fileInfo(fileNameTemp);
        QwtPlotRenderer renderer;

        renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, false);
        renderer.setLayoutFlag(QwtPlotRenderer::KeepFrames, true);
        renderer.renderDocument(this, (fileInfo.suffix() == selectedFilter) ? fileInfo.absoluteFilePath() : fileInfo.absoluteFilePath() + "." + selectedFilter,
                                selectedFilter, QSizeF(160, 120));
    }
}

void ChartBasic::setData(double *xval, double *yval, int count)
{
    const bool doReplot = autoReplot();
    setAutoReplot(false);

    m_curve->setSamples(xval, yval, count);

    setAutoReplot(doReplot);

    replot();
}

void ChartBasic::setData(QList<double> xval, QList<double> yval)
{
    double *txval = new double[xval.count()];
    double *tyval = new double[xval.count()];

    for (int i = 0; i < xval.count(); i++)
    {
        txval[i] = xval[i];
        tyval[i] = yval[i];
    }

    setData(txval, tyval, xval.count());

    delete [] txval;
    delete [] tyval;

    replot();
}

void ChartBasic::pickerValueMoved(const QPoint &pos)
{
    QString info;
    info.sprintf("x=%g, y=%g",
                 invTransform(QwtPlot::xBottom, pos.x()),
                 invTransform(QwtPlot::yLeft, pos.y()));
}

// ***********************************************************************************************************

Chart::Chart(QWidget *parent) : ChartBasic(parent)
{
    showGrid = new QAction(tr("Show grid"), this);
    showGrid->setCheckable(true);
    connect(showGrid, SIGNAL(changed()), this, SLOT(showGridChanged()));

    mnuChart = new QMenu(this);
    mnuChart->addAction(showGrid);

    // load settings
    QSettings settings;

    showGrid->setChecked(settings.value("Chart/ShowGrid", true).toBool());
    m_grid->setVisible(showGrid->isChecked());
}

void Chart::showGridChanged()
{
    QSettings settings;
    settings.setValue("Chart/ShowGrid", showGrid->isChecked());

    m_grid->setVisible(showGrid->isChecked());
    replot();
}

Chart::~Chart()
{
}

void Chart::contextMenuEvent(QContextMenuEvent *event)
{
    mnuChart->exec(event->globalPos());
}




