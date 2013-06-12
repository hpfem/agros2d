#ifndef __HERMES_CONFIG_H_
#define __HERMES_CONFIG_H_

// OpenMP
#define NUM_THREADS 4
#define WITH_OPENMP

#define HAVE_FMEMOPEN
/* #undef HAVE_LOG2 */
/* #undef EXTREME_QUAD */

#define WITH_UMFPACK
/* #define WITH_PARALUTION */
/* #undef WITH_MUMPS */
/* #undef WITH_SUPERLU */
/* #undef WITH_PETSC */
/* #undef WITH_HDF5 */
/* #undef WITH_EXODUSII */
/* #undef WITH_MPI */

#define WITH_TC_MALLOC
/* #undef UMFPACK_LONG_INT */

// stacktrace
/* #undef WITH_STACKTRACE */
/* #undef EXECINFO_FOUND */

// trilinos
/* #undef WITH_TRILINOS */
/* #undef HAVE_AMESOS */
/* #undef HAVE_AZTECOO */
/* #undef HAVE_TEUCHOS */
/* #undef HAVE_EPETRA */
/* #undef HAVE_IFPACK */
/* #undef HAVE_ML */
/* #undef HAVE_NOX */
/* #undef HAVE_KOMPLEX */

// no logo
/* #undef HERMES_NO_LOGO */

// GLUT
/* #undef NOGLUT */

#endif
