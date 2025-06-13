#ifndef INCLUDE_CUSTOM_DEFS_HH_
#define INCLUDE_CUSTOM_DEFS_HH_

// Default value of SA_SIZE is 16 (16x16 systolic array)
#include <stdint.h>

#ifndef SA_SIZE
#define SA_SIZE 12
#endif // SA_SIZE

// #define SMM_VEC_SIZE 2

#define KERNEL_DIM SA_SIZE
#define MAX_ROWS SA_SIZE
#define MAX_COLS SA_SIZE

#define mem2d(data, data_len, row, col) data[((row) * (data_len)) + (col)]

// Min and Max isn't defined in C standard's math.h
// Taken from: https://stackoverflow.com/a/3437484/21057906
// #define max(a, b) ((a) > (b) ? (a) : (b))
// #define min(a, b) ((a) < (b) ? (a) : (b))

// Converter: double to/from uint64_t
typedef union c64_t
{
    double   f;
    uint64_t u;
    double*  p;
} c64;
typedef uint64_t u64;
typedef int64_t  i64;
typedef double   f64;

#endif // INCLUDE_CUSTOM_DEFS_HH_
