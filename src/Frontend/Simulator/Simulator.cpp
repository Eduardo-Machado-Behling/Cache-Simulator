#include "Frontend/Simulator/Simulator.hpp"
#include "Frontend/Simulator/Engine/Components/Color.hpp"
#include "Frontend/Simulator/Engine/Components/Transform.hpp"
#include "glm/fwd.hpp"

#include <chrono>

Simulator::Simulator() : engine(1440, 960) {
  Engine::ID sd_id = 
      engine.object(&assets.get_shader("2d"), &assets.get_mesh("square"));
  Object& sidebar =
	  engine.get(sd_id);
  sidebar
      .add_component(new Transform(glm::vec3(1040, 0, 50.f),
                                    glm::vec3(400, 960, 1), glm::vec3(0)))
      .add_component(new Color(0x181818ff));
  sidebar.onResize = [](Object &self, glm::vec<2, int> &window) {
    Transform *t = self.get_component<Transform>("Transform");
    auto pos = t->get_position();
    auto scale = t->get_scale();

    pos.x = window.x - scale.x;
    scale.y = window.y;

    t->position(pos);
    t->scale(scale);
  };


  animator.add_track(
  {
  AnimationManager::Animation([&, sd_id](float progress){
	Object& sidebar = engine.get(sd_id);
    Transform *t = sidebar.get_component<Transform>("Transform");

	constexpr glm::vec3 dest = glm::vec3(1040, 0, 50.f);
	constexpr glm::vec3 original = glm::vec3(0, 0, 50.f);
	constexpr glm::vec3 delta = dest - original;

	glm::vec3 final = delta * progress + original;

	t->position(final);
  }, 0.5f),

  AnimationManager::Animation([&, sd_id](float progress){
	Object& sidebar = engine.get(sd_id);
    Transform *t = sidebar.get_component<Transform>("Transform");

	constexpr glm::vec3 original = glm::vec3(1040, 0, 50.f);
	constexpr glm::vec3 dest = glm::vec3(0, 0, 50.f);

	t->position(AnimationManager::lerp(original, dest, progress));
  }, 0.5f)
  }

  );
  assets.get_font("JetBrainsMono-Bold");

  Engine::ID bt_id = 
      engine.object(&assets.get_shader("2d"), &assets.get_mesh("square"));
  Object& bottom =
	  engine.get(bt_id);
  bottom
      .add_component(new Transform(glm::vec3(0, 0, 99.f),
                                    glm::vec3(1040, 250, 1), glm::vec3(0)))
      .add_component(new Color(0x44475Aff));

  bottom.onResize = [](Object &self, glm::vec<2, int> &window) {
    Transform *t = self.get_component<Transform>("Transform");
    auto scale = t->get_scale();
    scale.x = window.x;
    t->scale(scale);
  };
}

auto Simulator::tick(Backend *backend, std::queue<addr_t> &addrs) -> void {
  static auto start = std::chrono::steady_clock::now();
  auto now = std::chrono::steady_clock::now();
  auto elapsed = now - start;

  float elapsed_s = std::chrono::duration_cast<floating_seconds>(elapsed).count();
  addr_t addr = addrs.front();

  auto access = backend->process(addr);
  animator(elapsed_s);
  engine.update();

  start = now;
}
auto Simulator::halted() -> bool { return engine.halted(); }
