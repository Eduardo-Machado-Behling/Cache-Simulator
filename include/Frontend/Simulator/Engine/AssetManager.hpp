#ifndef ASSETMANAGER_HPP
#define ASSETMANAGER_HPP

#include "Frontend/Simulator/Engine/Components/Mesh.hpp"
#include "Frontend/Simulator/Engine/Components/Shader.hpp"
#include "Frontend/Simulator/Engine/Components/Texture.hpp"
#include "Frontend/Simulator/Engine/Font.hpp"

#include <filesystem>
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

  auto get_texture(std::string_view name) -> Texture &;
  auto get_shader(std::string_view name) -> Shader &;
  auto get_mesh(std::string_view name) -> Mesh &;
  auto get_font(std::string_view name) -> Font &;

  auto register_texture(std::string_view name, Texture &texture) -> void {
    textures.insert({name, std::move(texture)});
  }
  auto register_shader(std::string_view name, Shader &shader) -> void {
    shaders.insert({name, std::move(shader)});
  }

  auto register_mesh(std::string_view name,
                     std::vector<std::unique_ptr<MeshVertex>> &data) -> void {
    if (meshes.contains(name))
      meshes.erase(name);

    meshes.insert({name, data});
  }

  auto register_font(std::string_view name, Font &font) -> void {
    fonts.insert({name, std::move(font)});
  }

  std::unordered_map<std::string_view, Texture> textures;
  std::unordered_map<std::string_view, Shader> shaders;
  std::unordered_map<std::string_view, Mesh> meshes;
  std::unordered_map<std::string_view, Font> fonts;
};

#endif // ASSETMANAGER_HPP
