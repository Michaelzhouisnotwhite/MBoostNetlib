//
// Created by tssh on 24-2-21.
//

#ifndef SRC_BUFFERS
#define SRC_BUFFERS
#include <toy/type.h>

#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/address.hpp>

#include "fmt/format.h"

namespace mhttplib {
template <typename ValueType>
class VecBuffer {
public:
  using value_type = ValueType;
  using StorageType = Vec<ValueType>;
  explicit VecBuffer(u64 size);
  explicit VecBuffer(u64 size, const ValueType& stop_value);
  template <typename InputIt,
            typename = std::enable_if_t<std::is_same_v<InputIt, typename Vec<ValueType>::iterator>>>
  explicit VecBuffer(InputIt front, InputIt end)
      : storage_(front, end), storage_size_(std::distance(front, end)) {
  }
  auto begin() -> typename Vec<ValueType>::iterator {
    return storage_.begin();
  }
  auto end() -> typename Vec<ValueType>::iterator {
    return storage_.end();
  }
  auto Resize(u64 size) {
    storage_.resize(size, stop_value_);
    storage_size_ = size;
  }
  auto AsioBuffer() {
    return boost::asio::buffer(&storage_[WriteIdx()], RemainByteSize());
  }
  auto RemainByteSize() {
    return ByteSize() - WriteIdx() * sizeof(ValueType);
  }
  auto size() const -> typename Vec<ValueType>::size_type {
    return storage_.size();
  }
  auto ByteSize() const -> u64;
  auto data() -> void* {
    return storage_.data();
  }
  auto PopFront(u64 size) -> StorageType;
  auto Front(u64 size) const -> StorageType;
  auto operator[](i64 idx) -> ValueType&;
  auto PrettyStr() const -> String;

private:
  auto WriteIdx() const -> u64;
  StorageType storage_;
  ValueType stop_value_;
  u64 storage_size_ = 0;
};
template <typename ValueType>
VecBuffer<ValueType>::VecBuffer(u64 size) : storage_(size), stop_value_(), storage_size_(size) {
}
template <typename ValueType>
VecBuffer<ValueType>::VecBuffer(u64 size, const ValueType& stop_value)
    : storage_(size, stop_value), stop_value_(stop_value), storage_size_(size) {
}

template <typename ValueType>
auto VecBuffer<ValueType>::ByteSize() const -> u64 {
  return storage_.size() * sizeof(ValueType);
}
template <typename ValueType>
auto VecBuffer<ValueType>::PopFront(u64 size) -> StorageType {
  if (size >= storage_.size()) {
    auto res = StorageType(storage_.begin(), storage_.begin() + WriteIdx());
    std::fill(storage_.begin(), storage_.end(), stop_value_);
    return res;
  }
  auto res = Vec<ValueType>(storage_.begin(), storage_.begin() + size);
  storage_ = Vec<ValueType>(storage_.begin() + size, storage_.end());
  storage_.resize(storage_size_, stop_value_);
  return res;
}
template <typename ValueType>
auto VecBuffer<ValueType>::Front(u64 size) const -> StorageType {
  if (size >= storage_.size()) {
    return {storage_.begin(), storage_.begin() + WriteIdx()};
  }
  static auto last_res = Vec<ValueType>{};
  auto cur_res = Vec<ValueType>(storage_.begin(), storage_.begin() + size);
  if (last_res == cur_res) {
    return last_res;
  }
  last_res = cur_res;
  return last_res;
}
template <typename ValueType>
auto VecBuffer<ValueType>::operator[](i64 idx) -> ValueType& {
  return storage_.at(idx);
}
template <typename ValueType>
auto VecBuffer<ValueType>::PrettyStr() const -> String {
  String res;
  res.append("[");
  for (const auto& ele : storage_) {
    if constexpr (std::is_same_v<ValueType, char> || std::is_integral_v<ValueType>) {
      if (ele == '\0')
        break;
    }
    res.append(fmt::format("{}", ele));
  }
  res.append("]");
  return res;
}
template <typename ValueType>
auto VecBuffer<ValueType>::WriteIdx() const -> u64 {
  u64 increment = 0;
  for (const auto& ele : storage_) {
    if (ele == stop_value_) {
      return increment;
    }
    increment++;
  }
  if (increment == storage_.size()) {
    fmt::println("buffer overflow");
  }
  return increment;
}
}  // namespace mhttplib

#endif /* SRC_BUFFERS */
