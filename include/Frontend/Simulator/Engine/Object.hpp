#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "Frontend/Simulator/Engine/Components/Component.hpp"
#include "Frontend/Simulator/Engine/Components/Mesh.hpp"
#include "Frontend/Simulator/Engine/Components/Shader.hpp"

#include <functional>
#include <memory>
#include <unordered_map>

struct Object {
  Object(Shader *shader, Mesh *mesh);
  Object(Object &&other);

  auto add_component(Component *Component) -> Object &;
  auto rmv_component(Component *Component) -> Object &;

  template <typename T>
    requires IsComponent<T>
  auto set_component(Component *orig, T *newer) -> Object &;
  template <typename T>
    requires IsComponent<T>
  auto set_component(std::string_view name, T *newer) -> Object &;

  template <typename T>
    requires IsComponent<T>
  auto get_component(std::string_view name) -> T *;

  auto get_components()
      -> std::unordered_map<std::string_view, std::unique_ptr<Component>> &;

  std::function<void(Object &, glm::vec<2, int> &)> onResize;

private:
  Shader *shader;
  Mesh *mesh;

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

template <typename T>
  requires IsComponent<T>
auto Object::set_component(Component *orig, T *newer) -> Object & {
  return set_component<T>(orig->get_name(), newer);
}

template <typename T>
  requires IsComponent<T>
auto Object::set_component(std::string_view name, T *newer) -> Object & {
  if (name == "Shader") {
    shader = newer;
  } else if (name == "Mesh") {
    mesh = newer;
  } else {
    auto it = components.find(name);
    if (it != components.end()) {
      it->second.reset(newer);
    } else {
      add_component(newer);
    }
  }

  return *this;
}

#endif // OBJECT_HPP