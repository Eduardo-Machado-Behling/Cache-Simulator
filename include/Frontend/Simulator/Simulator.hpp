#ifndef HELLOWORLD_HPP
#define HELLOWORLD_HPP

#include "Frontend/Frontend.hpp"
#include "Frontend/Simulator/Engine/AssetManager.hpp"
#include "Frontend/Simulator/Engine/Engine.hpp"
#include "Frontend/Simulator/Engine/AnimationManager.hpp"

#include <glad/glad.h>
#include <chrono>

#include <GLFW/glfw3.h>

class Simulator : public Frontend {
public:
  Simulator();
  auto tick(Backend *backend, std::queue<addr_t> &addrs) -> void override;
  auto halted() -> bool override;

private:
  GLFWwindow *window;
  Engine engine;
  AssetManager assets;
  AnimationManager animator;

  using floating_seconds = std::chrono::duration<float>;
  std::chrono::time_point<std::chrono::steady_clock> start;
};

#endif // HELLOWORLD_HPP
