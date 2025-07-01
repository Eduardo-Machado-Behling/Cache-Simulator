#include "Frontend/Simulator/Engine/Components/Mesh.hpp"
#include "Frontend/Simulator/Engine/Engine.hpp"

#include "Frontend/Simulator/Engine/Debug.hpp"
#include <cstdint>
#include <glad/glad.h>
#include <iterator>
#include <vector>

namespace std {
template <> struct hash<glm::vec3> {
  std::size_t operator()(const glm::vec3 &v) const {
    std::size_t h1 = std::hash<float>{}(v.x);
    std::size_t h2 = std::hash<float>{}(v.y);
    std::size_t h3 = std::hash<float>{}(v.z);

    std::size_t seed = h1;
    seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);

    return seed;
  }
};
} // namespace std

static auto genVBA(const std::vector<float> &verts,
                   std::vector<float> &vertices, std::vector<uint16_t> &indices,
                   uint32_t &VAO, uint32_t &VBO, uint32_t &EBO) {
  if (vertices.size() % 3 != 0) {
    throw Mesh::VertsMissing("Mesh::Mesh: vert_amount % 3 != 0");
  }

  vertices.reserve(verts.size());
  indices.reserve(verts.size() / 3);

  std::unordered_map<glm::vec3, uint16_t> map;
  uint16_t i = 0;
  for (auto it = verts.begin(); it != verts.end(); it += 3) {
    glm::vec3 vec = glm::vec3(*it, *std::next(it), *std::next(it, 2));

    auto find = map.find(vec);
    if (find == map.end()) {
      map.insert({vec, i});
      indices.push_back(i++);
      vertices.push_back(vec.x);
      vertices.push_back(vec.y);
      vertices.push_back(vec.z);
    } else {
      indices.push_back(find->second);
    }
  }

  vertices.shrink_to_fit();
  indices.shrink_to_fit();

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  // VBO Setup (with correct size)
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
               vertices.data(), GL_STATIC_DRAW);

  // EBO Setup (with correct size)
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(indices.size() * sizeof(uint16_t)),
               indices.data(), GL_STATIC_DRAW);

  // Attribute Pointers
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

  // 4. Unbind
  glBindVertexArray(0);
}

Mesh::Mesh(std::vector<std::vector<float>> &vertices, GLenum mode)
    : Component("Mesh"), mode(mode), id(Mesh::__ID++) {
  for (auto &vec : vertices) {
    data.emplace_back();
    auto &da = data.back();

    da.vert_amount = static_cast<int32_t>(vertices.size() / 3);
    genVBA(vec, da.vertices, da.indices, da.VAO, da.VBO, da.EBO);
  }
}

Mesh::Mesh(std::initializer_list<std::vector<float>> vertices, GLenum mode)
    : Component("Mesh"), mode(mode), id(Mesh::__ID++) {
  for (auto &vec : vertices) {
    data.emplace_back();
    auto &da = data.back();

    da.vert_amount = static_cast<int32_t>(vertices.size() / 3);
    genVBA(vec, da.vertices, da.indices, da.VAO, da.VBO, da.EBO);
  }
}

auto Mesh::bind(Engine *engine) const -> void {
  this->__shader_Id = engine->get_shader()->getID();
}

auto Mesh::draw() const -> void {
  for (auto &da : data) {
    glBindVertexArray(da.VAO);
    glDrawElements(mode, static_cast<GLsizei>(da.indices.size()),
                   GL_UNSIGNED_SHORT, 0);
  }
}

auto Mesh::unbind() const -> void { glBindVertexArray(0); }

bool Mesh::operator==(const Mesh &other) noexcept {
  return this->id == other.id;
}

bool operator==(const Mesh &lhs, const Mesh &rhs) noexcept {
  return lhs.id == rhs.id;
}
