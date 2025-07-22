#include "Frontend/Simulator/Engine/Engine.hpp"
#include "Frontend/Simulator/Engine/Components/Shader.hpp"

#include <cstddef>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp> // Add this include
#include <iostream>

#define Z_NEAR 0.f
#define Z_FAR 100.f

static auto checkGLErrors(const std::string &operation, GLuint shaderID,
                          std::string_view uniformName) -> bool;

GLFWManager::GLFWManager() { glfwInit(); }
GLFWManager::~GLFWManager() { glfwTerminate(); }

Engine::Engine(int screen_width, int screen_height)
    : camera(screen_width, screen_height),
      projection(glm::ortho<float>(0.0f, screen_width, 0.0f, screen_height,
                                   Z_NEAR, -Z_FAR)) {
  // --- GLFW/GLAD Initialization ---
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  window = glfwCreateWindow(screen_width, screen_height, "2D Renderer", nullptr,
                            nullptr);
  glfwMakeContextCurrent(window);

  // === The Core of the Pattern ===
  // Store a pointer to this Engine instance within the GLFW window.
  glfwSetWindowUserPointer(window, this);
  // =============================

  // --- Register our static dispatcher functions ---

  // Register the framebuffer size callback (for window resizing)
  glfwSetFramebufferSizeCallback(window, FramebufferSizeCallbackDispatch);

  // Register the key callback (for keyboard input)
  glfwSetKeyCallback(window, KeyCallbackDispatch);

  // Register the cursor position callback (for mouse movement)
  glfwSetCursorPosCallback(window, CursorPosCallbackDispatch);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    exit(EXIT_FAILURE);
  }

  glViewport(0, 0, screen_width, screen_height);
  glEnable(GL_DEPTH_TEST);
  // glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

auto Engine::rmv_object(Object *obj) -> void {
  Shader *shader = obj->get_component<Shader>("Shader");
  Mesh *mesh = obj->get_component<Mesh>("Mesh");

  auto it = objects.find(*shader);
  if (it == objects.end())
    return;

  auto itt = it->second.find(mesh);
  if (itt == it->second.end())
    return;

  // TODO: I'm hate doing this, but I'm stuck
  for (auto it = itt->second.begin(); it != itt->second.end(); it++) {
    if (it->get() == obj) {
      itt->second.erase(it);
      break;
    }
  }
}

auto Engine::rmv_object(std::list<std::unique_ptr<Object>>::iterator obj)
    -> void {
  Shader *shader = (*obj)->get_component<Shader>("Shader");
  Mesh *mesh = (*obj)->get_component<Mesh>("Mesh");

  auto it = objects.find(*shader);
  if (it == objects.end())
    return;

  auto itt = it->second.find(mesh);
  if (itt == it->second.end())
    return;

  itt->second.erase(obj);
}

auto Engine::add_object(Object &obj) -> Engine::ID {
  return this->add_object(std::make_unique<Object>(std::move(obj)));
}

auto Engine::add_object(std::unique_ptr<Object> obj) -> Engine::ID {
  Shader *shader = obj->get_component<Shader>("Shader");
  Mesh *mesh = obj->get_component<Mesh>("Mesh");

  size_t i = objects[*shader][mesh].size();
  objects[*shader][mesh].push_back(std::move(obj));
  return std::next(objects[*shader][mesh].begin(), (long)i);
}

auto Engine::get_shader() -> Shader const * { return shader; }
auto Engine::set_view(int width, int height) -> void {
  glfwSetWindowSize(window, width, height);
}
auto Engine::get_view() -> glm::vec<2, int> {
  glm::vec<2, int> winSize;
  glfwGetWindowSize(window, &winSize.x, &winSize.y);
  return winSize;
}

auto Engine::halted() -> bool { return glfwWindowShouldClose(window); }

auto Engine::update() -> void {
  glClearColor(40 / 255.f, 42 / 255.f, 54 / 255.f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // glClear(GL_COLOR_BUFFER_BIT);

  for (const auto &[shader, meshes] : objects) {
    this->shader = &shader;
    shader.bind(this);
    this->shader->set("m_projection", projection);
    this->shader->set_unsafe("m_view", camera.getViewMatrix());
    for (const auto &[pmesh, objs] : meshes) {
      const Mesh &mesh = *pmesh;
      mesh.bind(this);
      for (const auto &obj : objs) {
        if (!obj->visible()) {
          continue;
        }

        for (const auto &[name, comp] : obj->get_components()) {
          comp->bind(this);
        }

        mesh.draw();
        checkGLErrors("Draw", this->shader->getID(), "N/A");

        for (const auto &[name, comp] : obj->get_components()) {
          comp->unbind();
        }
      }
      mesh.unbind();
    }
    shader.unbind();
  }

  glfwSwapBuffers(window);
  glfwPollEvents();
}

auto Engine::object(Shader *shader, Mesh *mesh) -> Engine::ID {
  std::list<std::unique_ptr<Object>> &vec = objects[*shader][mesh];
  size_t i = vec.size();
  vec.push_back(std::make_unique<Object>(shader, mesh));
  return std::next(vec.begin(), (long)i);
}

auto Engine::get(Engine::ID id) -> Object & { return **id; }

auto Engine::getCamera() -> Camera2D & { return camera; }
// --- Static Dispatcher Implementations ---

void Engine::KeyCallbackDispatch(GLFWwindow *window, int key, int scancode,
                                 int action, int mods) {
  // 1. Get the Engine instance that we stored in the window.
  Engine *self = static_cast<Engine *>(glfwGetWindowUserPointer(window));
  if (self) {
    // 2. Call the actual instance-specific handler method.
    self->handleKey(key, scancode, action, mods);
  }
}

void Engine::FramebufferSizeCallbackDispatch(GLFWwindow *window, int width,
                                             int height) {
  Engine *self = static_cast<Engine *>(glfwGetWindowUserPointer(window));
  if (self) {
    self->handleFramebufferSize(width, height);
  }
}

void Engine::CursorPosCallbackDispatch(GLFWwindow *window, double xpos,
                                       double ypos) {
  Engine *self = static_cast<Engine *>(glfwGetWindowUserPointer(window));
  if (self) {
    self->handleCursorPos(xpos, ypos);
  }
}

// --- Member Function Handler Implementations (Your Actual Logic) ---

void Engine::handleKey(int key, int scancode, int action, int mods) {
  // Close the window when the 'Escape' key is pressed.
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  if (onKey)
    onKey(key, scancode, action, mods);
}

void Engine::handleFramebufferSize(int width, int height) {
  glViewport(0, 0, width, height);

  projection = glm::ortho<float>(0.0f, static_cast<float>(width), 0.0f,
                                 static_cast<float>(height), Z_NEAR, -Z_FAR);
  glm::vec<2, int> win(width, height);
  for (const auto &[shader, meshes] : objects) {
    for (const auto &[mesh, objs] : meshes) {
      for (const auto &obj : objs) {
        if (obj->onResize)
          obj->onResize(*obj, win);
      }
    }
  }
}

void Engine::handleCursorPos(double xpos, double ypos) {

}

Engine::Camera2D::Camera2D(float viewportWidth, float viewportHeight)
    : m_ViewportWidth(viewportWidth), m_ViewportHeight(viewportHeight) {
  updateMatrices();
}

void Engine::Camera2D::pan(const glm::vec2 &offset) {
  m_Position += offset;
  updateMatrices();
}

void Engine::Camera2D::zoom(float factor) {
  // Clamp zoom to avoid getting too large or small
  m_Zoom = glm::clamp(m_Zoom * factor, 0.1f, 10.0f);
  updateMatrices();
}

void Engine::Camera2D::setPosition(const glm::vec2 &position) {
  m_Position = position;
  updateMatrices();
}

void Engine::Camera2D::setZoom(float zoomLevel) {
  m_Zoom = glm::clamp(zoomLevel, 0.1f, 10.0f);
  updateMatrices();
}

void Engine::Camera2D::setViewportSize(float width, float height) {
  m_ViewportWidth = width;
  m_ViewportHeight = height;
  updateMatrices();
}

void Engine::Camera2D::updateMatrices() {
  // --- Projection Matrix ---
  // This defines the "view box" of our camera.
  // Zooming is achieved by scaling the size of this box.
  float aspectRatio = m_ViewportWidth / m_ViewportHeight;
  float orthoWidth = m_Zoom * aspectRatio;
  float orthoHeight = m_Zoom;

  m_ProjectionMatrix = glm::ortho(-orthoWidth,  // left
                                  orthoWidth,   // right
                                  -orthoHeight, // bottom
                                  orthoHeight,  // top
                                  Z_NEAR,       // near plane
                                  -Z_FAR        // far plane
  );

  // --- View Matrix ---
  // This matrix moves the "world" to simulate the camera moving.
  // It's the inverse of the camera's position transformation.
  m_ViewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-m_Position, 0.0f));
}

static auto checkGLErrors(const std::string &operation, GLuint shaderID,
                          std::string_view uniformName) -> bool {
  GLenum error;
  bool hadError = false;
  // Loop to catch all error flags
  while ((error = glGetError()) != GL_NO_ERROR) {
    hadError = true;
    std::cerr << "OpenGL Error after operation '" << operation
              << "' for uniform '" << uniformName << "' in Shader Program "
              << shaderID << ": ";
    switch (error) {
    case GL_INVALID_ENUM:
      std::cerr << "GL_INVALID_ENUM";
      break;
    case GL_INVALID_VALUE:
      std::cerr << "GL_INVALID_VALUE";
      break;
    case GL_INVALID_OPERATION:
      std::cerr << "GL_INVALID_OPERATION";
      break;
    case GL_OUT_OF_MEMORY:
      std::cerr << "GL_OUT_OF_MEMORY";
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      std::cerr << "GL_INVALID_FRAMEBUFFER_OPERATION";
      break;
    default:
      std::cerr << "Unknown error code " << error;
      break;
    }
    std::cerr << std::endl;
  }
  return !hadError;
}
