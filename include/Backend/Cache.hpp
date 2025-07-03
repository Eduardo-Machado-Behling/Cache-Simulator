#ifndef CACHE_HPP
#define CACHE_HPP

#include "Backend/Backend.hpp"
#include "Backend/SubstitutionPolitics.hpp"

#include <iostream>
#include <random>
#include <span>
#include <vector>
#include <memory>
#include <tuple>

// Create a concrete implementation of the Backend interface
class Cache : public Backend {
public:
  // TODO: can't find a way to make interface inforce this :(,
  // so don't forget it :).
  Cache( std::span< std::string > command);
  ~Cache();

  auto process([[maybe_unused]] addr_t addr) -> CacheAccess & override;
  auto report() -> CacheReport & override;

private:
  bool isFull = false;
  struct CacheBlock
  {
    bool val = false;
    discrete_t tag = 0;
  };
  std::vector< std::vector< CacheBlock > > cache;
  std::unique_ptr< SubstitutionPolitics >  substitutionPolitics;
  auto setCacheSpecs( std::span< std::string > command) -> CacheSpecs;
  bool IsFull();
  std::tuple< bool , discrete_t > IsInTheCache( discrete_t index , discrete_t tag );
};

#endif