#ifndef __AGROS_CONFIG_H_
#define __AGROS_CONFIG_H_

/* For the case CMake does not define this. */
#define VERSION_MINOR 0
#define VERSION_SUB 0

#cmakedefine VERSION_MAJOR ${VERSION_MAJOR}
#cmakedefine VERSION_MINOR ${VERSION_MINOR}
#cmakedefine VERSION_SUB ${VERSION_SUB}
#cmakedefine VERSION_GIT ${VERSION_GIT}
#cmakedefine VERSION_YEAR ${VERSION_YEAR}
#cmakedefine VERSION_MONTH ${VERSION_MONTH}
#cmakedefine VERSION_DAY ${VERSION_DAY}

#endif