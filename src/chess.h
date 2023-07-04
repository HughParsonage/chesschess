#ifndef chess_H
#define chess_H

#if _OPENMP
#include <omp.h>
#endif

#include <R.h>
#define USE_RINTERNALS
#include <Rinternals.h>
#include <Rversion.h>
#include <stdint.h> // for uint64_t rather than unsigned long long
#include <stdbool.h>
#include <math.h>
#include <ctype.h>



#if (__GNUC__ > 7) || \
((__GNUC__ == 7) && (__GNUC_MINOR__ > 3))
#define BUILTIN_ADD_OVERFLOW_EXIST
#endif


#if defined _OPENMP && _OPENMP >= 201511
#define FORLOOP(content)                                                \
_Pragma("omp parallel for num_threads(nThread)")                        \
  for (R_xlen_t i = 0; i < N; ++i) {                                    \
    content                                                             \
  }
#else
#define FORLOOP(content)                                       \
for (R_xlen_t i = 0; i < N; ++i) {                             \
  content                                                      \
}
#endif


bool liesOnSameDiag(unsigned int p1, unsigned int p2);

#endif
