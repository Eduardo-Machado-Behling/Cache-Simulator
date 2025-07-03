#include "app.hpp"

#include "Backend/Cache.hpp"
#include "Frontend/Simulator/Simulator.hpp"
#include "common/TQueue.hpp"
#include "Frontend/HeadLess.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

static auto getBackend(std::span<std::string> args) -> std::unique_ptr<Backend>;
static auto getFrontend(std::string &id) -> std::unique_ptr<Frontend>;
static auto flipWord(addr_t *word) -> void;

App::App(std::unique_ptr<Backend> &&backend,
         std::unique_ptr<Frontend> &&frontend, std::string &path)
    : frontend(std::move(frontend)), backend(std::move(backend)),
      running(true) {

  addr_t addr = 0;

  std::filesystem::path root = std::filesystem::current_path();
  std::filesystem::path l = root / path;
  if (!std::filesystem::exists(l)) {
    root = l.parent_path() / "assets" / "inputs";
    l = root / path;
  }

  std::ifstream in{l.string(), std::ios::binary};

  while ( in.read((char *)&addr, sizeof(addr)) ) {
    flipWord(&addr);
    addrs.push(addr);
  }
}

auto App::run() -> void {
  while (!frontend->halted() && !addrs.empty() ) {
    frontend->tick(backend.get(), addrs);
    addrs.pop();
  }
  CacheReport results = backend.get()->report();
  std::cout << results.accesses << " " << results.hit_rate << " " << results.miss_rate << " " << results.compulsory_miss_rate << " " << results.capacity_miss_rate << " " << results.conflict_miss_rate << "\n";
}

auto App::generateApp(std::vector<std::string> &command) -> App {
  constexpr size_t SEP = 5;

  return App(getBackend(std::span(std::next(command.begin()), SEP - 1)),
             getFrontend(command.at(SEP)), command.at(SEP + 1));
}

static auto getBackend(std::span<std::string> args)
    -> std::unique_ptr<Backend> {
  return std::make_unique<Cache>(args);
}

static auto getFrontend(std::string &id) -> std::unique_ptr<Frontend> {
  int i = std::stoi(id);
  switch (i) {
  case 0:
    return std::make_unique<Simulator>();
  break;
  case 1:
    return std::make_unique<HeadLess>();
  break;
  default:
    std::cout << "Chose a frontend\n\tHALTING PROGRAM\n";
    exit(0);
    break;
  }
}

static auto flipWord(addr_t *word) -> void {
  uint8_t *data = (uint8_t *)word;

  for (size_t i = 0, j = sizeof(*word) - 1; i < j; i++, j--) {
    data[i] = data[j] ^ data[i];
    data[j] = data[i] ^ data[j];
    data[i] = data[j] ^ data[i];
  }
}
