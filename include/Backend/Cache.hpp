#include "Backend/Backend.hpp"

#include <iostream>
#include <random>
#include <span>

// Create a concrete implementation of the Backend interface
class Cache : public Backend {
public:
  // TODO: can't find a way to make interface inforce this :(,
  // so don't forget it :).
  Cache(std::span<std::string> command) : Backend(setCacheSpecs(command)){}
  ~Cache();

  auto process([[maybe_unused]] addr_t addr) -> CacheAccess & override;
  auto report() -> CacheReport & override;
private:
  struct CacheBlock
  {
    bool val;
    uint32_t tag;
  };
  auto setCacheSpecs( std::span< std::string > command) -> CacheSpecs;
};