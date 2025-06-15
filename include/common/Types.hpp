#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstdint>

using percentage_t = float;
using discrete_t = uint64_t;
using bits_t = uint8_t;
using addr_t = uint32_t;

enum class AccessResult {
  HIT,
  COMPULSORY_MISS,
  CONFLICT_MISS,
  CAPACITY_MISS,
  UNKOWN
};
enum class REPL { LRU, FIFO, RANDOM };

#endif // TYPES_HPP