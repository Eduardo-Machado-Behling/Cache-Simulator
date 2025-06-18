#include "Frontend/Simulator/Simulator.hpp"
#include "Frontend/Simulator/Engine/Components/Color.hpp"
#include "Frontend/Simulator/Engine/Components/Transform.hpp"

Simulator::Simulator() : engine(1440, 960) {
  Object *sidebar =
      engine.object(&assets.get_shader("2d"), &assets.get_mesh("square"));
  sidebar
      ->add_component(new Transform(glm::vec3(1040, 0, 50.f),
                                    glm::vec3(400, 960, 1), glm::vec3(0)))
      .add_component(new Color(0x181818ff));
  sidebar->onResize = [](Object &self, glm::vec<2, int> &window) {
    Transform *t = self.get_component<Transform>("Transform");
    auto pos = t->get_position();
    auto scale = t->get_scale();

    pos.x = window.x - scale.x;
    scale.y = window.y;

    t->position(pos);
    t->scale(scale);
  };

  Object *bottom =
      engine.object(&assets.get_shader("2d"), &assets.get_mesh("square"));
  bottom
      ->add_component(new Transform(glm::vec3(0, 0, 99.f),
                                    glm::vec3(1040, 250, 1), glm::vec3(0)))
      .add_component(new Color(0x44475Aff));

  bottom->onResize = [](Object &self, glm::vec<2, int> &window) {
    Transform *t = self.get_component<Transform>("Transform");
    auto scale = t->get_scale();
    scale.x = window.x;
    t->scale(scale);
  };
}

auto Simulator::tick(Backend *backend, std::queue<addr_t> &addrs) -> void {
  addr_t addr = addrs.front();

  auto access = backend->process(addr);
  engine.update();
}
auto Simulator::halted() -> bool { return engine.halted(); }