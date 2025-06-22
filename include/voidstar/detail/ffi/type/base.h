#ifndef VOIDSTAR_DETAIL_FFI_TYPE_BASE_H
#define VOIDSTAR_DETAIL_FFI_TYPE_BASE_H

#include <voidstar/detail/misc.h>

namespace voidstar::detail::ffi {

template <typename T> struct type_description {
  static_assert(dependent_false<T>::value, "This type is not supported");
};

} // namespace voidstar::detail::ffi

#endif
