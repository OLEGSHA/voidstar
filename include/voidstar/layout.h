// voidstar library. Copyright (c) 2025 OLEGSHA
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0 WITH Classpath-exception-2.0

#ifndef VOIDSTAR_LAYOUT_H
#define VOIDSTAR_LAYOUT_H

namespace voidstar {

template <typename> struct layout {};

template <typename T> struct layout<const T> : layout<T> {};
template <typename T> struct layout<volatile T> : layout<T> {};

} // namespace voidstar

#endif
