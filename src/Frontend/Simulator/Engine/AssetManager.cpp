#include "Frontend/Simulator/Engine/AssetManager.hpp"

#include <cstring>
#include <fstream>

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

  std::vector<float> verts;
  std::ifstream mesh_file(mesh);
  char buff[255];
  while (!mesh_file.eof()) {
    mesh_file.read(buff, sizeof(buff) - 1);

    char *f = strtok(buff, ",\n");
    while (f) {
      if (*f == '#')
        continue;
      else if (*f == '+')
        ++f;
      verts.push_back(atof(f));

      f = strtok(NULL, ",\n");
    }
  }
  mesh_file.close();

  meshes.emplace(name, verts);

  return meshes.at(name);
}

auto AssetManager::get_font(std::string_view name) -> Font &{
  std::filesystem::path root = ROOT / "fonts";
  std::filesystem::path font = root / (std::string(name) + ".ttf");

  fonts.emplace(name, font);
  return fonts.at(name);
}
