#ifndef MESH_HPP
#define MESH_HPP

#include "Frontend/Simulator/Engine/Components/Component.hpp"

#include <cstdint>
#include <exception>
#include <functional>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

struct Mesh : public Component {
  struct VertsMissing : public std::exception {
    std::string msg;

  public:
    explicit VertsMissing(const std::string &message) : msg(message) {}

    const char *what() const noexcept { return msg.c_str(); }
  };

  Mesh(std::vector<std::vector<float>>& vertices, GLenum mode = GL_TRIANGLES);
  Mesh(std::initializer_list<std::vector<float>> meshes,
       GLenum mode = GL_TRIANGLES);

  auto bind(Engine *engine) const -> void override;
  auto unbind() const -> void override;

  auto draw() const -> void;

  bool operator==(const Mesh &other) noexcept;

  friend struct std::hash<Mesh>;
  friend bool operator==(const Mesh &lhs, const Mesh &rhs) noexcept;

private:
  struct MeshData {
    uint32_t VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<uint16_t> indices;
    int32_t vert_amount;
  };

  std::vector<MeshData> data;
  GLenum mode;
  uint32_t id;

  static inline uint32_t __ID = 0;
  mutable uint32_t __shader_Id;
};

bool operator==(const Mesh &lhs, const Mesh &rhs) noexcept;

namespace std {
template <> struct hash<Mesh> {
  std::size_t operator()(const Mesh &obj) const {
    std::size_t h1 = std::hash<uint32_t>()(obj.__ID);
    return h1;
  }
};
} // namespace std

#endif // MESH_HPP
