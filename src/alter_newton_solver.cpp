#include "util.h"
#include "scene.h"
#include "alter_newton_solver.h"
#include "progressdialog.h"

void newton_log(char *str)
{
    Util::scene()->sceneSolution()->progressDialog()->showMessage(str, false, 0);
}


