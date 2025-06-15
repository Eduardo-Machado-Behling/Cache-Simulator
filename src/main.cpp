#include "app.hpp"

#include <memory>
#include <string>
#include <vector>

int main(int argc, const char **argv) {
  std::vector<std::string> args(argv, argv + argc);
  App app = App::generateApp(args);

  app.run();
}
