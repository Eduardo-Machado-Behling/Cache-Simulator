#ifndef BACKEND_HPP
#define BACKEND_HPP

#include "common/CacheAccess.hpp"
#include "common/CacheReport.hpp"
#include "common/CacheSpecs.hpp"
#include "common/Types.hpp"

#include <span>
#include <string>

struct Backend {
  Backend(CacheSpecs specs) : __specs(specs){};
  virtual ~Backend() = default;

  auto getCache() -> CacheSpecs & { return __specs; }

  virtual auto process(addr_t addr) -> CacheAccess & = 0;
  virtual auto report() -> CacheReport & = 0;
  virtual auto halted() -> bool = 0;

protected:
  CacheAccess __access;
  CacheReport __report;
  CacheSpecs __specs;
};

#endif // BACKEND_HPP