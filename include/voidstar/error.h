#ifndef VOIDSTAR_ERROR_H
#define VOIDSTAR_ERROR_H

#include <stdexcept>

namespace voidstar {

struct error : std::runtime_error {
  using std::runtime_error::runtime_error;
};

} // namespace voidstar

#endif
