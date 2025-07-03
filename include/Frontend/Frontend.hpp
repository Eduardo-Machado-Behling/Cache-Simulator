#ifndef FRONTEND_HPP
#define FRONTEND_HPP

#include "common/CacheAccess.hpp"
#include "common/CacheReport.hpp"
#include "common/CacheSpecs.hpp"
#include "common/Types.hpp"

#include "Backend/Backend.hpp"

#include <queue>

struct Frontend {
  virtual ~Frontend() = default;

  virtual auto tick(Backend *backend, addr_t addr) -> void = 0;
  virtual auto halted() -> bool = 0;
};

#endif // FRONTEND_HPP