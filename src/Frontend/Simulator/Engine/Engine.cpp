#include "Frontend/Simulator/Engine/Engine.hpp"

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

Engine::Engine(int screen_width, int screen_height)
    : projection(glm::ortho<float>(0.0f, screen_width, 0.0f, screen_height,
                                   Z_NEAR, -Z_FAR)) {
  // --- GLFW/GLAD Initialization ---
  glfwInit();
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

Engine::~Engine() { glfwTerminate(); }

auto Engine::rmv_object(Object *obj) -> void {
  Shader *shader = obj->get_component<Shader>("Shader");
  Mesh *mesh = obj->get_component<Mesh>("Mesh");

  auto it = objects.find(*shader);
  if (it == objects.end())
    return;

  auto itt = it->second.find(*mesh);
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

auto Engine::rmv_object(std::vector<std::unique_ptr<Object>>::iterator obj)
    -> void {
  Shader *shader = (*obj)->get_component<Shader>("Shader");
  Mesh *mesh = (*obj)->get_component<Mesh>("Mesh");

  auto it = objects.find(*shader);
  if (it == objects.end())
    return;

  auto itt = it->second.find(*mesh);
  if (itt == it->second.end())
    return;

  // TODO: I'm hate doing this, but I'm stuck
  for (auto it = itt->second.begin(); it != itt->second.end(); it++) {
    if (it == obj) {
      itt->second.erase(it);
      break;
    }
  }
}

auto Engine::add_object(Object &obj) -> void {
  this->add_object(std::make_unique<Object>(std::move(obj)));
}

auto Engine::add_object(std::unique_ptr<Object> obj) -> void {
  Shader *shader = obj->get_component<Shader>("Shader");
  Mesh *mesh = obj->get_component<Mesh>("Mesh");

  objects[*shader][*mesh].push_back(std::move(obj));
  this->update();
}

auto Engine::get_shader() -> Shader const * { return shader; }

auto Engine::halted() -> bool { return glfwWindowShouldClose(window); }

auto Engine::update() -> void {
  glClearColor(40 / 255.f, 42 / 255.f, 54 / 255.f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // glClear(GL_COLOR_BUFFER_BIT);

  for (const auto &[shader, meshes] : objects) {
    this->shader = &shader;
    shader.bind(this);
    this->shader->set("m_projection", projection);
#ifdef __DEBUG
    // --- DEBUGGING OUTPUT ---
    std::cout << "--- Update ---" << std::endl;
    std::cout << "ShaderID: " << shader.getID() << std::endl;
    std::cout << "Perpective Matrix:\n"
              << glm::to_string(this->projection) << std::endl;
    std::cout << "--------------------------" << std::endl;
#endif
    for (const auto &[mesh, objs] : meshes) {
      mesh.bind(this);
      for (const auto &obj : objs) {
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

auto Engine::object(Shader *shader, Mesh *mesh) -> Object * {
  std::vector<std::unique_ptr<Object>> &vec = objects[*shader][*mesh];
  vec.push_back(std::make_unique<Object>(shader, mesh));
  return vec.back().get();
}

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
  // Example: Close the window when the 'Escape' key is pressed.
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  // You can add logic here to handle other key presses
  // e.g., update a map of currently pressed keys for player movement.
  if (action == GLFW_PRESS) {
    std::cout << "Key Pressed: " << key << std::endl;
  }
}

void Engine::handleFramebufferSize(int width, int height) {
  glViewport(0, 0, width, height);

  projection = glm::ortho<float>(0.0f, static_cast<float>(width), 0.0f,
                                 static_cast<float>(height), 0.0f, 100.0f);

  for (const auto &[shader, meshes] : objects) {
    for (const auto &[mesh, objs] : meshes) {
      for (const auto &obj : objs) {
        if (obj->onResize)
          obj->onResize(*obj, glm::vec<2, int>(width, height));
      }
    }
  }
}

void Engine::handleCursorPos(double xpos, double ypos) {
  // Example: Print mouse coordinates.
  // In a real game, you would use this for camera controls or UI interaction.
  // std::cout << "Mouse Position: (" << xpos << ", " << ypos << ")" <<
  // std::endl;
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