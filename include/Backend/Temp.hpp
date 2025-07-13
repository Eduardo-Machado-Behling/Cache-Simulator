#include "Backend/Backend.hpp"

#include <iostream>
#include <random>

// Create a concrete implementation of the Backend interface
class Temp : public Backend {
public:
  // TODO: can't find a way to make interface inforce this :(,
  // so don't forget it :).
  explicit Temp(std::span<std::string> command)
      : Backend(setCache(command)), rd(), engine(rd()) {}
  ~Temp() {}

  auto process([[maybe_unused]] addr_t addr) -> CacheAccess & override {
    std::uniform_int_distribution<int> dist_block(1, __specs.assoc);
    std::uniform_int_distribution<int> dist_res(
        0, static_cast<int>(AccessResult::UNKOWN));
    this->__access.block = dist_block(engine);
    this->__access.orig = addr;
    this->__access.res = static_cast<AccessResult>(dist_res(engine));

    return this->__access;
  }

  auto report() -> CacheReport & override {
    // ... your logic ...
    return this->__report;
  }

  auto halted() -> bool override {
    // ... your logic ...
    return false;
  }

private:
  auto setCache(std::span<std::string> command) -> CacheSpecs {
    // ... your logic to set the cache ...

    return CacheSpecs(32, std::stoull(command[0]), std::stoull(command[1]),
                      std::stoull(command[2]));
  }

  std::random_device rd;
  std::mt19937 engine;
};
