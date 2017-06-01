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

#ifndef FORM_EXAMPLE_H
#define FORM_EXAMPLE_H

#include "util/form_interface.h"
#include "sceneview_common2d.h"

#include "firesafety.h"
#include "sceneedge.h"

#include "util.h"

class PropertyDialog : public QDialog
{
    Q_OBJECT
signals:

public slots:

public:
PropertyDialog();
private:


};

class SceneViewFireSafety : public SceneViewCommon2D
{
    Q_OBJECT
signals:

public slots:
    virtual void clear();
    virtual void refresh();
    // void doSceneObjectProperties();

public:
    SceneViewFireSafety(QWidget *parent = 0);
    ~SceneViewFireSafety();

    virtual QIcon iconView() { return icon("scene-firesafety"); }
    virtual QString labelView() { return tr("Fire Safety"); }

    inline QMap<SceneEdge *, FireProperty> properties() { return m_properties; }
    inline void setProperty(SceneEdge *edge, FireProperty prop) { m_properties[edge] = prop; }
    inline void removeProperty(SceneEdge *edge) { assert(m_properties.contains(edge)); m_properties.remove(edge); }
    inline FireProperty property(SceneEdge *edge) { assert(m_properties.contains(edge)); return m_properties[edge]; }
    inline bool hasProperty(SceneEdge *edge) { if (m_properties.contains(edge)) return true; else return false; }

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    // virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);

    virtual void paintGL();
    virtual void resizeGL(int w, int h);

    void paintGeometry(); // paint nodes, edges and labels

private:
    QMap<SceneEdge *, FireProperty> m_properties;
    QMap<SceneEdge *, QList<EnvelopePoint> > m_points;
    PropertyDialog *propertyDialog;
};

class AGROS_UTIL_API ToolFireSafety : public ToolInterface
{
    Q_OBJECT
    Q_INTERFACES(ToolInterface)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org.hpfem.agros2d.ToolFireSafety" FILE "")
#endif

public:
    ToolFireSafety(QWidget *parent = 0);
    virtual ~ToolFireSafety();

    virtual QString formId() { return "firesafety"; }
    virtual QString formName() { return tr("Fire Safety"); }

    virtual int show();

public slots:
    void keyPressEvent(QKeyEvent *event);

protected:
    QWidget *mainWidget;

    SceneViewFireSafety *sceneViewFireSafety;
};



#endif // FORM_EXAMPLE_H
