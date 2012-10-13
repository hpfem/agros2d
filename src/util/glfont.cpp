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

/* ============================================================================
 * Freetype GL - A C OpenGL Freetype engine
 * Platform:    Any
 * WWW:         http://code.google.com/p/freetype-gl/
 * ----------------------------------------------------------------------------
 * Copyright 2011,2012 Nicolas P. Rougier. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NICOLAS P. ROUGIER ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL NICOLAS P. ROUGIER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Nicolas P. Rougier.
 * ===============================================================================
 */

#include "glfont.h"

#include <QComboBox>

#include "fonts/ubuntu-10.h"
#include "fonts/ubuntu-12.h"
#include "fonts/ubuntu-14.h"
#include "fonts/ubuntu-16.h"
#include "fonts/ubuntu-18.h"
#include "fonts/ubuntu-20.h"
#include "fonts/ubuntu-22.h"
#include "fonts/ubuntu-24.h"

#include "fonts/liberation-10.h"
#include "fonts/liberation-12.h"
#include "fonts/liberation-14.h"
#include "fonts/liberation-16.h"
#include "fonts/liberation-18.h"
#include "fonts/liberation-20.h"
#include "fonts/liberation-22.h"
#include "fonts/liberation-24.h"

#include "fonts/google-droid-10.h"
#include "fonts/google-droid-12.h"
#include "fonts/google-droid-14.h"
#include "fonts/google-droid-16.h"
#include "fonts/google-droid-18.h"
#include "fonts/google-droid-20.h"
#include "fonts/google-droid-22.h"
#include "fonts/google-droid-24.h"

#include "fonts/computer-modern-10.h"
#include "fonts/computer-modern-12.h"
#include "fonts/computer-modern-14.h"
#include "fonts/computer-modern-16.h"
#include "fonts/computer-modern-18.h"
#include "fonts/computer-modern-20.h"
#include "fonts/computer-modern-22.h"
#include "fonts/computer-modern-24.h"

void fillFontsComboBox(QComboBox *cmbFonts)
{
    // store variable
    QString fnt = cmbFonts->itemData(cmbFonts->currentIndex()).toString();

    // clear combo
    cmbFonts->blockSignals(true);
    cmbFonts->clear();

    cmbFonts->addItem(fontUbuntuMono10Label, "fontUbuntuMono10");
    cmbFonts->addItem(fontUbuntuMono12Label, "fontUbuntuMono12");
    cmbFonts->addItem(fontUbuntuMono14Label, "fontUbuntuMono14");
    cmbFonts->addItem(fontUbuntuMono16Label, "fontUbuntuMono16");
    cmbFonts->addItem(fontUbuntuMono18Label, "fontUbuntuMono18");
    cmbFonts->addItem(fontUbuntuMono20Label, "fontUbuntuMono20");
    cmbFonts->addItem(fontUbuntuMono22Label, "fontUbuntuMono22");
    cmbFonts->addItem(fontUbuntuMono24Label, "fontUbuntuMono24");

    cmbFonts->addItem(fontLiberationMono10Label, "fontLiberationMono10");
    cmbFonts->addItem(fontLiberationMono12Label, "fontLiberationMono12");
    cmbFonts->addItem(fontLiberationMono14Label, "fontLiberationMono14");
    cmbFonts->addItem(fontLiberationMono16Label, "fontLiberationMono16");
    cmbFonts->addItem(fontLiberationMono18Label, "fontLiberationMono18");
    cmbFonts->addItem(fontLiberationMono20Label, "fontLiberationMono20");
    cmbFonts->addItem(fontLiberationMono22Label, "fontLiberationMono22");
    cmbFonts->addItem(fontLiberationMono24Label, "fontLiberationMono24");

    cmbFonts->addItem(fontGoogleDroidMono10Label, "fontGoogleDroidMono10");
    cmbFonts->addItem(fontGoogleDroidMono12Label, "fontGoogleDroidMono12");
    cmbFonts->addItem(fontGoogleDroidMono14Label, "fontGoogleDroidMono14");
    cmbFonts->addItem(fontGoogleDroidMono16Label, "fontGoogleDroidMono16");
    cmbFonts->addItem(fontGoogleDroidMono18Label, "fontGoogleDroidMono18");
    cmbFonts->addItem(fontGoogleDroidMono20Label, "fontGoogleDroidMono20");
    cmbFonts->addItem(fontGoogleDroidMono22Label, "fontGoogleDroidMono22");
    cmbFonts->addItem(fontGoogleDroidMono24Label, "fontGoogleDroidMono24");

    cmbFonts->addItem(fontComputerModernMono10Label, "fontComputerModernMono10");
    cmbFonts->addItem(fontComputerModernMono12Label, "fontComputerModernMono12");
    cmbFonts->addItem(fontComputerModernMono14Label, "fontComputerModernMono14");
    cmbFonts->addItem(fontComputerModernMono16Label, "fontComputerModernMono16");
    cmbFonts->addItem(fontComputerModernMono18Label, "fontComputerModernMono18");
    cmbFonts->addItem(fontComputerModernMono20Label, "fontComputerModernMono20");
    cmbFonts->addItem(fontComputerModernMono22Label, "fontComputerModernMono22");
    cmbFonts->addItem(fontComputerModernMono24Label, "fontComputerModernMono24");

    cmbFonts->setCurrentIndex(cmbFonts->findData(fnt));
    if (cmbFonts->currentIndex() == -1)
        cmbFonts->setCurrentIndex(0);

    cmbFonts->blockSignals(false);
}

TextureFont *textureFontFromStringKey(const QString &fnt)
{
    if (fnt == "fontUbuntuMono10")
        return fontUbuntuMono10();
    else if (fnt == "fontUbuntuMono12")
        return fontUbuntuMono12();
    else if (fnt == "fontUbuntuMono14")
        return fontUbuntuMono14();
    else if (fnt == "fontUbuntuMono16")
        return fontUbuntuMono16();
    else if (fnt == "fontUbuntuMono18")
        return fontUbuntuMono18();
    else if (fnt == "fontUbuntuMono20")
        return fontUbuntuMono20();
    else if (fnt == "fontUbuntuMono22")
        return fontUbuntuMono22();
    else if (fnt == "fontUbuntuMono24")
        return fontUbuntuMono24();
    else if (fnt == "fontLiberationMono10")
        return fontLiberationMono10();
    else if (fnt == "fontLiberationMono12")
        return fontLiberationMono12();
    else if (fnt == "fontLiberationMono14")
        return fontLiberationMono14();
    else if (fnt == "fontLiberationMono16")
        return fontLiberationMono16();
    else if (fnt == "fontLiberationMono18")
        return fontLiberationMono18();
    else if (fnt == "fontLiberationMono20")
        return fontLiberationMono20();
    else if (fnt == "fontLiberationMono22")
        return fontLiberationMono22();
    else if (fnt == "fontLiberationMono24")
        return fontLiberationMono24();
    else if (fnt == "fontGoogleDroidMono10")
        return fontGoogleDroidMono10();
    else if (fnt == "fontGoogleDroidMono12")
        return fontGoogleDroidMono12();
    else if (fnt == "fontGoogleDroidMono14")
        return fontGoogleDroidMono14();
    else if (fnt == "fontGoogleDroidMono16")
        return fontGoogleDroidMono16();
    else if (fnt == "fontGoogleDroidMono18")
        return fontGoogleDroidMono18();
    else if (fnt == "fontGoogleDroidMono20")
        return fontGoogleDroidMono20();
    else if (fnt == "fontGoogleDroidMono22")
        return fontGoogleDroidMono22();
    else if (fnt == "fontGoogleDroidMono24")
        return fontGoogleDroidMono24();
    else if (fnt == "fontComputerModernMono10")
        return fontComputerModernMono10();
    else if (fnt == "fontComputerModernMono12")
        return fontComputerModernMono12();
    else if (fnt == "fontComputerModernMono14")
        return fontComputerModernMono14();
    else if (fnt == "fontComputerModernMono16")
        return fontComputerModernMono16();
    else if (fnt == "fontComputerModernMono18")
        return fontComputerModernMono18();
    else if (fnt == "fontComputerModernMono20")
        return fontComputerModernMono20();
    else if (fnt == "fontComputerModernMono22")
        return fontComputerModernMono22();
    else if (fnt == "fontComputerModernMono24")
        return fontComputerModernMono24();

    // default
    return fontUbuntuMono10();
}

