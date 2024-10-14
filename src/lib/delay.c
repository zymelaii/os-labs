#include <stdlib.h>

#pragma GCC push_options
#pragma GCC optimize("O0")
void busy_delay(u32 times) {
  for (int i = 0; i < times; i++) {
    for (int j = 0; j < 1e7; ++j) {
      //! cosume cpu time with busy loop
    }
  }
}
#pragma GCC pop_options
