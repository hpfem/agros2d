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

#ifndef GUI_H
#define GUI_H

#include <qwt_scale_map.h>
#include <qwt_symbol.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_printfilter.h>
#include <qwt_counter.h>
#include <qwt_legend.h>
#include <qwt_text.h>
#include <qwt_plot.h>
#include <qwt_scale_engine.h>

#include "util.h"

void fillComboBoxScalarVariable(QComboBox *cmbFieldVariable);
void fillComboBoxVectorVariable(QComboBox *cmbFieldVariable);
void fillComboBoxTimeStep(QComboBox *cmbFieldVariable);
void addTreeWidgetItemValue(QTreeWidgetItem *parent, const QString &name, const QString &text, const QString &unit);

class SLineEditDouble : public QLineEdit
{
    Q_OBJECT
public:
    SLineEditDouble(double val = 0, bool validator = false, QWidget *parent = 0) : QLineEdit(parent)
    {
        if (validator)
            setValidator(new QDoubleValidator);

        setValue(val);
    }

    inline double value() { return text().toDouble(); }
    inline void setValue(double value) { setText(QString::number(value)); }
};

// ****************************************************************************************************

class SLineEditValue : public QWidget
{
    Q_OBJECT
public:
    SLineEditValue(QWidget *parent = 0);

    double number();
    void setNumber(double number);

    Value value();
    void setValue(Value value);

    inline void setMinimum(double min) { m_minimum = min; }
    inline void setMinimumSharp(double min) { m_minimumSharp = min; }
    inline void setMaximum(double max) { m_maximum = max; }
    inline void setMaximumSharp(double max) { m_maximumSharp = max; }

public slots:
    bool evaluate(bool quiet = true);

signals:
    void editingFinished();
    void evaluated(bool isError);

protected:
    void focusInEvent(QFocusEvent *event);

private:
    double m_minimum;
    double m_minimumSharp;
    double m_maximum;
    double m_maximumSharp;
    double m_number;

    QLineEdit *txtLineEdit;
    QLabel *lblValue;

    void setLabel(const QString &text, QColor color, bool isVisible);
};

// ****************************************************************************************************

class Chart : public QwtPlot
{
    Q_OBJECT
public:
    Chart(QWidget *parent = 0);
    ~Chart();

    inline QwtPlotCurve *curve() { return m_curve; }
    void saveImage(const QString &fileName = "");
    QImage image() const;

public slots:
   void setData(double *xval, double *yval, int count);

private:
    QwtPlotCurve *m_curve;
};

// ****************************************************************************************************

class FileBrowser : public QListWidget
{
    Q_OBJECT

public:
    FileBrowser(QWidget *parent = 0);
    void setDir(const QString &path);
    void setNameFilter(const QString &nameFilter);
    QString basePath();
    void refresh();

public slots:
    void createDir(const QString &dirName = "");
    void createFile(const QString &fileName = "");
    void deleteDir(const QString &dirName = "");
    void deleteFile(const QString &fileName = "");
    void deleteObject(const QString &name = "");
    void renameDir(const QString &dirName);
    void renameFile(const QString &fileName);
    void renameObject(const QString &name = "");

signals:
    void fileItemActivated(const QString &path);
    void fileItemDoubleClick(const QString &path);
    void directoryChanged(const QString &path);

private slots:
    void doFileItemActivated(QListWidgetItem *item);
    void doFileItemDoubleClick(QListWidgetItem *item);
    void doContextMenu(const QPoint &point);

private:
    QString m_nameFilter;
    QString m_basePath;

    QAction *actCreateDirectory;
    QAction *actCreateFile;
    QAction *actRename;
    QAction *actDelete;

    QMenu *mnuContext;
};

class ImageLoaderDialog : public QDialog
{
    Q_OBJECT

public:
    ImageLoaderDialog(QWidget *parent = 0);
    ~ImageLoaderDialog();

    inline QString fileName() { return m_fileName; }
    inline QRectF position() { return m_position; }

public slots:

private slots:
    void doAccept();
    void doReject();
    void doLoadFile(const QString &fileName);
    void doLoadFile();
    void doRemoveFile();

private:
    QString m_fileName;
    QRectF m_position;
    QLabel *lblImage;
    QLabel *lblImageFileName;

    SLineEditDouble *txtX;
    SLineEditDouble *txtY;
    SLineEditDouble *txtWidth;
    SLineEditDouble *txtHeight;

    void createControls();
};

// ***************************************************************************************************************

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    AboutDialog(QWidget *parent = 0);
    ~AboutDialog();

private:
    void createControls();

    QWidget *createMain();
    QWidget *createAgros2D();
    QWidget *createHermes2D();
    QWidget *createLibraries();
    QWidget *createLicense();
};

#endif // GUI_H
