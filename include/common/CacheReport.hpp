#ifndef CACHE_HPP
#define CACHE_HPP

#include "common/Types.hpp"

struct CacheReport {

  // #FIXME: delete this constructor
  CacheReport() = default;

  CacheReport(discrete_t accesses, discrete_t compulsory_misses,
              discrete_t capacity_miss_rate, discrete_t conflict_miss_rate);

  CacheReport(discrete_t accesses, percentage_t miss_rate,
              percentage_t hit_rate, percentage_t compulsory_miss_rate,
              percentage_t capacity_miss_rate, percentage_t conflict_miss_rate);

  const discrete_t accesses = 0;
  const percentage_t miss_rate = 0;
  const percentage_t hit_rate = 0;
  const percentage_t compulsory_miss_rate = 0;
  const percentage_t capacity_miss_rate = 0;
  const percentage_t conflict_miss_rate = 0;
};

#endif // CACHE_HPP