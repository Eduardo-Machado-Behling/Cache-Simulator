#ifndef ASSETMANAGER_HPP
#define ASSETMANAGER_HPP

#include "Frontend/Simulator/Engine/Components/Mesh.hpp"
#include "Frontend/Simulator/Engine/Components/Shader.hpp"
#include "Frontend/Simulator/Engine/Components/Texture.hpp"

#include <filesystem>
#include <memory>
#include <string_view>
#include <unordered_map>

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

  std::unordered_map<std::string_view, Texture> textures;
  std::unordered_map<std::string_view, Shader> shaders;
  std::unordered_map<std::string_view, Mesh> meshes;
};

#endif // ASSETMANAGER_HPP