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

constexpr static size_t BATCH_SIZE = 1'000;
using namespace std;

/** @brief A range specified using a yielding coroutine.
 *
 * `std::generator` is a utility class for defining ranges using coroutines
 * that yield elements as a range.  Generator coroutines are synchronous.
 *
 * @headerfile generator
 * @since C++23
 */
template<typename Ref, typename Val = void>
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
  static_assert(is_reference_v<Yielded>);
  using Yielded_deref = remove_reference_t<Yielded>;
  using Yielded_decvref = remove_cvref_t<Yielded>;
  using ValuePtr = add_pointer_t<Yielded>;

  template<typename, typename, typename>
  friend
  class custom::generator;

  struct Copy_awaiter;
public:
  suspend_always initial_suspend() const noexcept { return {}; }

  struct SuspendIfAwaiter {
    bool suspend_;

    constexpr bool await_ready() noexcept { return !suspend_; }

    void await_suspend(std::coroutine_handle<>) noexcept {
      return;
    }

    constexpr void
    await_resume() const noexcept {}
  };

  template <typename T>
  SuspendIfAwaiter yield_value(T&& val) noexcept {
    M_buffer_.emplace_back(std::forward<T>(val));
    return {M_buffer_.size() >= BUFSIZ};
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

  std::vector<Yielded_decvref> M_buffer_;
  std::exception_ptr M_except;
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
  using Reference = gen::Reference_t<Ref, Val>;

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
    return {h, &M_coro.promise().M_buffer()};
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
  using BufferPtr = std::add_pointer_t<std::remove_reference_t<decltype(std::declval<Coro_handle>().promise().M_buffer())>>;

  friend bool
  operator==(const Iterator &i, default_sentinel_t) noexcept { return
            i.idx_ >= i.bufferPtr_->size() &&
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
    ++idx_;
    if (idx_ >= bufferPtr_->size()) {
      idx_ = 0;
      bufferPtr_->clear();
      M_coro.resume();
    }
    return *this;
  }

  void
  operator++(int) { this->operator++(); }

  Reference
  operator*()
  const noexcept(is_nothrow_move_constructible_v<Reference>) {
    return static_cast<Reference>((*bufferPtr_)[idx_]);
  }

private:
  friend class generator;

  Iterator(Coro_handle g, BufferPtr ptr)
          : M_coro{g}, bufferPtr_{ptr} { M_coro.resume(); }

  Coro_handle M_coro;
  BufferPtr bufferPtr_;
  size_t idx_ = 0;
};

/// @}

} // namespace std

