#include "Frontend/Simulator/Engine/Components/Mesh.hpp"
#include "Frontend/Simulator/Engine/Components/Component.hpp"
#include "Frontend/Simulator/Engine/Engine.hpp"

#include <cstdint>
#include <glad/glad.h>
#include <vector>

#include <unordered_map>

Mesh::Mesh(std::vector<std::unique_ptr<MeshVertex>> &vertices, GLenum mode)
    : Component("Mesh"), mode(mode) {
  if (vertices.empty()) {
    throw Mesh::VertsMissing("Mesh::Mesh: vert_amount == 0");
  }

  data.vertices.reserve(vertices.size());
  data.indices.reserve(vertices.size());

  std::unordered_map<MeshVertex *, uint16_t, MeshVertex::Hasher,
                     MeshVertex::Equal>
      map;
  uint16_t i = 0;
  for (auto it = vertices.begin(); it != vertices.end(); it++) {
    auto find = map.find(it->get());
    if (find == map.end()) {
      map.insert({it->get(), i});
      data.indices.push_back(i++);
      data.vertices.push_back(std::move(*it));
    } else {
      data.indices.push_back(find->second);
    }
  }

  data.vertices.shrink_to_fit();
  data.indices.shrink_to_fit();

  glGenVertexArrays(1, &data.VAO);
  glGenBuffers(1, &data.VBO);
  glGenBuffers(1, &data.EBO);

  glBindVertexArray(data.VAO);

  size_t size = data.vertices.size() * (size_t)data.vertices[0]->size();
  uint8_t *bdata = new uint8_t[size];

  void *working = bdata;
  for (auto const &vert : data.vertices) {
    working = vert->setData(working);
  }

  // VBO Setup (with correct size)
  glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), bdata,
               GL_STATIC_DRAW);

  delete[] bdata;

  // EBO Setup (with correct size)
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(data.indices.size() * sizeof(uint16_t)),
               data.indices.data(), GL_STATIC_DRAW);

  data.vertices[0]->operator()();

  // 4. Unbind
  glBindVertexArray(0);
}
// Mesh::Mesh(Mesh &&mesh)
//     : Component("Mesh"), data(std::move(mesh.data)), mode(mesh.mode),
//       __shader_Id(mesh.__shader_Id) {}

Mesh::~Mesh() {
  glDeleteVertexArrays(1, &data.VAO);

  GLuint buffers_to_delete[] = {data.VBO, data.EBO};
  glDeleteBuffers(2,
                  buffers_to_delete); // '2' is the number of buffers to delete
}

Mesh::Mesh(Mesh&& other) noexcept
    // 1. Move the base class part first, if it's movable
    : Component(std::move(other)),
    // 2. Copy simple data members
      mode(other.mode),
      __shader_Id(other.__shader_Id) 
{
    // 3. Move the vector resources. This transfers ownership without copying elements.
    data.vertices = std::move(other.data.vertices);
    data.indices = std::move(other.data.indices);

    // 4. Copy the simple data and GPU handles.
    data.VAO = other.data.VAO;
    data.VBO = other.data.VBO;
    data.EBO = other.data.EBO;
    data.vert_amount = other.data.vert_amount;

    // 5. IMPORTANT: Invalidate the source object's handles.
    // This prevents the destructor of 'other' from deleting the GPU resources
    // that this new object now owns.
    other.data.VAO = 0;
    other.data.VBO = 0;
    other.data.EBO = 0;
    other.data.vert_amount = 0;
}	

auto Mesh::bind(Engine *engine) const -> void {
  this->__shader_Id = engine->get_shader()->getID();
}

auto Mesh::draw() const -> void {
  glBindVertexArray(data.VAO);
  glDrawElements(mode, static_cast<GLsizei>(data.indices.size()),
                 GL_UNSIGNED_SHORT, 0);
}

auto Mesh::unbind() const -> void { glBindVertexArray(0); }

bool operator==(const Mesh &lhs, const Mesh &rhs) noexcept {
  return lhs.data.VAO == rhs.data.VAO;
}

auto Mesh::PointerHasher::operator()(const Mesh *mesh) const -> std::size_t {
  // Dereference the pointer and use the existing std::hash<Mesh> specialization
  return std::hash<Mesh>{}(*mesh);
}

// Equality comparator for Mesh*
auto Mesh::PointerEqual::operator()(const Mesh *a, const Mesh *b) const
    -> bool {
  // Dereference the pointers and use the existing operator== for Mesh
  if (a == b)
    return true;
  if (!a || !b)
    return false;
  return *a == *b;
}

// namespace std {
// template <> struct hash<glm::vec3> {
//   std::size_t operator()(const glm::vec3 &v) const {
//     std::size_t h1 = std::hash<float>{}(v.x);
//     std::size_t h2 = std::hash<float>{}(v.y);
//     std::size_t h3 = std::hash<float>{}(v.z);
//
//     std::size_t seed = h1;
//     seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
//     seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
//
//     return seed;
//   }
// };
// } // namespace std
//
// static auto genVBA(const std::vector<float> &verts,
//                    std::vector<float> &vertices, std::vector<uint16_t>
//                    &indices, uint32_t &VAO, uint32_t &VBO, uint32_t &EBO) {
//   if (vertices.size() % 3 != 0) {
//     throw Mesh::VertsMissing("Mesh::Mesh: vert_amount % 3 != 0");
//   }
//
//   vertices.reserve(verts.size());
//   indices.reserve(verts.size() / 3);
//
//   std::unordered_map<glm::vec3, uint16_t> map;
//   uint16_t i = 0;
//   for (auto it = verts.begin(); it != verts.end(); it += 3) {
//     glm::vec3 vec = glm::vec3(*it, *std::next(it), *std::next(it, 2));
//
//     auto find = map.find(vec);
//     if (find == map.end()) {
//       map.insert({vec, i});
//       indices.push_back(i++);
//       vertices.push_back(vec.x);
//       vertices.push_back(vec.y);
//       vertices.push_back(vec.z);
//     } else {
//       indices.push_back(find->second);
//     }
//   }
//
//   vertices.shrink_to_fit();
//   indices.shrink_to_fit();
//
//   glGenVertexArrays(1, &VAO);
//   glGenBuffers(1, &VBO);
//   glGenBuffers(1, &EBO);
//
//   glBindVertexArray(VAO);
//
//   // VBO Setup (with correct size)
//   glBindBuffer(GL_ARRAY_BUFFER, VBO);
//   glBufferData(GL_ARRAY_BUFFER,
//                static_cast<GLsizeiptr>(vertices.size() * sizeof(float)),
//                vertices.data(), GL_STATIC_DRAW);
//
//   // EBO Setup (with correct size)
//   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//   glBufferData(GL_ELEMENT_ARRAY_BUFFER,
//                static_cast<GLsizeiptr>(indices.size() * sizeof(uint16_t)),
//                indices.data(), GL_STATIC_DRAW);
//
//   // Attribute Pointers
//   glEnableVertexAttribArray(0);
//   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void
//   *)0);
//
//   // 4. Unbind
//   glBindVertexArray(0);
// }
//
// Mesh::Mesh(std::vector<std::vector<float>> &vertices, GLenum mode)
//     : Component("Mesh"), mode(mode), id(Mesh::__ID++) {
//   for (auto &vec : vertices) {
//     data.emplace_back();
//     auto &da = data.back();
//
//     da.vert_amount = static_cast<int32_t>(vertices.size() / 3);
//     genVBA(vec, da.vertices, da.indices, da.VAO, da.VBO, da.EBO);
//   }
// }
//
// Mesh::Mesh(std::initializer_list<std::vector<float>> vertices, GLenum mode)
//     : Component("Mesh"), mode(mode), id(Mesh::__ID++) {
//   for (auto &vec : vertices) {
//     data.emplace_back();
//     auto &da = data.back();
//
//     da.vert_amount = static_cast<int32_t>(vertices.size() / 3);
//     genVBA(vec, da.vertices, da.indices, da.VAO, da.VBO, da.EBO);
//   }
// }
//
// auto Mesh::bind(Engine *engine) const -> void {
//   this->__shader_Id = engine->get_shader()->getID();
// }
//
// auto Mesh::draw() const -> void {
//   for (auto &da : data) {
//     glBindVertexArray(da.VAO);
//     glDrawElements(mode, static_cast<GLsizei>(da.indices.size()),
//                    GL_UNSIGNED_SHORT, 0);
//   }
// }
//
// auto Mesh::unbind() const -> void { glBindVertexArray(0); }
//
// bool Mesh::operator==(const Mesh &other) noexcept {
//   return this->id == other.id;
// }
//
// bool operator==(const Mesh &lhs, const Mesh &rhs) noexcept {
//   return lhs.id == rhs.id;
// }
