#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include "Frontend/Simulator/Engine/Components/Component.hpp"

#include <glm/glm.hpp>

struct Transform : public Component {
  Transform(glm::vec3 pos, glm::vec3 scale, glm::vec3 rotation);

  auto bind(Engine *engine) const -> void override;
  auto unbind() const -> void override;

  auto scale(glm::vec3 scale) -> void;
  auto rotate(glm::vec3 rotation) -> void;
  auto position(glm::vec3 position) -> void;
  auto move(glm::vec3 delta) -> void;

  auto get_scale() -> glm::vec3;
  auto get_rotate() -> glm::vec3;
  auto get_position() -> glm::vec3;

private:
  auto apply_vectors() -> void;

  glm::vec3 vec_position;
  glm::vec3 vec_scale;
  glm::vec3 vec_rotation;
  glm::mat4 transform = glm::mat4(1);
};

#endif // TRANSFORM_HPP