#ifndef __HERMES_OMP_H_
#define __HERMES_OMP_H_

inline static int omp_get_max_threads() { return 1; }
inline static int omp_get_thread_num() { return 0; }

#endif
