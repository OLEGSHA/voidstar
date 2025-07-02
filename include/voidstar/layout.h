// voidstar library. Copyright (c) 2025 OLEGSHA
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0 WITH Classpath-exception-2.0

#ifndef VOIDSTAR_LAYOUT_H
#define VOIDSTAR_LAYOUT_H

namespace voidstar {

/**
 * @brief Specifies member types of @a T for libffi.
 *
 * Specialize this struct to add support to types that are not already supported
 * by voidstar:
 *
 * ```c++
 * template <>
 * struct voidstar::layout<example_type> {
 *   // example_type is layed out like struct{int; float[5]; member_struct;}
 *   using members = std::tuple<int, float[5], member_struct>;
 * };
 * ```
 */
template <typename T> struct layout {};

// cv-qualified types have the same layout as unqualified types
template <typename T> struct layout<const T> : layout<T> {};
template <typename T> struct layout<volatile T> : layout<T> {};

} // namespace voidstar

#endif
