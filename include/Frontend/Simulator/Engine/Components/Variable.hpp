#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include "Frontend/Simulator/Engine/Components/Component.hpp"
#include "glm/common.hpp"

#include <glm/glm.hpp>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <type_traits>

template <typename T, typename... Us>
inline constexpr bool is_one_of_v = (std::is_same_v<T, Us> || ...);

template <typename T, typename Variant>
inline constexpr bool is_alternative_of_v = false; // Base case

template <typename T, typename... Us>
inline constexpr bool is_alternative_of_v<T, std::variant<Us...>> = is_one_of_v<T, Us...>;

using UniformVariant = std::variant<bool, int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat4, glm::mat3>;
template<typename T>
concept IsInUniformVariant = is_alternative_of_v<T, UniformVariant>;

struct Variable : public Component {
  auto bind(Engine *engine) const -> void override;
  auto unbind() const -> void override;

  template <IsInUniformVariant T>
  auto set(std::string_view name, T& value) -> void {
	  variables[name] = value;
  }
  template <IsInUniformVariant T>
  auto set(std::string_view name, T value) -> void {
	  variables[name] = value;
  }

  auto remove(std::string_view name);

private:
  std::unordered_map<std::string_view, UniformVariant> variables;
};

#endif // VARIABLE_HPP
