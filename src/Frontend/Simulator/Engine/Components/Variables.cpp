#include "Frontend/Simulator/Engine/Components/Component.hpp"
#include "Frontend/Simulator/Engine/Components/Variable.hpp"
#include "Frontend/Simulator/Engine/Engine.hpp"

Variable::Variable() : Component("Variable") {}
auto Variable::bind(Engine *engine) const -> void {
  const Shader *shader = engine->get_shader();

  for (auto &[key, val] : variables) {
    std::visit(
        [&](auto &&arg) { // 'auto&&' makes it a forwarding reference, handles
                          // const and rvalue
          using T = std::decay_t<decltype(arg)>; // Get the raw type
          shader->set<T>(key, static_cast<T>(arg));
        },
        val);
  }
}

auto Variable::unbind() const -> void {}

auto Variable::remove(std::string_view name) { variables.erase(name); }
