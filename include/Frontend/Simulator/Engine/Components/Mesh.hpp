#ifndef MESH_HPP
#define MESH_HPP

#include "Frontend/Simulator/Engine/Components/Component.hpp"

#include <cstdint>
#include <exception>
#include <functional>
#include <glm/glm.hpp>
#include <vector>

struct Mesh : public Component {
  struct VertsMissing : public std::exception {
    std::string msg;

  public:
    explicit VertsMissing(const std::string &message) : msg(message) {}

    const char *what() const noexcept { return msg.c_str(); }
  };
  Mesh(std::vector<float> &vertices);

  auto bind(Engine *engine) const -> void override;
  auto unbind() const -> void override;

  auto draw() const -> void;

  bool operator==(const Mesh &other) noexcept;

  friend struct std::hash<Mesh>;
  friend bool operator==(const Mesh &lhs, const Mesh &rhs) noexcept;

private:
  uint32_t VAO, VBO, EBO;
  mutable uint32_t __shader_Id;

  const int32_t vert_amount;
  std::vector<float> vertices;
  std::vector<uint16_t> indices;
};

bool operator==(const Mesh &lhs, const Mesh &rhs) noexcept;

namespace std {
template <> struct hash<Mesh> {
  std::size_t operator()(const Mesh &obj) const {
    std::size_t h1 = std::hash<uint32_t>()(obj.VAO);
    return h1;
  }
};
} // namespace std

#endif // MESH_HPP