#ifndef INCLUDE_ARM_SYSTOLIC_ARRAY_UTILS_HH_
#define INCLUDE_ARM_SYSTOLIC_ARRAY_UTILS_HH_

#include <cstddef>

#include "defs.h"

namespace gem5 {
class SATile;
};

namespace myutils {

namespace sa {

struct PrintTile
{
public:
  PrintTile(gem5::SATile *tile, int precision, int padding);
  ~PrintTile() {
    delete[] inw_max_num_chars_cols;
    delete[] inWaitingMemory;
  };
  void print_fifo_tile();
  void print_out();

private:
  gem5::SATile *tile;
  size_t *inw_max_num_chars_cols;
  f64 *inWaitingMemory;
  int print_max_padding = 2;
};

} // namespace sa

} // namespace myutils

#endif // INCLUDE_ARM_SYSTOLIC_ARRAY_UTILS_HH_
