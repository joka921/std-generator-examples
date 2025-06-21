// <generator> -*- C++ -*-

// Copyright (C) 2023-2024 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/** @file include/generator
 *  This is a Standard C++ Library header.
 */

#pragma once

#include <ranges>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <coroutine>

#include <type_traits>
#include <concepts>

namespace custom {
using namespace std;

/** @brief A range specified using a yielding coroutine.
 *
 * `std::generator` is a utility class for defining ranges using coroutines
 * that yield elements as a range.  Generator coroutines are synchronous.
 */
template<typename Ref, typename Val = void>
class generator;

namespace gen {
/// Allocator and value type erased generator promise type.
/// \tparam Yielded The corresponding generators yielded type.
template<typename Yielded>
class Promise_erased {
  static_assert(is_reference_v<Yielded>);
  using Yielded_deref = remove_reference_t<Yielded>;
  using Yielded_decvref = remove_cvref_t<Yielded>;
  using ValuePtr = add_pointer_t<Yielded>;

  template<typename, typename, typename>
  friend class custom::generator;

private:
  ValuePtr M_value_ = nullptr;
  std::exception_ptr M_except;

public:

  struct Copy_awaiter;
public:
  suspend_always initial_suspend() const noexcept { return {}; }

  std::suspend_always final_suspend() noexcept { return {}; }

  suspend_always yield_value(Yielded val) noexcept {
    M_value() = std::addressof(val);
    return {};
  }

  auto yield_value(const Yielded_deref &val)
  noexcept(is_nothrow_constructible_v<Yielded_decvref,
          const Yielded_deref &>) requires (is_rvalue_reference_v<Yielded>
                                            && constructible_from<Yielded_decvref,
          const Yielded_deref &>) { return Copy_awaiter(val, M_value()); }


  void unhandled_exception() {
    this->M_except = std::current_exception();
  }

  void await_transform() = delete;

  void return_void() const noexcept {}

  ValuePtr &M_value() noexcept { return M_value_; }

};

template<typename Yielded>
struct Promise_erased<Yielded>::Copy_awaiter {
  Yielded_decvref M_value;
  ValuePtr &M_bottom_value;

  constexpr bool await_ready() noexcept { return false; }

  template<typename Promise>
  void await_suspend(std::coroutine_handle<Promise>) noexcept {
    M_bottom_value = ::std::addressof(M_value);
  }

  constexpr void
  await_resume() const noexcept {}
};

} // namespace gen
/// @endcond

template<typename Ref, typename Val>
class generator : public ranges::view_interface<generator<Ref, Val>> {
  using Value = conditional_t<is_void_v<Val>, remove_cvref_t<Ref>, Val>;
  using Reference = conditional_t<is_void_v<Val>, Ref &&, Ref>;

  using Yielded = conditional_t<is_reference_v<Reference>, Reference, const Reference &>;
  using Erased_promise = gen::Promise_erased<Yielded>;
  friend Erased_promise;

  struct Iterator;

public:
  struct promise_type : Erased_promise {
    generator get_return_object() noexcept { return {coroutine_handle<promise_type>::from_promise(*this)}; }
  };

  generator(const generator &) = delete;

  generator(generator &&other) noexcept
          : M_coro(std::exchange(other.M_coro, nullptr)) {}

  ~generator() {
    if (auto &c = this->M_coro)
      c.destroy();
  }

  generator &
  operator=(generator other) noexcept {
    swap(other.M_coro, this->M_coro);
  }

  Iterator begin() {
    auto h = Coro_handle::from_promise(M_coro.promise());
    return {h};
  }

  std::default_sentinel_t end() const noexcept { return default_sentinel; }

private:
  using Coro_handle = std::coroutine_handle<Erased_promise>;

  generator(coroutine_handle<promise_type> coro) noexcept
          : M_coro{move(coro)} {}

  coroutine_handle<promise_type> M_coro;
};

template<class Ref, class Val>
struct generator<Ref, Val>::Iterator {
  using value_type = Value;
  using difference_type = ptrdiff_t;

  friend bool
  operator==(const Iterator &i, default_sentinel_t) noexcept { return i.M_coro.done(); }

  friend class generator;

  Iterator(Iterator &&o) noexcept
          : M_coro(std::exchange(o.M_coro, {})) {}

  Iterator &
  operator=(Iterator &&o) noexcept {
    this->M_coro = std::exchange(o.M_coro, {});
    return *this;
  }

  Iterator &
  operator++() {
    M_coro.resume();
    return *this;
  }

  void
  operator++(int) { this->operator++(); }

  Reference
  operator*()
  const noexcept(is_nothrow_move_constructible_v<Reference>) {
    auto &p = this->M_coro.promise();
    return static_cast<Reference>(*p.M_value());
  }

private:
  friend class generator;

  Iterator(Coro_handle g)
          : M_coro{g} { M_coro.resume(); }

  Coro_handle M_coro;
};

/// @}

} // namespace std

