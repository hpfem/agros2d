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

#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include "util.h"
#include "util/form_script.h"
#include "sceneview_common.h"

#include <QWebView>

class SceneViewPreprocessor;

class InfoWidget : public QWidget
{
    Q_OBJECT

public:
    InfoWidget(SceneViewPreprocessor *sceneView, QWidget *parent = 0);
    ~InfoWidget();

    inline void setRecentProblemFiles(QStringList *recentProblemFiles) { m_recentProblemFiles = recentProblemFiles; }
    inline void setRecentScriptFiles(QStringList *recentScriptFiles) { m_recentScriptFiles = recentScriptFiles; }

signals:
    void open(const QString &fileName);
    void openForm(const QString &fileName, const QString &formName);
    void examples(const QString &groupName);

public slots:
    void refresh();

private:
    SceneViewPreprocessor *m_sceneViewGeometry;
    QString m_cascadeStyleSheet;

    QWebView *webView;

    QStringList *m_recentProblemFiles;
    QStringList *m_recentScriptFiles;

private slots:
    void showWelcome();
    void showInfo();

    void linkClicked(const QUrl &url);
};

#endif // SCENEINFOVIEW_H
