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
#include <vector>
#include <utility>

#include <type_traits>
#include <concepts>


namespace batched {

constexpr static size_t BATCH_SIZE = 100;
using namespace std;

/** @brief A range specified using a yielding coroutine.
 *
 * `std::generator` is a utility class for defining ranges using coroutines
 * that yield elements as a range.  Generator coroutines are synchronous.
 *
 * @headerfile generator
 * @since C++23
 */
template<typename val>
class generator;

namespace gen {
/// Reference type for a generator whose reference (first argument) and
/// value (second argument) types are Ref and Val.
template<typename Ref, typename Val>
using Reference_t = conditional_t<is_void_v<Val>, Ref &&, Ref>;

/// Allocator and value type erased generator promise type.
/// \tparam Yielded The corresponding generators yielded type.
template<typename Yielded>
class Promise_erased {
  static_assert(is_object_v<Yielded>);

  template<typename>
  friend
  class batched::generator;

public:
  suspend_always initial_suspend() const noexcept { return {}; }

  struct SuspendIfAwaiter {
    bool suspend_;

    constexpr __attribute__((always_inline)) bool await_ready() noexcept { return !suspend_; }

    void await_suspend(std::coroutine_handle<>) noexcept {
      return;
    }

    constexpr void
    await_resume() const noexcept {}
  };

  template <typename T>
  __attribute__((always_inline)) SuspendIfAwaiter yield_value(T&& val) noexcept {
    M_buffer_.emplace_back(std::forward<T>(val));
    return {M_buffer_.size() >= BATCH_SIZE};
  }
  std::suspend_always
  final_suspend() noexcept { return {}; }

  void unhandled_exception() {
    this->M_except = std::current_exception();
  }

  void await_transform() = delete;

  void return_void() const noexcept {}

  auto& M_buffer() noexcept { return M_buffer_; }
private:

  std::vector<Yielded> M_buffer_;
  std::exception_ptr M_except;
};






} // namespace gen
/// @endcond

template<typename T>
class generator : public ranges::view_interface<generator<T>> {
  using Erased_promise = gen::Promise_erased<T>;
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
          : M_coro{std::move(coro)} {}

  coroutine_handle<promise_type> M_coro;
};

template<class T>
struct generator<T>::Iterator {
  using value_type = std::vector<T>;
  using reference  = std::vector<T>&;
  using difference_type = ptrdiff_t;
  using BufferPtr = std::add_pointer_t<std::remove_reference_t<decltype(std::declval<Coro_handle>().promise().M_buffer())>>;

  friend bool
  operator==(const Iterator &i, default_sentinel_t) noexcept {
    return
        i.M_coro.done();
  }

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
      M_coro.promise().M_buffer().clear();
      M_coro.resume();
    return *this;
  }

  void
  operator++(int) { this->operator++(); }

  reference operator*()
  const noexcept {
    return M_coro.promise().M_buffer();
  }

private:
  friend class generator;

  Iterator(Coro_handle g)
          : M_coro{g}  { M_coro.resume(); }

  Coro_handle M_coro;
};

/// @}

} // namespace std

