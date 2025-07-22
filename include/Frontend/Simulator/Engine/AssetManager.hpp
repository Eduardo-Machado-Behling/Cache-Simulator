#ifndef ASSETMANAGER_HPP
#define ASSETMANAGER_HPP

#include "Frontend/Simulator/Engine/Components/Mesh.hpp"
#include "Frontend/Simulator/Engine/Components/Shader.hpp"
#include "Frontend/Simulator/Engine/Components/Texture.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

class AssetManager {
public:
  class AssetNotFound : public std::runtime_error {
    std::string msg;

  public:
    explicit AssetNotFound(const std::string &message)
        : std::runtime_error(message), msg(message) {}

    const char *what() const noexcept override { return msg.c_str(); }
  };

  class NotImplemented : public std::runtime_error {
    std::string msg;

  public:
    explicit NotImplemented(const std::string &message)
        : std::runtime_error(message), msg(message) {}

    const char *what() const noexcept override { return msg.c_str(); }
  };

  inline static const std::filesystem::path ROOT =
      std::filesystem::current_path() / "assets";

  auto get_texture(std::string name) -> Texture &;
  auto get_shader(std::string name) -> Shader &;
  auto get_mesh(std::string name) -> Mesh &;

  auto register_texture(std::string name, Texture &texture) -> void {
    textures.insert({name, std::move(texture)});
  }
  auto register_shader(std::string name, Shader &shader) -> void {
    shaders.insert({name, std::move(shader)});
  }

  auto register_mesh(std::string name,
                     std::vector<std::unique_ptr<MeshVertex>> &data) -> void {
    if (meshes.contains(name)) {
      meshes.at(name).changeData(data);
    } else {
      meshes.emplace(name, std::move(data));
    }
  }

  std::unordered_map<std::string, Texture> textures;
  std::unordered_map<std::string, Shader> shaders;
  std::unordered_map<std::string, Mesh> meshes;
};

#endif // ASSETMANAGER_HPP
