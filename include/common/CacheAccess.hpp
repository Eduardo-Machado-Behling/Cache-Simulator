#ifndef CACHE_ACCESS_HPP
#define CACHE_ACCESS_HPP

#include "common/Types.hpp"

struct CacheAccess {
  addr_t orig;
  discrete_t block = 0;

  AccessResult res = AccessResult::UNKOWN;
};

#endif // CACHE_ACCESS_HPP