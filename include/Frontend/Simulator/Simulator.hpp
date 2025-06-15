#ifndef HELLOWORLD_HPP
#define HELLOWORLD_HPP

#include "FrontEnd/Frontend.hpp"
#include "Frontend/Simulator/Engine/AssetManager.hpp"
#include "Frontend/Simulator/Engine/Engine.hpp"

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <memory>

class Simulator : public Frontend {
public:
  Simulator();
  auto tick(Backend *backend, std::queue<addr_t> &addrs) -> void override;
  auto halted() -> bool override;

private:
  GLFWwindow *window;
  Engine engine;
  AssetManager assets;
};

#endif // HELLOWORLD_HPP