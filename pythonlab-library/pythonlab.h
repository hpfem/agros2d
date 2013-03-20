#ifndef PYTHONLAB_H
#define PYTHONLAB_H

#include <QMainWindow>

class PythonLab : public QMainWindow
{
    Q_OBJECT
    
public:
    PythonLab(QWidget *parent = 0);
    ~PythonLab();
};

#endif // PYTHONLAB_H
