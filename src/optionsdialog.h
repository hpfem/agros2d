#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QApplication>
#include <QStyleFactory>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QLabel>
#include <QtGui/QDialog>
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>
#include <QtGui/QDateTimeEdit>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QListWidget>
#include <QtGui/QStackedWidget>
#include <QtGui/QPushButton>
#include <QtGui/QColorDialog>

#include "util.h"
#include "scene.h"

class ColorButton;

class OptionsDialog : public QDialog
{
    Q_OBJECT
public:
    OptionsDialog(SceneViewSettings *sceneViewSettings, QWidget *parent);
    ~OptionsDialog();

private slots:
    void doCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void doAccept();
    void doReject();

private:
    SceneViewSettings *m_sceneViewSettings;

    QListWidget *lstView;
    QStackedWidget *pages;
    QWidget *panMain;
    QWidget *panColors;

    // main
    QComboBox *cmbGUIStyle;
    QComboBox *cmbLanguage;

    // colors
    ColorButton *colorContours;

    void load();
    void save();

    void createControls();
    QWidget *createMainWidget();
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
