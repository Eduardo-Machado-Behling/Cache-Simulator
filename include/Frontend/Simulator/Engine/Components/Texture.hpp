#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "Frontend/Simulator/Engine/Components/Component.hpp"

#pragma once

#include <filesystem>
#include <glad/glad.h> // Assuming you're using GLAD for OpenGL function pointers
#include <stb_image.h> // Recommended for easy image loading
#include <string>

struct Texture : public Component {
  // OpenGL texture ID
  GLuint ID;
  // Path to the image file (for debugging/reference, optional)
  std::filesystem::path path;

  // Constructor: Loads pixel data from file and creates OpenGL texture
  Texture(const std::string &imagePath, GLenum textureType, GLenum slot,
          GLenum format, GLenum pixelType);

  auto bind(Engine *engine) const -> void override;
  auto unbind() const -> void override;

  // Destructor: Cleans up OpenGL texture
  ~Texture();

  // Prevent copying (textures often represent unique OpenGL resources)
  Texture(const Texture &) = delete;
  Texture &operator=(const Texture &) = delete;

  // Allow moving (useful for managing textures in containers)
  Texture(Texture &&other) noexcept;

  Texture &operator=(Texture &&other) noexcept;
};

#endif // TEXTURE_HPP
