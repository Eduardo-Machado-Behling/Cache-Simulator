#include "Frontend/Simulator/Engine/Object.hpp"

Object::Object(Shader *shader, Mesh *mesh) : shader(shader), mesh(mesh) {}
Object::Object(Object &&other)
    : shader(std::move(other.shader)), mesh(std::move(other.mesh)),
      components(std::move(other.components)) {}

auto Object::add_component(Component *component) -> Object & {
  if (component->get_name() == "Mesh") {
    mesh = dynamic_cast<decltype(mesh)>(component);
  } else if (component->get_name() == "Shader") {
    shader = dynamic_cast<decltype(shader)>(component);
  } else {
    this->components.emplace(component->get_name(), component);
  }

  return *this;
}

auto Object::hide() -> void{
	active = false;
}
auto Object::show() -> void {
	active = true;
}

auto Object::visible() -> bool{
	return active;
}

auto Object::rmv_component(Component *component) -> Object & {
  if (component->get_name() != "Mesh" || component->get_name() != "Shader") {
    components.erase(component->get_name());
  }

  return *this;
}

auto Object::get_components()
    -> std::unordered_map<std::string_view, std::unique_ptr<Component>> & {
  return this->components;
}
