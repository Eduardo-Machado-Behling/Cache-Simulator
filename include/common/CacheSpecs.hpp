#ifndef CACHE_SPECS_HPP
#define CACHE_SPECS_HPP

#include "common/Types.hpp"
#include <cmath>
#include <string>

struct CacheSpecs {
  CacheSpecs(discrete_t addr_size, discrete_t nsets, discrete_t block, discrete_t assoc, std::string substitutionPolitics )
      : nsets(nsets), block(block), assoc(assoc), substitutionPolitics( SetREPL( substitutionPolitics) ) ,
        bits{.index = static_cast<uint8_t>(log2l(nsets)),
             .offset = static_cast<uint8_t>(log2l(block)),
             .tag = static_cast<uint8_t>(addr_size - log2l(nsets) -
                                         log2l(block))} { }
  REPL SetREPL( std::string string ) {
    if( string.compare( "R" ) == 0 ) {
      return REPL::RANDOM;
    }
    if( string.compare( "L" ) == 0 ) {
      return REPL::LRU;
    }
    if( string.compare( "F" ) == 0 ) {
      return REPL::FIFO;
    }
    return REPL::RANDOM;
  }
  const discrete_t nsets;
  const discrete_t block;
  const discrete_t assoc;
  const REPL substitutionPolitics;

  const struct {
    const bits_t index;
    const bits_t offset;
    const bits_t tag;
  } bits;
};

#endif // CACHE_SPECS_HPP