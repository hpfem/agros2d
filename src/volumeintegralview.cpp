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

#include "volumeintegralview.h"

#include "scene.h"
#include "gui.h"

VolumeIntegralValueView::VolumeIntegralValueView(QWidget *parent): QDockWidget(tr("Volume Integral"), parent)
{
    logMessage("VolumeIntegralValue::VolumeIntegralValueView()");

    setMinimumWidth(280);
    setObjectName("VolumeIntegralValueView");

    txtView = new QTextEdit(this);
    txtView->setReadOnly(true);
    txtView->setMinimumSize(160, 160);

    setWidget(txtView);
}

void VolumeIntegralValueView::doShowVolumeIntegral()
{
    logMessage("VolumeIntegralValue::doShowVolumeIntegral()");

    // TODO: replace by template
    QString htmlHeader = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">"
            "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\" lang=\"en-US\">"
            "<head>"
            "<meta http-equiv=\"content-type\" content=\"application/xhtml+xml; charset=UTF-8\" />"
            "<title>Local point</title>"
            "</head>"
            "<body style=\"font-size: 14px;\">"
            "<table width=\"100%\">";

    QString htmlFooter = "</table>"
            "</body>";

    QString htmlBody;

    VolumeIntegralValue *volumeIntegralValue = Util::scene()->problemInfo()->module()->volume_integral_value();
    for (std::map<Hermes::Module::Integral *, double>::iterator it = volumeIntegralValue->values.begin(); it != volumeIntegralValue->values.end(); ++it)
    {
            htmlBody += "<tr>"
                    "<td><b>" + QString::fromStdString(it->first->name) + "</b></td>"
                    "<td><i>" + QString::fromStdString(it->first->shortname) + "</i></td>"
                    "<td align=\"right\">" + QString("%1").arg(it->second, 0, 'e', 3) + "</td>"
                    "<td>" + unitToHTML(QString::fromStdString(it->first->unit)) + "</td>"
                    "</tr>";
    }

    txtView->setText(htmlHeader + htmlBody + htmlFooter);
}
