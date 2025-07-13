#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "Frontend/Simulator/Engine/Components/Mesh.hpp"
#include "Frontend/Simulator/Engine/Components/Shader.hpp"
#include "Frontend/Simulator/Engine/Object.hpp"

#include <functional>
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/detail/qualifier.hpp>
#include <glm/glm.hpp>
#include <list>
#include <unordered_map>

struct Engine {
  using ID = std::list<std::unique_ptr<Object>>::iterator;

  class Camera2D {
  public:
    Camera2D(float viewportWidth, float viewportHeight);

    // --- Controls ---
    // Moves the camera by a certain amount in world units
    void pan(const glm::vec2 &offset);
    // Zooms in or out. zoomFactor > 1 zooms in, < 1 zooms out.
    void zoom(float factor);

    // --- Setters ---
    void setPosition(const glm::vec2 &position);
    void setZoom(float zoomLevel);
    void setViewportSize(float width, float height);

    // --- Getters ---
    const glm::mat4 &getViewMatrix() const { return m_ViewMatrix; }
    const glm::mat4 &getProjectionMatrix() const { return m_ProjectionMatrix; }
    float getZoom() const { return m_Zoom; }
	glm::vec2 getPos() const { return m_Position; }

  private:
    void updateMatrices();

  private:
    float m_ViewportWidth, m_ViewportHeight;
    float m_Zoom = 1.0f; // Higher value is more zoomed out
    glm::vec2 m_Position = {0.0f, 0.0f};

    glm::mat4 m_ViewMatrix;
    glm::mat4 m_ProjectionMatrix;
  };

  Engine(int screen_width, int screen_height);
  ~Engine();

  auto rmv_object(std::list<std::unique_ptr<Object>>::iterator obj) -> void;
  auto rmv_object(Object *obj) -> void;

  auto add_object(std::unique_ptr<Object> obj) -> Engine::ID;
  auto add_object(Object &obj) -> Engine::ID;

  auto get_shader() -> Shader const *;

  auto set_view(int width, int height) -> void;
  auto get_view() -> glm::vec<2, int>;

  auto update() -> void;
  auto halted() -> bool;

  auto object(Shader *shader, Mesh *mesh) -> Engine::ID;
  auto get(Engine::ID id) -> Object &;
  auto getCamera() -> Camera2D&;

  std::function<void(int key, int scancode, int action, int mods)> onKey;

private:
  Camera2D camera;
  Shader const *shader;
  std::unordered_map<
      Shader, std::unordered_map<Mesh *, std::list<std::unique_ptr<Object>>,
                                 Mesh::PointerHasher, Mesh::PointerEqual>>
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
