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

#include "util.h"
#include "scene.h"

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

    QLineEdit *txtGridStep;

    QComboBox *cmbGUIStyle;
    QComboBox *cmbLanguage;

    void load();
    void save();

    void createControls();
    QWidget *createMainWidget();
};


#endif // OPTIONSDIALOG_H
