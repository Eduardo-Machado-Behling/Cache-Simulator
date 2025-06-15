#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <concepts>
#include <string>
#include <string_view>
#include <type_traits>

struct Engine;

struct Component {
  virtual ~Component() = default;

  virtual auto bind(Engine *engine) const -> void = 0;
  virtual auto unbind() const -> void = 0;

  Component(std::string name) : name(name) {}
  auto get_name() -> std::string_view { return name; }

protected:
  std::string name;
};

template <typename T>
concept IsComponent = std::is_base_of<Component, T>::value;

#endif // COMPONENT_HPP