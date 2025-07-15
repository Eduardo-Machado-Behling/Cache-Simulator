#include "Frontend/Simulator/Engine/Components/Shader.hpp"

#include "Frontend/Simulator/Engine/Engine.hpp"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <optional>

static auto checkGLErrors(const std::string &operation, GLuint shaderID,
                          std::string_view uniformName) -> bool;

static auto compile_shader(GLenum type, const char *src)
    -> std::optional<uint32_t>;
static auto compile_program(std::initializer_list<uint32_t> shaders)
    -> std::optional<uint32_t>;
static auto check_compile_errors(unsigned int shader, GLenum type) -> bool;

Shader::Shader(std::string_view name, std::string_view vertex_src,
               std::string_view frag_src)
    : Component("Shader"), name(name) {
  auto vertex = compile_shader(GL_VERTEX_SHADER, vertex_src.data());
  auto frag = compile_shader(GL_FRAGMENT_SHADER, frag_src.data());

  if (!vertex.has_value() || !frag.has_value()) {
    return;
  }

  auto program = compile_program({vertex.value(), frag.value()});
  if (!program.has_value()) {
    return;
  }

  ID = program.value();
}
Shader::~Shader() { glDeleteProgram(this->ID); }

auto Shader::bind(Engine *engine) const -> void { glUseProgram(this->ID); }
auto Shader::unbind() const -> void { glUseProgram(0); }
auto Shader::getID() const -> uint32_t { return ID; }

// --- Matrix Types ---

template <>
auto Shader::set<glm::mat4>(std::string_view name, const glm::mat4 &data) const
    -> bool {
  GLint location = glGetUniformLocation(ID, name.data());
  if (location == -1) {
    std::cerr << "Error Shader::set(" << name << ") location not found"
              << std::endl;
    // It's often better to warn once rather than spamming the console
    // std::cerr << "Warning: Uniform '" << name << "' not found in Shader " <<
    // ID << std::endl;
    return false;
  }
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(data));
  return checkGLErrors("glUniformMatrix4fv", ID, name);
}

template <>
auto Shader::set<glm::mat3>(std::string_view name, const glm::mat3 &data) const
    -> bool {
  GLint location = glGetUniformLocation(ID, name.data());
  if (location == -1) {
    std::cerr << "Error Shader::set(" << name << ") location not found"
              << std::endl;
    return false;
  }
  glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(data));
  return checkGLErrors("glUniformMatrix3fv", ID, name);
}

template <>
auto Shader::set_unsafe<glm::mat4>(std::string_view name,
                                   const glm::mat4 &data) const -> bool {
  GLint location = glGetUniformLocation(ID, name.data());
  if (location == -1) {
    return false;
  }
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(data));
  return checkGLErrors("glUniformMatrix4fv", ID, name);
}

template <>
auto Shader::set_unsafe<glm::mat3>(std::string_view name,
                                   const glm::mat3 &data) const -> bool {
  GLint location = glGetUniformLocation(ID, name.data());
  if (location == -1) {
    return false;
  }
  glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(data));
  return checkGLErrors("glUniformMatrix3fv", ID, name);
}

// --- Vector Types ---

template <>
auto Shader::set_unsafe<glm::vec4>(std::string_view name,
                                   const glm::vec4 &data) const -> bool {
  GLint location = glGetUniformLocation(ID, name.data());
  if (location == -1) {
    return false;
  }
  glUniform4fv(location, 1, glm::value_ptr(data));
  return checkGLErrors("glUniform4fv", ID, name);
}

template <>
auto Shader::set_unsafe<glm::vec3>(std::string_view name,
                                   const glm::vec3 &data) const -> bool {
  GLint location = glGetUniformLocation(ID, name.data());
  if (location == -1) {
    return false;
  }
  glUniform3fv(location, 1, glm::value_ptr(data));
  return checkGLErrors("glUniform3fv", ID, name);
}

template <>
auto Shader::set_unsafe<glm::vec2>(std::string_view name,
                                   const glm::vec2 &data) const -> bool {
  GLint location = glGetUniformLocation(ID, name.data());
  if (location == -1) {
    return false;
  }
  glUniform2fv(location, 1, glm::value_ptr(data));
  return checkGLErrors("glUniform2fv", ID, name);
}

template <>
auto Shader::set<glm::vec4>(std::string_view name, const glm::vec4 &data) const
    -> bool {
  GLint location = glGetUniformLocation(ID, name.data());
  if (location == -1) {
    std::cerr << "Error Shader::set(" << name << ") location not found"
              << std::endl;
    return false;
  }
  glUniform4fv(location, 1, glm::value_ptr(data));
  return checkGLErrors("glUniform4fv", ID, name);
}

template <>
auto Shader::set<glm::vec3>(std::string_view name, const glm::vec3 &data) const
    -> bool {
  GLint location = glGetUniformLocation(ID, name.data());
  if (location == -1) {
    std::cerr << "Error Shader::set(" << name << ") location not found"
              << std::endl;
    return false;
  }
  glUniform3fv(location, 1, glm::value_ptr(data));
  return checkGLErrors("glUniform3fv", ID, name);
}

template <>
auto Shader::set<glm::vec2>(std::string_view name, const glm::vec2 &data) const
    -> bool {
  GLint location = glGetUniformLocation(ID, name.data());
  if (location == -1) {
    std::cerr << "Error Shader::set(" << name << ") location not found"
              << std::endl;
    return false;
  }
  glUniform2fv(location, 1, glm::value_ptr(data));
  return checkGLErrors("glUniform2fv", ID, name);
}

// --- Primitive Types ---

template <>
auto Shader::set_unsafe<int>(std::string_view name, const int &value) const
    -> bool {
  GLint location = glGetUniformLocation(ID, name.data());
  if (location == -1) {
    return false;
  }
  glUniform1i(location, value);
  return checkGLErrors("glUniform1i", ID, name);
}

template <>
auto Shader::set_unsafe<float>(std::string_view name, const float &value) const
    -> bool {
  GLint location = glGetUniformLocation(ID, name.data());
  if (location == -1) {
    return false;
  }
  glUniform1f(location, value);
  return checkGLErrors("glUniform1f", ID, name);
}

template <>
auto Shader::set_unsafe<bool>(std::string_view name, const bool &value) const
    -> bool {
  GLint location = glGetUniformLocation(ID, name.data());
  if (location == -1) {
    return false;
  }
  glUniform1i(location, static_cast<int>(value)); // Send bools as integers
  return checkGLErrors("glUniform1i (for bool)", ID, name);
}
template <>
auto Shader::set<int>(std::string_view name, const int &value) const -> bool {
  GLint location = glGetUniformLocation(ID, name.data());
  if (location == -1) {
    std::cerr << "Error Shader::set(" << name << ") location not found"
              << std::endl;
    return false;
  }
  glUniform1i(location, value);
  return checkGLErrors("glUniform1i", ID, name);
}

template <>
auto Shader::set<float>(std::string_view name, const float &value) const
    -> bool {
  GLint location = glGetUniformLocation(ID, name.data());
  if (location == -1) {
    std::cerr << "Error Shader::set(" << name << ") location not found"
              << std::endl;
    return false;
  }
  glUniform1f(location, value);
  return checkGLErrors("glUniform1f", ID, name);
}

template <>
auto Shader::set<bool>(std::string_view name, const bool &value) const -> bool {
  GLint location = glGetUniformLocation(ID, name.data());
  if (location == -1) {
    std::cerr << "Error Shader::set(" << name << ") location not found"
              << std::endl;
    return false;
  }
  glUniform1i(location, static_cast<int>(value)); // Send bools as integers
  return checkGLErrors("glUniform1i (for bool)", ID, name);
}

bool Shader::operator==(const Shader &other) noexcept {
  return this->ID == other.ID;
}

bool operator==(const Shader &lhs, const Shader &rhs) noexcept {
  return lhs.ID == rhs.ID;
}

static auto compile_shader(GLenum type, const char *src)
    -> std::optional<uint32_t> {
  uint32_t shader;

  switch (type) {
  case GL_FRAGMENT_SHADER:
  case GL_VERTEX_SHADER:
    shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    break;

  default:
    return false;
  }

  if (check_compile_errors(shader, type)) {
    return shader;
  } else {
    return std::nullopt;
  }
}

static auto compile_program(std::initializer_list<uint32_t> shaders)
    -> std::optional<uint32_t> {
  uint32_t program = glCreateProgram();

  for (auto shader : shaders)
    glAttachShader(program, shader);
  glLinkProgram(program);

  if (check_compile_errors(program, UINT32_MAX)) {
    for (auto shader : shaders)
      glDeleteShader(shader);
    return program;
  } else {
    return std::nullopt;
  }
}

static auto check_compile_errors(unsigned int shader, GLenum type) -> bool {
  int success;
  char infoLog[2048];

  switch (type) {
  case GL_FRAGMENT_SHADER:
  case GL_VERTEX_SHADER:
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, sizeof(infoLog) - 1, NULL, infoLog);
      std::cout
          << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
          << infoLog
          << "\n -- --------------------------------------------------- -- "
          << std::endl;
    }
    break;

  default:
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(shader, sizeof(infoLog) - 1, NULL, infoLog);
      std::cout
          << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
          << infoLog
          << "\n -- --------------------------------------------------- -- "
          << std::endl;
    }
    break;
  }

  return success;
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
