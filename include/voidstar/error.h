// voidstar library. Copyright (c) 2025 OLEGSHA
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0 WITH Classpath-exception-2.0

#ifndef VOIDSTAR_ERROR_H
#define VOIDSTAR_ERROR_H

#include <stdexcept>

namespace voidstar {

/**
 * @brief Base class for exceptions thrown by voidstar internals on unexpected
 * errors.
 *
 * Specific subclasses are private unless documented.
 */
struct error : std::runtime_error {
  using std::runtime_error::runtime_error;
};

} // namespace voidstar

#endif
