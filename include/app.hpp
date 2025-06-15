#ifndef APP_HPP
#define APP_HPP

#include "Backend/Backend.hpp"
#include "Frontend/Frontend.hpp"

#include <memory>
#include <queue>
#include <string>
#include <vector>

struct App {
  static auto generateApp(std::vector<std::string> &command) -> App;

  auto run() -> void;

private:
  App(std::unique_ptr<Backend> &&backend, std::unique_ptr<Frontend> &&frontend,
      std::string &path);

  std::unique_ptr<Frontend> frontend;
  std::unique_ptr<Backend> backend;
  std::queue<addr_t> addrs;

  bool running;
};

#endif