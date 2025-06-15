#ifndef COLOR_HPP
#define COLOR_HPP

#include "Frontend/Simulator/Engine/Components/Component.hpp"
#include <glm/glm.hpp>

struct Color : public Component {
    Color(glm::vec4 color);
    Color(uint32_t hex);
    Color(float r, float g, float b, float a = 1.0);

  auto bind(Engine *engine) const -> void override;
  auto unbind() const -> void override;

  glm::vec4 color;

private:
  inline static const glm::vec4 UNSETTED_COLOR = glm::vec4(0.6, 0.6, 0.6, 1);
};

#endif // COLOR_HPP
