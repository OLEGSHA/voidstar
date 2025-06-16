#pragma once

namespace voidstar {

template <typename> struct layout {};

template <typename T> struct layout<const T> : layout<T> {};
template <typename T> struct layout<volatile T> : layout<T> {};

} // namespace voidstar
