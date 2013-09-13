#ifndef __AGROS_CONFIG_H_
#define __AGROS_CONFIG_H_

#cmakedefine VERSION_MAJOR ${VERSION_MAJOR}
#cmakedefine VERSION_MINOR ${VERSION_MINOR}
#cmakedefine VERSION_SUB ${VERSION_SUB}
#cmakedefine VERSION_GIT ${VERSION_GIT}
#cmakedefine VERSION_YEAR ${VERSION_YEAR}
#cmakedefine VERSION_MONTH ${VERSION_MONTH}
#cmakedefine VERSION_DAY ${VERSION_DAY}

/* For the case CMake does not define this. */
#ifndef VERSION_MINOR
#define VERSION_MINOR 0
#endif
#ifndef VERSION_SUB
#define VERSION_SUB 0
#endif

#endif
