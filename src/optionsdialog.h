#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include "util.h"
#include "scene.h"
#include "sceneview.h"

class ColorButton;

struct SceneViewSettings;

class OptionsDialog : public QDialog
{
    Q_OBJECT
public:
    OptionsDialog(SceneViewSettings *sceneViewSettings, QWidget *parent);
    ~OptionsDialog();

private slots:
    void doCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void doPaletteFilter(int state);
    void doClearCommandHistory();

    void doAccept();
    void doReject();

private:
    SceneViewSettings *m_sceneViewSettings;

    QListWidget *lstView;
    QStackedWidget *pages;
    QWidget *panMain;
    QWidget *panView;
    QWidget *panColors;

    // main
    QComboBox *cmbGUIStyle;
    QComboBox *cmbLanguage;

    // geometry
    QSpinBox *txtGeometryAngleSegmentsCount;
    SLineEditDouble *txtGeometryNodeSize;
    SLineEditDouble *txtGeometryEdgeWidth;
    SLineEditDouble *txtGeometryLabelSize;

    // colors
    ColorButton *colorBackground;
    ColorButton *colorGrid;
    ColorButton *colorCross;
    ColorButton *colorNodes;
    ColorButton *colorEdges;
    ColorButton *colorLabels;
    ColorButton *colorContours;
    ColorButton *colorVectors;
    ColorButton *colorInitialMesh;
    ColorButton *colorSolutionMesh;
    ColorButton *colorHighlighted;
    ColorButton *colorSelected;

    // grid
    QLineEdit *txtGridStep;

    // rulers
    QCheckBox *chkRulers;

    // contours
    QSpinBox *txtContoursCount;

    // scalar field
    QComboBox *cmbPalette;
    QCheckBox *chkPaletteFilter;
    QSpinBox *txtPaletteSteps;

    // 3d
    QCheckBox *chkView3DLighting;

    // delete files
    QCheckBox *chkDeleteTriangleMeshFiles;
    QCheckBox *chkDeleteHermes2DMeshFile;

    // clear command history
    QPushButton *cmdClearCommandHistory;

    void load();
    void save();

    void createControls();
    QWidget *createMainWidget();
    QWidget *createViewWidget();
    QWidget *createColorsWidget();
};

// *************************************************************************

class ColorButton : public QPushButton
{
    Q_OBJECT

public:
    ColorButton(QWidget *parent = 0);
    ~ColorButton();

    inline QColor color() { return m_color; }
    void setColor(const QColor &color);

protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void doClicked();

private:
    QColor m_color;
};

#endif // OPTIONSDIALOG_H
