#include "Frontend/Simulator/Engine/Components/Transform.hpp"
#include "Frontend/Simulator/Engine/Engine.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp> // Add this include
#include <iostream>

Transform::Transform(glm::vec3 pos, glm::vec3 scale, glm::vec3 rotation)
    : Component("Transform"), vec_position(pos), vec_scale(scale),
      vec_rotation(rotation) {
  apply_vectors();
}
auto Transform::bind(Engine *engine) const -> void {
  engine->get_shader()->set("m_model", transform);
}
auto Transform::unbind() const -> void {}

auto Transform::scale(glm::vec3 scale) -> void {
  if (scale == vec_scale)
    return;

  vec_scale = scale;
  apply_vectors();
}
auto Transform::rotate(glm::vec3 rotation) -> void {
  if (rotation == vec_rotation)
    return;

  vec_rotation = rotation;
  apply_vectors();
}
auto Transform::position(glm::vec3 position) -> void {
  if (position == vec_position)
    return;
  vec_position = position;
  apply_vectors();
}

auto Transform::move(glm::vec3 delta) -> void {
  if (delta == glm::vec3(0.0))
    return;

  vec_position += delta;
  apply_vectors();
}

auto Transform::get_scale() -> glm::vec3 { return vec_scale; }
auto Transform::get_rotate() -> glm::vec3 { return vec_rotation; }
auto Transform::get_position() -> glm::vec3 { return vec_position; }

auto Transform::apply_vectors() -> void {
  // 1. Create the three transformation matrices separately from the identity
  // matrix.
  glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), vec_position);

  // For rotation, it's often best to combine them. For 2D, we only need Z.
  glm::mat4 rotationMatrix = glm::rotate(
      glm::mat4(1.0f), glm::radians(vec_rotation.z), glm::vec3(0.0, 0.0, 1.0));
  // If you needed all 3 axes, you would apply them sequentially to the
  // rotation matrix: rotationMatrix = glm::rotate(rotationMatrix, ...);
  // rotationMatrix = glm::rotate(rotationMatrix, ...);

  glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), vec_scale);

  // 2. Combine them in the correct T * R * S order.
  //    This ensures vertices are scaled first, then rotated, then translated.
  this->transform = translationMatrix * rotationMatrix * scaleMatrix;
}
