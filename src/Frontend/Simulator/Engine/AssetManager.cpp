#include "Frontend/Simulator/Engine/AssetManager.hpp"
#include "Frontend/Simulator/Engine/Components/Mesh.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct FileVertex final : public MeshVertex {
  union __U {
    glm::vec3 named;
    float data[3];
  } vec;

   auto operator()() -> void override {
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, size(),
                          (void *)offsetof(__U, data));
  }

   auto size() -> int override { return sizeof(this->vec); }
   auto hash() const -> size_t override { return std::hash<glm::vec3>{}(vec.named); }
   auto equal(const MeshVertex *other) const -> bool override {
	const FileVertex* vert = reinterpret_cast<decltype(vert)>(other);
	return vert->vec.named == vec.named;
   }
   auto setData(void* data) -> void* override{
	   std::memcpy(data, &vec, sizeof(vec));
	   return static_cast<uint8_t*>(data) + sizeof(vec);
   }
};

auto AssetManager::get_texture(std::string_view name) -> Texture & {
  auto it = textures.find(name);
  if (it != textures.end()) {
    return it->second;
  }

  throw AssetManager::NotImplemented("Sorry :)");
}
auto AssetManager::get_shader(std::string_view name) -> Shader & {
  auto it = shaders.find(name);
  if (it != shaders.end()) {
    return it->second;
  };

  std::filesystem::path root = ROOT / "shaders";
  std::filesystem::path frag = root / (std::string(name) + ".frag");
  std::filesystem::path vertex = root / (std::string(name) + ".vert");

  std::ifstream vShaderFile(vertex);
  std::ifstream fShaderFile(frag);

  std::stringstream frag_src;
  frag_src << fShaderFile.rdbuf();
  std::stringstream vertex_src;
  vertex_src << vShaderFile.rdbuf();

  vShaderFile.close();
  fShaderFile.close();

  shaders.try_emplace(name, vertex_src.str(), frag_src.str());

  return shaders.at(name);
}

auto AssetManager::get_mesh(std::string_view name) -> Mesh & {
  auto it = meshes.find(name);
  if (it != meshes.end()) {
    return it->second;
  };

  std::filesystem::path root = ROOT / "meshes";
  std::filesystem::path mesh = root / (std::string(name) + ".csv");

  std::vector<std::unique_ptr<MeshVertex>> verts;
  std::ifstream mesh_file(mesh);
  char buff[255];

  size_t i = 0;
  while (!mesh_file.eof()) {
    mesh_file.read(buff, sizeof(buff) - 1);

    char *f = strtok(buff, ",\n");

    FileVertex *vertex = new FileVertex();
    while (f) {
      if (*f == '#')
        continue;
      else if (*f == '+')
        ++f;

      if (i % 3 == 0 && i) {
        verts.emplace_back(vertex);
        i = 0;
        vertex = new FileVertex();
      }

      vertex->vec.data[i++] = static_cast<float>(atof(f));

      f = strtok(NULL, ",\n");
    }

    verts.emplace_back(vertex);
  }

  mesh_file.close();

  meshes.insert({name, verts});

  return meshes.at(name);
}

auto AssetManager::get_font(std::string_view name) -> Font & {
  std::filesystem::path root = ROOT / "fonts";
  std::filesystem::path font = root / (std::string(name) + ".ttf");

  fonts.emplace(name, font);
  return fonts.at(name);
}

auto AssetManager::register_texture(std::string_view name, Texture &texture)
    -> void {
  textures.insert({name, std::move(texture)});
}
auto AssetManager::register_shader(std::string_view name, Shader &shader)
    -> void {
  shaders.insert({name, std::move(shader)});
}
auto AssetManager::register_mesh(std::string_view name, Mesh &mesh) -> void {
  meshes.insert({name, std::move(mesh)});
}
auto AssetManager::register_font(std::string_view name, Font &font) -> void {
  fonts.insert({name, std::move(font)});
}
