#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp> // For printing matrices
#include <iostream>
#include <string>
#include <vector>

// Helper to convert OpenGL enum types to human-readable strings
static std::string gl_type_to_string(GLenum type) {
  switch (type) {
  case GL_FLOAT:
    return "GL_FLOAT";
  case GL_FLOAT_VEC2:
    return "GL_FLOAT_VEC2";
  case GL_FLOAT_VEC3:
    return "GL_FLOAT_VEC3";
  case GL_FLOAT_VEC4:
    return "GL_FLOAT_VEC4";
  case GL_INT:
    return "GL_INT";
  case GL_UNSIGNED_INT:
    return "GL_UNSIGNED_INT";
  case GL_BOOL:
    return "GL_BOOL";
  case GL_FLOAT_MAT4:
    return "GL_FLOAT_MAT4";
  case GL_FLOAT_MAT3:
    return "GL_FLOAT_MAT3";
  case GL_FLOAT_MAT2:
    return "GL_FLOAT_MAT2";
  default:
    return "Unknown Type (" + std::to_string(type) + ")";
  }
}

// The main debug dump function
static void dump_opengl_state_at_draw_call(GLuint programID, GLuint index_count,
                                           GLenum index_type) {
  std::cout << "\n\n==================== OpenGL State Dump ===================="
            << std::endl;

  // --- 1. Shader Program State ---
  GLint currentProgram = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
  std::cout << "[Shader Program]" << std::endl;
  std::cout << "  - Expected Program ID: " << programID << std::endl;
  std::cout << "  - Currently Bound Program ID: " << currentProgram
            << std::endl;
  if (programID != (GLuint)currentProgram) {
    std::cout << "  *** CRITICAL ERROR: Wrong shader program is bound! ***"
              << std::endl;
  }

  // --- 2. VAO, VBO, EBO Bindings ---
  GLint vao = 0, vbo = 0, ebo = 0;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &vbo);
  glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &ebo);
  std::cout << "\n[Buffer Bindings]" << std::endl;
  std::cout << "  - Bound VAO: " << vao << std::endl;
  std::cout << "  - Bound VBO (Array Buffer): " << vbo << std::endl;
  std::cout << "  - Bound EBO (Element Buffer): " << ebo << std::endl;
  if (vao == 0 || vbo == 0 || ebo == 0) {
    std::cout << "  *** WARNING: A required buffer is not bound (ID is 0). ***"
              << std::endl;
  }

  // --- 3. Vertex Attribute Pointers (for the bound VAO) ---
  std::cout << "\n[Vertex Attributes (for VAO " << vao << ")]" << std::endl;
  GLint max_attributes;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_attributes);
  // Use GLuint for loop variable to match OpenGL types
  for (GLuint i = 0; i < (GLuint)max_attributes; ++i) {
    GLint enabled = 0;
    glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
    if (enabled) {
      GLint size = 0, type = 0, stride = 0, normalized = 0;
      GLvoid *pointer = nullptr;
      glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);
      glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_TYPE, &type);
      glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);
      glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &normalized);

      // CORRECTED ENUM HERE
      glGetVertexAttribPointerv(i, GL_VERTEX_ATTRIB_ARRAY_POINTER, &pointer);

      // Use static_cast for type safety when calling our helper
      std::cout << "  - Location " << i << ": Enabled, Size: " << size
                << ", Type: " << gl_type_to_string(static_cast<GLenum>(type))
                << ", Stride: " << stride
                << ", Normalized: " << (normalized ? "Yes" : "No")
                << ", Offset: " << pointer << std::endl;
    }
  }

  // --- 4. Draw Call Parameters ---
  std::cout << "\n[Draw Call]" << std::endl;
  std::cout << "  - Function: glDrawElements" << std::endl;
  std::cout << "  - Mode: GL_TRIANGLES" << std::endl;
  std::cout << "  - Index Count: " << index_count << std::endl;
  std::cout << "  - Index Type: "
            << (index_type == GL_UNSIGNED_SHORT ? "GL_UNSIGNED_SHORT" : "Other")
            << std::endl;

  // --- 5. Uniform Values ---
  std::cout << "\n[Uniform Values (for Program " << programID << ")]"
            << std::endl;
  GLint numUniforms = 0;
  glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &numUniforms);
  for (GLint i = 0; i < numUniforms; i++) {
    char name[256];
    GLsizei length;
    GLint size;
    GLenum type;
    glGetActiveUniform(programID, (GLuint)i, sizeof(name), &length, &size,
                       &type, name);
    GLint location = glGetUniformLocation(programID, name);

    std::cout << "  - Uniform '" << name << "' (Location " << location
              << ", Type: " << gl_type_to_string(type) << ")" << std::endl;

    if (type == GL_FLOAT_MAT4 && location != -1) {
      glm::mat4 mat;
      glGetUniformfv(programID, location, glm::value_ptr(mat));
      std::cout << "    Value:\n" << glm::to_string(mat) << std::endl;
    }
    if (type == GL_FLOAT_VEC4 && location != -1) {
      glm::vec4 vec;
      glGetUniformfv(programID, location, glm::value_ptr(vec));
      std::cout << "    Value: " << glm::to_string(vec) << std::endl;
    }
  }

  // --- 6. Core GL States ---
  std::cout << "\n[GL States]" << std::endl;
  std::cout << "  - GL_DEPTH_TEST: "
            << (glIsEnabled(GL_DEPTH_TEST) ? "Enabled" : "Disabled")
            << std::endl;
  GLint depth_func = 0;
  glGetIntegerv(GL_DEPTH_FUNC, &depth_func);
  std::cout << "  - GL_DEPTH_FUNC: "
            << (depth_func == GL_LESS ? "GL_LESS" : "Other") << std::endl;
  std::cout << "  - GL_BLEND: "
            << (glIsEnabled(GL_BLEND) ? "Enabled" : "Disabled") << std::endl;

  std::cout << "================== End of State Dump ==================\n"
            << std::endl;
}