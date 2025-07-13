#include "Frontend/Simulator/Engine/Components/Color.hpp"

#include "Frontend/Simulator/Engine/Engine.hpp"

Color::Color(glm::vec4 color) : Component("Color"), color(color) {}
Color::Color(uint32_t hex) : Component("Color") {
  color.r = static_cast<float>((hex >> 24) & 0xff) / 255.f;
  color.g = static_cast<float>((hex >> 16) & 0xff) / 255.f;
  color.b = static_cast<float>((hex >> 8) & 0xff) / 255.f;
  color.a = static_cast<float>(hex & 0xff) / 255.f;
}

Color::Color(float r, float g, float b, float a)
    : Component("Color"), color(r, g, b, a) {}

auto Color::bind(Engine *engine) const -> void {
  engine->get_shader()->set("v_color", color);
}
auto Color::unbind() const -> void {}

glm::vec4 Color::hextorgba(uint32_t hex){
  glm::vec4 color;

  color.r = static_cast<float>((hex >> 24) & 0xff) / 255.f;
  color.g = static_cast<float>((hex >> 16) & 0xff) / 255.f;
  color.b = static_cast<float>((hex >> 8) & 0xff) / 255.f;
  color.a = static_cast<float>(hex & 0xff) / 255.f;

  return color;
}
