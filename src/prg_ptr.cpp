#include "prg_ptr.hpp"

#include <mapper.h>

namespace cog {

[[clang::always_inline]] char set_prg_bank(char bank) {
  const char prev = ::get_prg_bank();
  if (bank != prev)
    ::set_prg_bank(bank);

  return prev;
}

} // end namespace cog
