#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "Frontend/Simulator/Engine/Components/Component.hpp"
#include "Frontend/Simulator/Engine/Components/Mesh.hpp"
#include "Frontend/Simulator/Engine/Components/Shader.hpp"

#include <functional>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <xkbcommon/xkbcommon.h>

template <size_t N> struct StringLiteral {
  constexpr StringLiteral(const char (&str)[N]) { std::copy_n(str, N, value); }

  // Allow comparison with other string literals
  constexpr bool operator==(const std::string_view other) const {
    return std::string_view(value) == other;
  }

  constexpr std::string_view get() const { return std::string_view(value); }

  char value[N];
};

struct Object {
  Object(Shader *shader, Mesh *mesh);
  Object(Object &&other);

  auto add_component(Component *Component) -> Object &;
  auto rmv_component(Component *Component) -> Object &;
  auto hide() -> void;
  auto show() -> void;
  auto visible() -> bool;

  template <StringLiteral name, typename T>
    requires IsComponent<T>
  constexpr auto set_component(T *newer) -> Object &;

  template <typename T>
    requires IsComponent<T>
  auto get_component(std::string_view name) -> T *;

  auto get_components()
      -> std::unordered_map<std::string_view, std::unique_ptr<Component>> &;

  std::function<void(Object &, glm::vec<2, int> &)> onResize;

private:
  Shader *shader;
  Mesh *mesh;
  bool active = true;

  std::unordered_map<std::string_view, std::unique_ptr<Component>> components;
};

// Template method implementation
template <typename T>
  requires IsComponent<T>
auto Object::get_component(std::string_view name) -> T * {
  if constexpr (std::is_same_v<T, Shader>) {
    if (name == "Shader") {
      return shader;
    }
    return nullptr;
  } else if constexpr (std::is_same_v<T, Mesh>) {
    if (name == "Mesh") {
      return mesh;
    }
    return nullptr;
  }

  else {
    auto it = components.find(name);
    if (it != components.end()) {
      return dynamic_cast<T *>(it->second.get());
    }
    return nullptr;
  }
}

template <StringLiteral name, typename T>
  requires IsComponent<T>
constexpr auto Object::set_component(T *newer) -> Object & {
  if constexpr (name == "Shader") {
    shader = newer;
  } else if constexpr (name == "Mesh") {
    mesh = newer;
  } else {
    auto it = components.find(name.get());
    if (it != components.end()) {
      it->second.reset(newer);
    } else {
      add_component(newer);
    }
  }

  return *this;
}

#endif // OBJECT_HPP
