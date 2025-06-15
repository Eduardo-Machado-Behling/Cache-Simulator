#include "Frontend/Simulator/Simulator.hpp"
#include "Frontend/Simulator/Engine/Components/Color.hpp"
#include "Frontend/Simulator/Engine/Components/Transform.hpp"

Simulator::Simulator() : engine(1440, 960) {
  Object *sidebar =
      engine.object(&assets.get_shader("debug"), &assets.get_mesh("square"));
  sidebar
      // FIX: Give it a Z-value *inside* the [0, 100] range.
      // Let's make it the front-most object.
      ->add_component(new Transform(glm::vec3(1040, 0, 50.f),
                                    glm::vec3(400, 960, 1), glm::vec3(0)))
      .add_component(new Color(0x181818ff));
  // ... onResize lambda ...

  Object *bottom =
      engine.object(&assets.get_shader("debug"), &assets.get_mesh("square"));
  bottom
      // FIX: Push this object further back so the sidebar appears in front.
      ->add_component(new Transform(glm::vec3(0, 0, 99.f),
                                    glm::vec3(1040, 250, 1), glm::vec3(0)))
      .add_component(new Color(0x44475Aff));
  // ... onResize lambda ...
}

auto Simulator::tick(Backend *backend, std::queue<addr_t> &addrs) -> void {
  addr_t addr = addrs.front();

  auto access = backend->process(addr);
  engine.update();
}
auto Simulator::halted() -> bool { return engine.halted(); }