#include "scene.h"
#include "logview.h"
#include "alter_newton_solver.h"

void newton_log(char *str)
{
    Util::log()->printDebug(QObject::tr("Solver"), str);
}
