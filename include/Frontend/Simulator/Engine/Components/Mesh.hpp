#ifndef MESH_HPP
#define MESH_HPP

#include "Frontend/Simulator/Engine/Components/Component.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

struct MeshVertex {
  virtual ~MeshVertex() = default;

  virtual auto operator()() -> void = 0;
  virtual auto size() -> int = 0;
  virtual auto hash() const -> size_t = 0;
  virtual auto equal(const MeshVertex *other) const -> bool = 0;

  // set data in $data and return the next addr
  virtual auto setData(void *data) -> void * = 0;

  struct Hasher {
    auto operator()(const MeshVertex *v) const -> size_t {
      // Delegate hashing to the virtual function of the object
      return v->hash();
    }
  };

  // A functor that knows how to compare two MeshVertices by calling virtual
  // equal()
  struct Equal {
    auto operator()(const MeshVertex *a, const MeshVertex *b) const -> bool {
      if (a == b)
        return true; // Pointers are same, objects are same
      if (!a || !b)
        return false; // One is null

      // Delegate comparison to the virtual function of the object
      return a->equal(b);
    }
  };
};

struct Mesh : public Component {
  struct VertsMissing : public std::exception {
    std::string msg;

  public:
    explicit VertsMissing(const std::string &message) : msg(message) {}

    const char *what() const noexcept { return msg.c_str(); }
  };

  Mesh(std::vector<std::unique_ptr<MeshVertex>> vertices,
       GLenum mode = GL_TRIANGLES);
  Mesh(Mesh &&mesh) noexcept;
  ~Mesh();

  auto changeData(std::vector<std::unique_ptr<MeshVertex>> &vertices,
                  GLenum mode = GL_TRIANGLES) -> void;

  auto bind(Engine *engine) const -> void override;
  auto unbind() const -> void override;

  auto draw() const -> void;

  friend struct std::hash<Mesh>;
  friend bool operator==(const Mesh &lhs, const Mesh &rhs) noexcept;

  // Hasher for Mesh*
  struct PointerHasher {
    auto operator()(const Mesh *mesh) const -> std::size_t;
  };

  // Equality comparator for Mesh*
  struct PointerEqual {
    auto operator()(const Mesh *a, const Mesh *b) const -> bool;
  };

private:
  auto init(std::vector<std::unique_ptr<MeshVertex>> &vertices) -> void;

  struct MeshData {
    uint32_t VAO, VBO, EBO;
    std::vector<std::unique_ptr<MeshVertex>> vertices;
    std::vector<uint16_t> indices;
    int32_t vert_amount;
  };

  MeshData data;
  GLenum mode;
  mutable uint32_t __shader_Id;
};

bool operator==(const Mesh &lhs, const Mesh &rhs) noexcept;

namespace std {
template <> struct hash<Mesh> {
  std::size_t operator()(const Mesh &obj) const { return obj.data.VAO; }
};
} // namespace std

#endif // MESH_HPP
