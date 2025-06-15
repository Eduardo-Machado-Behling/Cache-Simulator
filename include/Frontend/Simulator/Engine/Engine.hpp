#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "Frontend/Simulator/Engine/Components/Mesh.hpp"
#include "Frontend/Simulator/Engine/Components/Shader.hpp"
#include "Frontend/Simulator/Engine/Object.hpp"

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

struct Engine {
  Engine(int screen_width, int screen_height);
  ~Engine();

  auto rmv_object(std::vector<std::unique_ptr<Object>>::iterator obj) -> void;
  auto rmv_object(Object *obj) -> void;

  auto add_object(std::unique_ptr<Object> obj) -> void;
  auto add_object(Object &obj) -> void;

  auto get_shader() -> Shader const *;
  auto set_view(int width, int height) -> void;

  auto update() -> void;
  auto halted() -> bool;

  auto object(Shader *shader, Mesh *mesh) -> Object *;

private:
  Shader const *shader;
  std::unordered_map<
      Shader, std::unordered_map<Mesh, std::vector<std::unique_ptr<Object>>>>
      objects;
  glm::mat4 projection;
  GLFWwindow *window;

  // 1. The actual "handler" methods that contain our logic.
  //    These will be called by the static dispatchers.
  void handleKey(int key, int scancode, int action, int mods);
  void handleFramebufferSize(int width, int height);
  void handleCursorPos(double xpos, double ypos);

  // 2. The static "dispatcher" functions that GLFW will call directly.
  //    Their only job is to find the correct Engine instance and call the
  //    handler.
  static void KeyCallbackDispatch(GLFWwindow *window, int key, int scancode,
                                  int action, int mods);
  static void FramebufferSizeCallbackDispatch(GLFWwindow *window, int width,
                                              int height);
  static void CursorPosCallbackDispatch(GLFWwindow *window, double xpos,
                                        double ypos);
};

#endif // ENGINE_HPP