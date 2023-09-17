#pragma once

#include <utility>

namespace cog {

// Bank 15 is fixed. Any bank value >= kMinFixedBank is considered
// fixed and no bank swapping will occur
constexpr char kMinFixedBank = 15;

// Switches to the given PRG bank, returning the previous bank
//
// The region that is switched depends on the bank ID. Even banks switch the
// 0x8000 region, odd banks switch the 0xa000 region.
//
// If the bank is >= kMinFixedBank then no bank switch will occur, and the
// given bank will be returned.
//
// If the bank is already current, no bank switch will occur.
char set_prg_bank(char bank);

class ScopedBank {
public:
  constexpr ScopedBank(char new_bank) : m_prev_bank(set_prg_bank(new_bank)) {}

  constexpr ScopedBank(ScopedBank &&other) = default;
  constexpr ScopedBank(ScopedBank const &other) = delete;
  ScopedBank &operator=(ScopedBank const &other) = default;
  constexpr bool operator==(ScopedBank const &other) const = default;

  ~ScopedBank() { set_prg_bank(m_prev_bank); }

private:
  char m_prev_bank;
};

template <class ValT, class PtrT> class ScopedPrgPtr;

// Base template for PrgPtr
template <class ValT, class PtrT> class PrgPtrBase {
public:
  // If you _really_ want a nullptr, then construct it with nullptr
  constexpr PrgPtrBase() = delete;

  // Construct the PrgPtr with the given pointer and bank
  constexpr PrgPtrBase(PtrT const ptr, char const bank)
      : m_ptr(ptr), m_bank(bank) {}

  constexpr PrgPtrBase(PrgPtrBase &&other) = default;
  constexpr PrgPtrBase(PrgPtrBase const &other) = default;
  constexpr PrgPtrBase &operator=(PrgPtrBase const &other) = default;
  constexpr bool operator==(PrgPtrBase const &other) const = default;

  // Set the bank to allow use of the pointer
  //
  // Returns a scoped smart pointer that will restore the previous bank upon
  // destruction
  [[nodiscard, clang::always_inline]] constexpr ScopedPrgPtr<ValT, PtrT>
  push_bank() const {
    return ScopedPrgPtr(*this);
  }

  // Raw accessors - use at your own risk
  [[nodiscard, clang::always_inline]] constexpr PtrT ptr() const {
    return m_ptr;
  }
  void set_ptr(PtrT ptr) { m_ptr = ptr; }

  [[nodiscard, clang::always_inline]] constexpr char bank() const {
    return m_bank;
  }
  void set_bank(char bank) { m_bank = bank; }

private:
  PtrT m_ptr;
  char m_bank;
};

// This is a proxy type that is used for accessing the data behind a PrgPtr. The
// target bank is swapped in during construction and restored when this goes out
// of scope.
//
// TODO: just use unique_ptr with a custom deleter
template <class ValT, class PtrT> class ScopedPrgPtr {
public:
  constexpr ScopedPrgPtr(PrgPtrBase<ValT, PtrT> prg_ptr)
      : m_ptr(prg_ptr.ptr()), m_scoped_bank(prg_ptr.bank()) {}

  // Unique-ptr semantics
  constexpr ScopedPrgPtr(ScopedPrgPtr &&) = default;
  constexpr ScopedPrgPtr(ScopedPrgPtr const &) = default;
  constexpr ScopedPrgPtr &operator=(ScopedPrgPtr const &) = default;
  constexpr bool operator==(ScopedPrgPtr const &other) const = default;

  [[nodiscard, clang::always_inline]] constexpr operator PtrT() const {
    return m_ptr;
  }
  [[nodiscard, clang::always_inline]] constexpr ValT &operator*() const {
    return *m_ptr;
  }
  [[nodiscard, clang::always_inline]] constexpr PtrT operator->() const {
    return m_ptr;
  }

private:
  PtrT m_ptr;
  ScopedBank m_scoped_bank;
};

// PrgPtr for normal values - behaves exactly like base template
template <class T> class PrgPtr : public PrgPtrBase<T, T *> {
public:
  using ValT = T;
  using PtrT = T *;

  using PrgPtrBase<ValT, PtrT>::PrgPtrBase;
};

// PrgPtr for functions - has same type for ValT and PtrT
template <class Ret, class... Params>
class PrgPtr<Ret (*)(Params...)>
    : public PrgPtrBase<Ret (*)(Params...), Ret (*)(Params...)> {
public:
  using ValT = Ret (*)(Params...);
  using PtrT = ValT;

  using PrgPtrBase<ValT, PtrT>::PrgPtrBase;
};

// Do a banked call with arbitrary args - switches to/from the function's bank
// before/after the call
//
// Note: this must run from the fixed bank, hence the `noinline` and `section`
// attributes
template <class Ret, class... Params, class... Args>
[[clang::noinline, gnu::section(".text")]] constexpr Ret
banked_call(Ret (*fn)(Params...), char bank, Args &&...args) {
  ScopedBank scoped_bank{bank};
  return fn(std::forward<Args>(args)...);
}

template <class Ret, class... Params, class... Args>
[[clang::always_inline]] constexpr Ret
banked_call(PrgPtr<Ret (*)(Params...)> ptr, Args &&...args) {
  return banked_call(ptr.ptr(), ptr.bank(), std::forward<Args>(args)...);
}

// Create a PrgPtr from an existing pointer
template <class T> constexpr auto make_prg_ptr(T *ptr, char bank) {
  return PrgPtr<T>(ptr, bank);
}

// Create a PrgPtr from an existing function pointer
template <class Ret, class... Args>
constexpr auto make_prg_ptr(Ret (*ptr)(Args...), char bank) {
  return PrgPtr<decltype(ptr)>(ptr, bank);
}

// Create a PrgPtr from an existing fixed-bank pointer
template <class T> constexpr PrgPtr<T> make_fixed_prg_ptr(T *ptr) {
  return make_prg_ptr(ptr, kMinFixedBank);
}

// Create a PrgPtr from an existing fixed-bank pointer
template <class Ret, class... Args>
constexpr auto make_fixed_prg_ptr(Ret (*ptr)(Args...)) {
  return make_prg_ptr(ptr, kMinFixedBank);
}

} // end namespace cog

// Get the bank number for a given symbol.
//
// In order for the symbol to be recognized, it must be declared `extern` so
// that it is available to the inline assembler. Functions should be declared
// `extern "C"` to avoid name mangling. Convenience macrose are provided below.
//
// TODO: this will likely be in llvm-mos soon
#define GET_BANK(symbol)                                                       \
  []() {                                                                       \
    register char bank asm("a");                                               \
    asm("ld%0 #mos24bank(" #symbol ")\n" : "=r"(bank) : "r"(bank) :);          \
    return bank;                                                               \
  }()

// Declare data `extern` for use with GET_BANK
#define BANK_DATA extern

// Declare functions `extern "C"` for use with GET_BANK
#define BANK_FUNC extern "C"

// Create a PrgPtr with automatic bank detection
//
// As with GET_BANK, the symbol must be global, declared `extern`, and if it's a
// function, `extern "C"`
#define PRG_PTR(ptr_symbol)                                                    \
  ::cog::make_prg_ptr(ptr_symbol, GET_BANK(ptr_symbol))

// Swap in the bank necessary to use the symbol.
//
// All the caveats of GET_BANK and set_prg_bank apply
#define SET_BANK_FOR(symbol) ::cog::set_prg_bank(GET_BANK(symbol))

// Do a banked call with automatic bank detection
//
// All the caveats of GET_BANK and banked_call apply
#define BANKED_CALL(fn_symbol, ...)                                            \
  ::cog::banked_call(fn_symbol, GET_BANK(fn_symbol), __VA_ARGS__)
