#ifndef CACHE_SPECS_HPP
#define CACHE_SPECS_HPP

#include "common/Types.hpp"
#include <cmath>
#include <string>

struct CacheSpecs {
  CacheSpecs(discrete_t addr_size, discrete_t nsets, discrete_t block,
             discrete_t assoc, std::string substitutionPolitics)
      : nsets(nsets), block(block), assoc(assoc),
        substitutionPolitics(SetREPL(substitutionPolitics)),
        bits(addr_size, nsets, block) {}
  REPL SetREPL(std::string string) {
    if (string.compare("R") == 0) {
      return REPL::RANDOM;
    }
    if (string.compare("L") == 0) {
      return REPL::LRU;
    }
    if (string.compare("F") == 0) {
      return REPL::FIFO;
    }
    return REPL::RANDOM;
  }
  const discrete_t nsets;
  const discrete_t block;
  const discrete_t assoc;
  const REPL substitutionPolitics;

  const struct Bits {
    const bits_t index;
    const bits_t offset;
    const bits_t tag;

    Bits(discrete_t addrSize, discrete_t nsets, discrete_t block)
        : index(static_cast<bits_t>(ceilf(log2f((float)nsets)))),
          offset(static_cast<uint8_t>(ceilf(log2f((float)block)))),
          tag(static_cast<uint8_t>(addrSize - index - offset)) {}
  } bits;
};

#endif // CACHE_SPECS_HPP
