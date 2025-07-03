#ifndef CACHE_REPORT_HPP
#define CACHE_REPORT_HPP

#include "common/Types.hpp"

struct CacheReport {

  // #FIXME: delete this constructor
  CacheReport() = default;

  CacheReport(discrete_t accesses, discrete_t compulsory_misses,
              discrete_t capacity_miss_rate, discrete_t conflict_miss_rate);

  CacheReport(discrete_t accesses, percentage_t miss_rate,
              percentage_t hit_rate, percentage_t compulsory_miss_rate,
              percentage_t capacity_miss_rate, percentage_t conflict_miss_rate);

  void Calculate() {
    miss_rate = ( float ) miss / ( float ) accesses;
    hit_rate = ( float ) hits / ( float ) accesses;
    compulsory_miss_rate = ( float ) compulsory_miss / ( float ) accesses;
    conflict_miss_rate = ( float ) conflict_miss / ( float ) accesses;
    capacity_miss_rate = ( float ) capacity_miss / ( float ) accesses;
  }

  discrete_t accesses = 0;
  discrete_t hits = 0;
  discrete_t miss = 0;
  discrete_t compulsory_miss = 0;
  discrete_t conflict_miss = 0;
  discrete_t capacity_miss = 0;
  percentage_t miss_rate = 0.0f;
  percentage_t hit_rate = 0.0f;
  percentage_t compulsory_miss_rate = 0.0f;
  percentage_t capacity_miss_rate = 0.0f;
  percentage_t conflict_miss_rate = 0.0f;
};

#endif // CACHE_HPP