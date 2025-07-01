#include "Backend/Backend.hpp"

#include <iostream>
#include <random>

// Create a concrete implementation of the Backend interface
class Cache : public Backend {
public:
  // TODO: can't find a way to make interface inforce this :(,
  // so don't forget it :).
  explicit Cache(std::span<std::string> command) : Backend(setCacheSpecs(command)) {


  }
  ~Cache() {}

  auto process([[maybe_unused]] addr_t addr) -> CacheAccess & override {

    return this->__access;
  }

  auto report() -> CacheReport & override {
    
    return this->__report;
  }


private:
  struct CacheBlock
  {
    bool val;
    int tag;
  };
  
  
  auto setCacheSpecs(std::span<std::string> command) -> CacheSpecs {
    return CacheSpecs(32, std::stoull(command[0]), std::stoull(command[1]),
                      std::stoull(command[2]));
  }
};