#include "Frontend/Simulator/Engine/Components/Texture.hpp"
#include "Frontend/Simulator/Engine/Components/Component.hpp"
#include "Frontend/Simulator/Engine/Engine.hpp"
#include <filesystem>
#include <iostream>

// Include stb_image implementation once in your project (e.g., in a .cpp file)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture::Texture(const std::string &imagePath, GLenum textureType, GLenum slot,
                 GLenum format, GLenum pixelType)
    : Component("Texture"), path(std::filesystem::current_path() / "assets" /
                                 "textures" / imagePath) {
  // Generate a new OpenGL texture ID
  glGenTextures(1, &ID);
  // Activate the texture unit before binding (e.g., GL_TEXTURE0)
  glActiveTexture(slot);
  // Bind the texture to its type (e.g., GL_TEXTURE_2D)
  glBindTexture(textureType, ID);

  // Set texture wrapping parameters
  // GL_REPEAT: Repeats the texture image
  // GL_CLAMP_TO_BORDER: Clamps to a user-specified border color
  // GL_CLAMP_TO_EDGE: Clamps to the edges of the texture
  glTexParameteri(textureType, GL_TEXTURE_WRAP_S,
                  GL_REPEAT); // S-axis (U coordinate)
  glTexParameteri(textureType, GL_TEXTURE_WRAP_T,
                  GL_REPEAT); // T-axis (V coordinate)

  // Set texture filtering parameters for minification and magnification
  // GL_LINEAR: Smooth filtering
  // GL_NEAREST: Pixelated filtering
  glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR); // Minification filter
  glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER,
                  GL_LINEAR); // Magnification filter

  // Load image data using stb_image
  int width, height, numChannels;
  // Set 1 to flip textures vertically on load (OpenGL expects 0,0 at
  // bottom-left)
  stbi_set_flip_vertically_on_load(1);
  unsigned char *data =
      stbi_load(path.c_str(), &width, &height, &numChannels, 0);

  if (data) {
    // Specify the texture image data
    // For common image formats, `format` might be GL_RGB or GL_RGBA
    // `pixelType` is typically GL_UNSIGNED_BYTE
    glTexImage2D(textureType, 0, format, width, height, 0, format, pixelType,
                 data);
    // Generate mipmaps for better quality at different distances
    glGenerateMipmap(textureType);
  } else {
    std::cerr << "Failed to load texture: " << imagePath << std::endl;
  }

  // Free image data after it's uploaded to the GPU
  stbi_image_free(data);

  // Unbind the texture to prevent accidental modification
  glBindTexture(textureType, 0);
}

void Texture::bind(Engine *engine) const {
  // You should activate the correct texture unit before binding in your
  // rendering loop For simplicity, this example binds to GL_TEXTURE0 if no slot
  // is specified. In actual use, you'd likely pass the slot to this method or
  // manage it externally. glActiveTexture(GL_TEXTURE0 + <your_slot_number>);
  glBindTexture(GL_TEXTURE_2D, ID); // Assuming GL_TEXTURE_2D for most cases
}

void Texture::unbind() const {
  glBindTexture(GL_TEXTURE_2D, 0); // Unbind from GL_TEXTURE_2D target
}

Texture::~Texture() {
  // Delete the OpenGL texture object
  if (ID != 0) { // Check if it's a valid ID (not moved-from)
    glDeleteTextures(1, &ID);
  }
}

Texture::Texture(Texture &&other) noexcept
    : Component("Texture"), ID(other.ID), path(std::move(other.path)) {
  other.ID = 0; // Invalidate the moved-from object
}

Texture &Texture::operator=(Texture &&other) noexcept {
  if (this != &other) {
    glDeleteTextures(1, &ID); // Delete current texture if exists
    ID = other.ID;
    path = std::move(other.path);
    other.ID = 0;
  }
  return *this;
}
