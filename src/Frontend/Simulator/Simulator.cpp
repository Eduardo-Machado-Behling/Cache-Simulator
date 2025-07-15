#include "Frontend/Simulator/Simulator.hpp"
#include "Backend/Backend.hpp"
#include "Frontend/Simulator/Engine/AnimationManager.hpp"
#include "Frontend/Simulator/Engine/AssetManager.hpp"
#include "Frontend/Simulator/Engine/Components/Color.hpp"
#include "Frontend/Simulator/Engine/Components/Mesh.hpp"
#include "Frontend/Simulator/Engine/Components/Texture.hpp"
#include "Frontend/Simulator/Engine/Components/Transform.hpp"
#include "Frontend/Simulator/Engine/Components/Variable.hpp"
#include "Frontend/Simulator/Engine/Engine.hpp"
#include "common/CacheAccess.hpp"
#include "common/CacheReport.hpp"
#include "common/CacheSpecs.hpp"
#include "common/Types.hpp"
#include <array>
#include <cstdint>
#include <cstring>
#include <experimental/filesystem>
#include <functional>
#include <glm/geometric.hpp>
#include <ios>
#include <iterator>
#include <memory>
#include <queue>
#include <string_view>
#include <utility>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/fwd.hpp"
#include "glm/gtx/norm.hpp" // For glm::distance2
#include <glm/gtx/hash.hpp>
#include <glm/gtx/io.hpp>

#include <chrono>
#include <format> // Required for std::format
#include <iostream>

Text::Text(Engine &engine, AssetManager &assets, std::string_view string,
           float spacing, std::string_view shader)
    : position(glm::vec3(0)), string(string), spacing(spacing) {
  populate(engine, assets, this->string, shader);
}

Text::Text(std::span<Engine::ID> span, std::string_view str, float spacing)
    : position(glm::vec3(0)) {
  objs.reserve(span.size());
  for (auto id : span) {
    objs.emplace_back(id);
  }

  this->string = str;
  this->spacing = spacing;
}

void Text::populate(Engine &engine, AssetManager &assets, std::string &string,
                    std::string_view shader) {
  for (size_t i = 0; i < string.length(); i++) {
    std::string letter = string[i] != '.' ? std::string(1, string[i]) : "dot";
    if (letter.front() == ' ') {
      Engine::ID id(nullptr);
      objs.push_back(id);
    } else {
      objs.push_back(engine.object(&assets.get_shader(std::string(shader)),
                                   &assets.get_mesh(letter)));

      Object &obj = engine.get(objs.back());

      obj.set_component<"Transform">(
             new Transform(glm::vec3(0), glm::vec3(0.01f), glm::vec3(0)))
          .set_component<"Color">(new Color(0xffffffff));
    }
  }
}

float Text::getSpacing() { return spacing; }
void Text::foreach (std::function<void(Engine::ID &)> callback) {
  if (!callback)
    return;

  for (auto &id : objs) {
    callback(id);
  }
}
void Text::foreach (std::function<void(Engine::ID &, size_t)> callback) {
  if (!callback)
    return;

  for (size_t i = 0; i < objs.size(); i++) {
    auto &id = objs[i];
    callback(id, i);
  }
}

std::span<Engine::ID> Text::getSpan(size_t begin, size_t amount) {
  auto beg = std::next(objs.begin(), (long)begin);
  auto end = std::next(beg, (long)amount);

  return std::span(beg, end);
}

Text Text::getSubText(size_t begin, size_t amount) {
  return {getSpan(begin, amount),
          std::string_view(string.c_str() + begin, amount), spacing};
}

void Text::changeText(Engine &engine, AssetManager &assets,
                      std::string_view newText, std::string_view shader) {
  size_t i = 0;
  glm::vec3 pos = position;
  pos.x -= spacing;

  for (; i < newText.length(); i++) {
    pos.x += spacing;
    std::string letter = newText[i] != '.' ? std::string(1, newText[i]) : "dot";
    if (newText[i] == ' ' || (i < string.length() && string[i] == newText[i])) {
      continue;
    }

    if (i < objs.size()) {
      if (objs[i]._M_node) {
        Engine::ID &id = objs[i];
        Object obj = std::move(engine.get(id));

        engine.rmv_object(id);
        obj.set_component<"Mesh">(&assets.get_mesh(letter));
        objs[i] = engine.add_object(obj);
      } else {
        Object &obj = engine.get(engine.object(
            &assets.get_shader(std::string(shader)), &assets.get_mesh(letter)));
        obj.set_component<"Transform">(
               new Transform(pos, glm::vec3(0.01f), glm::vec3(0)))
            .set_component<"Color">(new Color(0xffffffff));
        objs[i] = engine.add_object(obj);
      }

    } else if (i >= objs.size()) {
      auto it = objs.emplace_back(engine.object(
          &assets.get_shader(std::string(shader)), &assets.get_mesh(letter)));
      Object &obj = **it;

      obj.set_component<"Transform">(
             new Transform(pos, glm::vec3(0.01f), glm::vec3(0)))
          .set_component<"Color">(new Color(0xffffffff));
    }
  }

  string = newText;

  for (size_t j = objs.size() - 1; j >= i; j--) {
    engine.rmv_object(objs[j]);
    objs.erase(std::next(objs.begin(), (long)j));
  }
}

void Text::setPos(Engine &engine, glm::vec3 pos) {
  position = pos;

  foreach ([&](Engine::ID &id) {
    if (id._M_node && id->get()) {
      Object &object = engine.get(id);

      Transform *t = object.get_component<Transform>("Transform");
      t->position(pos);
    }
    pos.x += spacing;
  })
    ;
}

void Text::show(Engine &engine) {
  static const auto show = [&engine](Engine::ID &id) { engine.get(id).show(); };
  foreach (show)
    ;
}
void Text::hide(Engine &engine) {
  static const auto show = [&engine](Engine::ID &id) { engine.get(id).hide(); };
  foreach (show)
    ;
}

glm::vec3 Text::getPos() { return position; }

size_t Text::getLength() { return string.length(); }
std::string_view Text::getString() { return string; }

Simulator::Simulator(std::unique_ptr<Backend> &backend) : engine(1440, 960) {
  animator.setPlaybackRate(1.0);
  auto view = engine.get_view();
  Engine::ID sd_id =
      engine.object(&assets.get_shader("2d"), &assets.get_mesh("square"));
  Object &sidebar = engine.get(sd_id);
  sidebar
      .add_component(new Transform(glm::vec3(1040, 0, 35.f),
                                   glm::vec3(400, 960, 1), glm::vec3(0)))
      .add_component(new Color(0x181818ff));
  sidebar.onResize = [](Object &self, glm::vec<2, int> &window) {
    Transform *t = self.get_component<Transform>("Transform");
    auto pos = t->get_position();
    auto scale = t->get_scale();

    pos.x = (float)window.x - scale.x;
    scale.y = (float)window.y;

    t->position(pos);
    t->scale(scale);
  };

  engine.onKey = [&](int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
      float playback = animator.getPlaybackRate();
      if (key == GLFW_KEY_UP) {
        playback += 0.5f;
      } else if (key == GLFW_KEY_DOWN) {
        playback -= 0.5f;
      } else if (key == GLFW_KEY_LEFT) {
        playback -= 0.5f;
      } else if (key == GLFW_KEY_RIGHT) {
        playback += 0.5f;
      } else if (key == GLFW_KEY_SPACE) {
        animator.setPlaybackRate(1.0);
        return;
      }

      animator.setPlaybackRate(playback > 20.0 ? 20.0 : playback);
    }
  };

  Engine::ID bt_id =
      engine.object(&assets.get_shader("2d"), &assets.get_mesh("square"));
  Object &bottom = engine.get(bt_id);
  bottom
      .add_component(new Transform(glm::vec3(0, 0, 40.f),
                                   glm::vec3(1040, 300, 1), glm::vec3(0)))
      .add_component(new Color(0x44475Aff));

  bottom.onResize = [](Object &self, glm::vec<2, int> &window) {
    Transform *t = self.get_component<Transform>("Transform");
    auto scale = t->get_scale();
    scale.x = window.x;
    t->scale(scale);
  };

  Engine::ID hs_id =
      engine.object(&assets.get_shader("2d"), &assets.get_mesh("square"));
  Object &history = engine.get(hs_id);
  history
      .add_component(new Transform(glm::vec3(view.x - 400, 0, 20.f),
                                   glm::vec3(400, 300, 1), glm::vec3(0)))
      .add_component(new Color(0xff0000ff))
      .hide();

  // Engine::ID wl_id =
  //     engine.object(&assets.get_shader("2d"), &assets.get_mesh("square"));
  // Object &wall = engine.get(wl_id);
  // wall.add_component(new Transform(glm::vec3(0, 250, 80.f),
  //                                  glm::vec3(view.x - 400, view.y - 250, 1),
  //                                  glm::vec3(0)))
  //     .add_component(new Color(glm::vec4(0)));
  //
  // wall.onResize = [](Object &self, glm::vec<2, int> &window) {
  //   Transform *t = self.get_component<Transform>("Transform");
  //   auto scale = t->get_scale();
  //   scale.x = (float)window.x - 400;
  //   scale.y = (float)window.y - 250;
  //   t->scale(scale);
  // };

  const auto tagSize = backend->getCache().bits.tag;

  glm::vec3 setPos = glm::vec3{100, view.y - 350, 80};
  for (size_t i = 0; i < backend->getCache().assoc; i++) {
    CacheSet *set =
        &sets.emplace_back(setPos, glm::vec2{25, 40}, backend.get()->getCache(),
                           engine, assets, backend->getCache().nsets);

    auto pos = set->lookAt(0, 1);
    pos.z = setPos.z - 5;
    texts.emplace_back(engine, assets, "Tag", 25, "2d_camera");
    Text &tagLabel = texts.back();
    pos.y += 60;
    pos.x -= 25.f * 3 / 2;
    tagLabel.setPos(engine, pos);

    pos = set->lookAt(0, 2);
    texts.emplace_back(engine, assets, "Information", 25, "2d_camera");
    Text &infoLabel = texts.back();
    pos.y += 60;
    pos.x -= ((float)infoLabel.getLength() * infoLabel.getSpacing()) / 2;
    infoLabel.setPos(engine, pos);

    std::string str(set->getInfoSize(), 'X');
    for (size_t i = 0; i < backend->getCache().nsets; i++) {
      glm::vec3 pos = set->getPos(i, 2);

      texts.emplace_back(engine, assets, str, 25, "2d_camera");
      Text &bin = texts.back();

      pos.x += 2;
      pos.z = setPos.z - 5;
      bin.setPos(engine, pos);
    }

    setPos.x += set->getXOff(3) + 20;
  }

  const auto borderColor = glm::vec4(0, 0, 255, 255);

  size_t fieldLabelId = texts.size();
  texts.emplace_back(engine, assets, "Tag", 25, "2d");
  Text &fieldLabel = texts.back();
  fieldLabel.foreach ([this, &borderColor](Engine::ID &id) {
    Object &obj = engine.get(id);
    Color *color = obj.get_component<Color>("Color");
    color->color = borderColor;
    color->color.a = 0;
    obj.hide();
  });

  fieldBlock[0] = std::make_pair(
      engine.object(&assets.get_shader("border"), &assets.get_mesh("square")),
      fieldLabelId);
  Engine::ID &id = std::get<0>(fieldBlock[0]);
  Object &box = engine.get(id);

  float width = fieldLabel.getSpacing() * (float)(tagSize) + 10;
  float height = sets.back().getOff().y + 20;

  Variable *vars = new Variable();
  vars->set("u_borderColor", borderColor);
  vars->set("u_fillColor", glm::vec4(0));
  vars->set("u_rectPixelSize", glm::vec2(width, height));
  vars->set("u_borderPixelWidth", 2.f);
  vars->set("u_progress", 0.f);

  box.add_component(
         new Transform(glm::vec3(0), glm::vec3(width, height, 1), glm::vec3(0)))
      .add_component(vars)
      .hide();

  const auto indexBorderColor = glm::vec4(255, 0, 0, 255);
  size_t indexFieldLabelID = texts.size();
  texts.emplace_back(engine, assets, "Index", 25, "2d");
  Text &indexFieldLabel = texts.back();
  indexFieldLabel.foreach ([this, &indexBorderColor](Engine::ID &id) {
    Object &obj = engine.get(id);
    Color *color = obj.get_component<Color>("Color");
    color->color = indexBorderColor;
    color->color.a = 0;
    obj.hide();
  });

  fieldBlock[1] = std::make_pair(
      engine.object(&assets.get_shader("border"), &assets.get_mesh("square")),
      indexFieldLabelID);
  Engine::ID &indexId = std::get<0>(fieldBlock[1]);
  Object &indexBox = engine.get(indexId);

  width =
      indexFieldLabel.getSpacing() * (float)(backend->getCache().bits.index) +
      10;
  height = sets.back().getOff().y + 20;

  vars = new Variable();

  vars->set("u_borderColor", indexBorderColor);
  vars->set("u_fillColor", glm::vec4(0));
  vars->set("u_rectPixelSize", glm::vec2(width, height));
  vars->set("u_borderPixelWidth", 2.f);
  vars->set("u_progress", 0.f);

  indexBox
      .add_component(new Transform(glm::vec3(0), glm::vec3(width, height, 1),
                                   glm::vec3(0)))
      .add_component(vars)
      .hide();

  const auto offsetBorderColor = glm::vec4(204, 85, 0, 255);
  size_t offsetLabelId = texts.size();
  texts.emplace_back(engine, assets, "Offset", 25, "2d");
  Text &offsetLabel = texts.back();
  offsetLabel.foreach ([this, &offsetBorderColor](Engine::ID &id) {
    Object &obj = engine.get(id);
    Color *color = obj.get_component<Color>("Color");
    color->color = offsetBorderColor;
    color->color.a = 0;
    obj.hide();
  });

  fieldBlock[2] = std::make_pair(
      engine.object(&assets.get_shader("border"), &assets.get_mesh("square")),
      offsetLabelId);
  Engine::ID &offsetId = std::get<0>(fieldBlock[2]);
  Object &offsetBox = engine.get(offsetId);

  width =
      offsetLabel.getSpacing() * (float)(backend->getCache().bits.offset) + 10;
  height = sets.back().getOff().y + 20;

  vars = new Variable();

  vars->set("u_borderColor", offsetBorderColor);
  vars->set("u_fillColor", glm::vec4(0));
  vars->set("u_rectPixelSize", glm::vec2(width, height));
  vars->set("u_borderPixelWidth", 2.f);
  vars->set("u_progress", 0.f);

  offsetBox
      .add_component(new Transform(glm::vec3(0), glm::vec3(width, height, 1),
                                   glm::vec3(0)))
      .add_component(vars)
      .hide();

  std::array<uint32_t, 6> colors = {0xff0000ff, 0x0000ffff, 0xffff00ff,
                                    0x00ffffff, 0x000000ff, 0x00ff00ff};

  for (size_t i = 0; i < pathObjs.size(); i++) {
    pathObjs[i] =
        engine.object(&assets.get_shader("path"), &assets.get_mesh("square"));
    Object &obj = engine.get(pathObjs[i]);

    obj.add_component(
           new Transform(glm::vec3(0, 0, 65), glm::vec3(1), glm::vec3(0)))
        .add_component(new Variable())
        .add_component(new Color(colors[i]));
    obj.hide();
  }

  for (size_t i = 0; i < symbolObjs.size(); i++) {
    symbolObjs[i] = engine.object(&assets.get_shader("texture_camera"),
                                  &assets.get_mesh("square"));
    Object &obj = engine.get(symbolObjs[i]);

    obj.add_component(
           new Transform(glm::vec3(0, 0, 60), glm::vec3(50), glm::vec3(0)))
        .add_component(new Variable())
        .add_component(new Texture(i == 0 ? "comparator.png" : "and.png",
                                   GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA,
                                   GL_UNSIGNED_BYTE));
    obj.show();
  }

  // discrete_t accesses = 0;
  // discrete_t hits = 0;
  // discrete_t miss = 0;
  // discrete_t compulsory_miss = 0;
  // discrete_t conflict_miss = 0;
  // discrete_t capacity_miss = 0;
  // percentage_t miss_rate = 0.0f;
  // percentage_t hit_rate = 0.0f;
  // percentage_t compulsory_miss_rate = 0.0f;
  // percentage_t capacity_miss_rate = 0.0f;
  // percentage_t conflict_miss_rate = 0.0f;

  const char *metrics[][2] = {
      {"Accesses", "{:>{}}"},
      {"Hits", "{:>{}}"},
      {"Miss", "{:>{}}"},
      {"Compulsory Miss", "{:>{}}"},
      {"Conflict Miss", "{:>{}}"},
      {"Capacity Miss", "{:>{}}"},

      {"Miss Rate", "{.2}"},
      {"Hit Rate", "{.2}"},
      {"Compulsory MissRate", "{.2}"},
      {"Capacity MissRate", "{.2}"},
      {"Conflict MissRate", "{.2}"},
  };

  Transform *t = bottom.get_component<Transform>("Transform");

  glm::vec3 pos = t->get_position();
  pos.y += t->get_scale().y - 50;
  const float original_y = pos.y;
  pos.x += 10;
  pos.z -= 5;
  size_t longest = 0;
  const size_t amount = sizeof(metrics) / sizeof(*metrics);
  size_t j = 0;
  for (size_t i = 0; i <= amount; i++) {
    if ((i % 6 == 0 && i > 0) || i == amount) {
      pos.y = original_y;
      glm::vec3 value_pos = pos;
      value_pos.x += 25.f * (float)longest;
      for (; j < i; j++) {
        std::string format;
        if (strstr(metrics[j][0], "Rate") == NULL) {
          format = std::format("{:>{}}", 0, value_digits);
        } else {
          size_t decimal_places = value_digits - 2;
          format = std::format("{:>{}.{}f}", 0.f, value_digits, decimal_places);
        }
        size_t valueId = texts.size();
        texts.emplace_back(engine, assets, format, 25, "2d");
        Text &valueLabel = texts.back();
        valueLabel.setPos(engine, value_pos);
        this->resLabel[j].value = valueId;

        value_pos.y -= 45;
      }
      pos.x += 100.f + value_pos.x + 25.f * (float)value_digits;

      if (i == amount)
        break;
    }
    std::string format = std::format("{}: ", metrics[i][0], 0);
    if (format.length() > longest) {
      longest = format.length();
    }

    size_t labelId = texts.size();
    texts.emplace_back(engine, assets, format, 25, "2d");
    Text &reportLabel = texts.back();
    reportLabel.setPos(engine, pos);
    this->resLabel.emplace_back(labelId, 0, (void *)NULL);

    pos.y -= 45;
  }

  binId = texts.size();
  texts.emplace_back(engine, assets, "000", 25, "cutting");

  auto hideText = [this](Engine::ID &id) { engine.get(id).hide(); };

  resultLabel.first = texts.size();
  texts.emplace_back(engine, assets, "Compulsory", 25, "2d_camera")
      .foreach (hideText);
  resultLabel.second = texts.size();
  texts.emplace_back(engine, assets, "Miss", 25, "2d_camera")
      .foreach (hideText);
}

auto Simulator::populateAddrs(std::queue<addr_t> &addrs) -> HistCycleInfo {
  constexpr size_t in = 30;
  texts.reserve(texts.capacity() + in + 5);
  const auto view = engine.get_view();

  size_t f = texts.size();
  float y = (float)view.y - 40.f;
  for (size_t i = 0; i < in; i++) {
    addr_t addr = addrs.front();
    addrs.pop();

    std::cout << i << ": " << addr << '\n';
    std::string hex_string = std::format("0x{:08x}", addr);
    texts.emplace_back(engine, assets, hex_string, 26, "cutting");
    Text &text = texts.back();
    text.setPos(engine, glm::vec3(view.x - 400 + 10, y, 20));
    y -= 40;
    text.foreach ([&](Engine::ID &id) {
      Object &obj = engine.get(id);
      Variable *vars = new Variable();
      vars->set("v_border", glm::vec2{view.x - 400, view.y});
      vars->set("orient", false);

      obj.add_component(vars);
    });
  }

  auto end = texts.size() - 1;
  auto bottom = texts.back().getPos();
  bottom.y -= 40;

  return {.bottom = bottom, .begin = f, .amount = in, .end = end};
}

auto Simulator::decomposeAddr(std::queue<addr_t> &addrs, HistCycleInfo &info,
                              Text &top, Backend *backend, CacheAccess access)
    -> void {
  addr_t addr = access.orig;

  Text *bin = &texts[binId];
  std::string hex_string = std::format("{:032b}", addr);
  auto view = engine.get_view();
  bin->changeText(engine, assets, hex_string, "cutting");
  bin->setPos(engine, glm::vec3(view.x - 400 + 10, view.y - 50, 20.f));
  bin->foreach ([&](Engine::ID &id) {
    Object &obj = engine.get(id);
    Variable *vars = new Variable();

    vars->set("v_border", glm::vec2{view.x - 400, 0});
    vars->set("orient", true);

    obj.add_component(vars);
  });

  const size_t tagSize = backend->getCache().bits.tag;

  std::vector<AnimationManager::Animation> textAnim = {
      AnimationManager::Animation(
          [this, &top, &addrs, &info](float progress, bool inverse) {
            static Text *bin = &texts[binId];
            static glm::vec3 original = top.getPos();
            static glm::vec3 dest = original - glm::vec3(300, 0, 0);
            static glm::vec3 binOriginal = bin->getPos();
            static glm::vec3 binDest = {binOriginal.x - bin->getSpacing() * 32,
                                        binOriginal.y, binOriginal.z};

            if (inverse) {
              glm::vec3 temp = original;
              original = dest;
              dest = temp;
            }

            top.setPos(engine,
                       AnimationManager::lerp(original, dest, progress));
            bin->setPos(engine,
                        AnimationManager::lerp(binOriginal, binDest, progress));

            if (progress >= 1.0) {
              populateBottom(addrs, info);
            }
          },
          2.f),
      AnimationManager::Animation(
          [this](float progress, bool inverse) {
            static Text *bin = &texts[binId];
            static glm::vec3 original = bin->getPos();
            static glm::vec3 dest(150, original.y - 50, original.z);

            if (inverse) {
              glm::vec3 temp = original;
              original = dest;
              dest = temp;
            }

            bin->setPos(engine,
                        AnimationManager::lerp(original, dest, progress));
          },
          2.f),
      AnimationManager::Animation(
          [this, access, tagSize](float progress, bool inverse) {
            static Text *bin = &texts[binId];
            static glm::vec3 original = bin->getPos();
            static glm::vec3 dest(150, original.y - 25, original.z);

            if (inverse) {
              glm::vec3 temp = original;
              original = dest;
              dest = temp;
            }

            if (progress >= 1.0) {
              for (auto &idx : fieldBlock) {
                Engine::ID &id = std::get<0>(idx);
                Text &label = texts[std::get<1>(idx)];

                engine.get(id).show();
                label.foreach (
                    [this](Engine::ID &id) { engine.get(id).show(); });
              }

              showField(*bin, tagSize, access);
            }

            bin->setPos(engine,
                        AnimationManager::lerp(original, dest, progress));
          },
          2.f),
  };

  auto &ttext = animator.emplace(textAnim);
  ttext.autoNext = true;
}

auto Simulator::populateBottom(std::queue<addr_t> &addrs,
                               HistCycleInfo &populateBottom) -> void {
  std::vector<glm::vec3> original(populateBottom.amount);
  std::vector<glm::vec3> dest(populateBottom.amount);

  std::cout << "Remaining: " << addrs.size() << '\n';
  if (addrs.empty()) {
    Text &text = texts[populateBottom.begin + populateBottom.i];
    text.foreach ([this](Engine::ID &id) { engine.get(id).hide(); });
  } else {
    addr_t addr = addrs.front();
    addrs.pop();

    std::string hex_string = std::format("0x{:08x}", addr);
    Text &text = texts[populateBottom.begin + populateBottom.i];
    text.changeText(engine, assets, hex_string, "cutting");
    text.setPos(engine, populateBottom.bottom);
  }

  for (size_t i = 0; i < populateBottom.amount; i++) {
    size_t idx =
        populateBottom.begin + (populateBottom.i + i) % populateBottom.amount;
    Text &text = texts[idx];

    original[i] = text.getPos();
    dest[i] = glm::vec3(original[i].x, original[i].y + 40, original[i].z);
  }

  AnimationManager::AnimationTrack track({AnimationManager::Animation(
      [this, &populateBottom, original, dest](float progress, bool inverse) {
        for (size_t i = 0; i < populateBottom.amount; i++) {
          size_t idx = populateBottom.begin +
                       (populateBottom.i + i) % populateBottom.amount;
          Text &text = texts[idx];

          glm::vec3 final =
              AnimationManager::lerp(original[i], dest[i], progress);
          text.setPos(engine, final);
        }

        if (progress == 1.0) {
          populateBottom.i = (populateBottom.i + 1) % populateBottom.amount;
        }
      },
      0.5f)});
  track.i = 0;
  track.autoNext = true;
  animator.emplace(track);
}

auto Simulator::tick(Backend *backend, std::queue<addr_t> &addrs) -> void {
  static std::queue<addr_t> tempAddrs(addrs);
  static auto histInfo = [&]() { return populateAddrs(tempAddrs); }();
  this->backend = backend;

  static auto start = std::chrono::steady_clock::now();
  auto now = std::chrono::steady_clock::now();
  auto elapsed = now - start;

  float elapsed_s =
      std::chrono::duration_cast<floating_seconds>(elapsed).count();
  addr_t addr = addrs.front();

  if (requestNew) {
    auto access = backend->process(addr);
    decomposeAddr(tempAddrs, histInfo, texts[histInfo.begin + histInfo.i],
                  backend, access);
    requestNew = false;
    addrs.pop();
  }
  animator(elapsed_s);
  engine.update();

  start = now;
}
auto Simulator::halted() -> bool { return engine.halted(); }
auto Simulator::showField(Text &bin, discrete_t tagSize, CacheAccess access)
    -> void {
  constexpr float delta = 20;
  Engine::ID &id = std::get<0>(fieldBlock[0]);
  Text &label = texts[std::get<1>(fieldBlock[0])];
  Object &box = engine.get(id);
  Transform *t = box.get_component<Transform>("Transform");

  Text text = bin.getSubText(0, tagSize);
  glm::vec3 orig = bin.getPos();
  glm::vec3 pos = orig;
  pos.x -= delta;
  glm::vec3 target = pos;

  float height = t->get_scale().y;

  pos.x -= 6;
  pos.y -= (height - 25) / 2;

  glm::vec3 labelPos = pos;
  labelPos.x += t->get_scale().x / 2.0f -
                ((float)label.getLength() * 0.5f) * label.getSpacing();
  labelPos.y += height + 15;

  label.setPos(engine, labelPos);
  t->position(pos);
  label.setPos(engine, labelPos);
  label.foreach ([this](Engine::ID &id) {
    Object &obj = engine.get(id);
    obj.show();
  });
  box.show();

  Engine::ID &indexId = std::get<0>(fieldBlock[1]);
  Text &indexLabel = texts[std::get<1>(fieldBlock[1])];
  Object &indexBox = engine.get(indexId);
  t = indexBox.get_component<Transform>("Transform");

  glm::vec3 indexBinOrig = bin.getPos();
  indexBinOrig.x += bin.getSpacing() * (float)tagSize;

  glm::vec3 indexOrig = bin.getPos();
  indexOrig.x += bin.getSpacing() * (float)tagSize;

  indexOrig.x -= 6;
  indexOrig.y -= (height - 25) / 2;

  labelPos = indexOrig;
  labelPos.x +=
      t->get_scale().x / 2.0f -
      ((float)indexLabel.getLength() * 0.5f) * indexLabel.getSpacing();
  labelPos.y += height + 15;

  t->position(indexOrig);

  indexLabel.setPos(engine, labelPos);
  indexLabel.foreach ([this](Engine::ID &id) {
    Object &obj = engine.get(id);
    obj.show();
  });

  indexBox.show();

  Engine::ID &offsetId = std::get<0>(fieldBlock[2]);
  Text &offsetLabel = texts[std::get<1>(fieldBlock[2])];
  Object &offsetBox = engine.get(offsetId);
  t = offsetBox.get_component<Transform>("Transform");

  const float deltaField =
      (float)tagSize + (float)backend->getCache().bits.index;
  glm::vec3 offsetBinOrig = bin.getPos();
  offsetBinOrig.x += bin.getSpacing() * deltaField;
  glm::vec3 offsetBinDest = offsetBinOrig;
  offsetBinDest.x += delta;

  glm::vec3 offsetOrig = offsetBinDest;

  offsetOrig.x -= 6;
  offsetOrig.y -= (height - 25) / 2;

  labelPos = offsetOrig;
  labelPos.y += height + 15;

  t->position(offsetOrig);

  offsetLabel.setPos(engine, labelPos);
  offsetLabel.foreach ([this](Engine::ID &id) {
    Object &obj = engine.get(id);
    obj.show();
  });

  offsetBox.show();
  const size_t indexSize = backend->getCache().bits.index;

  AnimationManager::AnimationTrack track(
      {{[this, &bin, orig, target, offsetBinDest, offsetBinOrig, indexSize,
         tagSize](float progress, bool inverse) {
          Text tagText = bin.getSubText(0, tagSize);
          Text offsetText =
              bin.getSubText(tagSize + indexSize, 32 - (tagSize + indexSize));

          glm::vec3 final = AnimationManager::lerp(orig, target, progress);
          tagText.setPos(engine, final);
          final =
              AnimationManager::lerp(offsetBinOrig, offsetBinDest, progress);
          offsetText.setPos(engine, final);
        },
        0.2f},
       {[this, &box, &label, &indexBox, &indexLabel, &offsetLabel, &offsetBox,
         access](float progress, bool inverse) {
          Variable *vars = box.get_component<Variable>("Variable");
          vars->set<float>("u_progress", (float)progress);
          vars = indexBox.get_component<Variable>("Variable");
          vars->set<float>("u_progress", (float)progress);
          vars = offsetBox.get_component<Variable>("Variable");
          vars->set<float>("u_progress", (float)progress);

          label.foreach ([this, &progress](Engine::ID &id) {
            Object &obj = engine.get(id);
            Color *color = obj.get_component<Color>("Color");
            color->color = glm::vec4(color->color.r, color->color.g,
                                     color->color.b, progress);
          });

          indexLabel.foreach ([this, &progress](Engine::ID &id) {
            Object &obj = engine.get(id);
            Color *color = obj.get_component<Color>("Color");
            color->color = glm::vec4(color->color.r, color->color.g,
                                     color->color.b, progress);
          });

          offsetLabel.foreach ([this, &progress](Engine::ID &id) {
            Object &obj = engine.get(id);
            Color *color = obj.get_component<Color>("Color");
            color->color = glm::vec4(color->color.r, color->color.g,
                                     color->color.b, progress);
          });

          if (progress == 1.0) {
            CacheAccess acc = access;
            drawConnection(box, indexBox, acc);
          }
        },
        0.8f}});
  track.i = 0;
  track.autoNext = true;
  animator.emplace(track);
}

struct PathVertex final : public MeshVertex {
  struct Vertex {
    glm::vec3 vert;
    float distance;
    float totalDistance;
  } vec;

  PathVertex(glm::vec3 data, float distance, float totalDistance)
      : vec(data, distance, totalDistance) {}

  auto operator()() -> void override {
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, size(),
                          (void *)offsetof(Vertex, vert));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, size(),
                          (void *)offsetof(Vertex, distance));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, size(),
                          (void *)offsetof(Vertex, totalDistance));
  }

  auto size() -> int override { return sizeof(this->vec); }
  auto hash() const -> size_t override {
    return std::hash<glm::vec3>{}(vec.vert);
  }
  auto equal(const MeshVertex *other) const -> bool override {
    const PathVertex *vert = reinterpret_cast<decltype(vert)>(other);
    return vert->vec.vert == vec.vert;
  }
  auto setData(void *data) -> void * override {
    std::memcpy(data, &vec, sizeof(vec));
    return static_cast<uint8_t *>(data) + sizeof(vec);
  }
};

// static auto triangulate(std::vector<glm::vec3> &path, glm::vec2 size,
//                         std::vector<std::unique_ptr<MeshVertex>> &meshData)
//     -> void {
//   std::array<glm::vec3, 4> derived;
//   glm::vec2 halfSize = size * 0.5f;
//   float currDistance = 0;
//   float totalDistance = 0;
//
//   std::cout << "Vertices:\n";
//   for (size_t i = 1; i < path.size(); i++) {
//     glm::vec3 prev = path[i - 1];
//     glm::vec3 curr = path[i];
//
//     totalDistance += glm::distance(curr, prev);
//     std::cout << '\t' << prev << " -> " << curr << '\n';
//   }
//
//   std::cout << "\nShape:\n";
//
//   for (size_t i = 1; i < path.size(); i++) {
//     glm::vec3 prev = path[i - 1];
//     glm::vec3 curr = path[i];
//
//     glm::vec3 direction = glm::normalize(curr - prev);
//     glm::vec3 normal = glm::vec3(-direction.y, direction.x, 0.0f);
//     currDistance += glm::distance(curr, prev);
//
//     derived[0] = prev - normal * halfSize.x;
//     derived[1] = prev + normal * halfSize.x;
//     derived[2] = curr - normal * halfSize.x;
//     derived[3] = curr + normal * halfSize.x;
//
//     const std::array<size_t, 6> indices = {0, 1, 2, 1, 2, 3};
//     for (size_t index : indices) {
//       std::cout << '\t' << derived[index] << '\n';
//       meshData.emplace_back(
//           new PathVertex(derived[index], currDistance, totalDistance));
//     }
//   }
// }

static auto triangulate(const std::vector<glm::vec3> &path, float lineWidth,
                        std::vector<std::unique_ptr<MeshVertex>> &meshData)
    -> void {
  if (path.size() < 2)
    return;

  float halfWidth = lineWidth * 0.5f;
  float totalDistance = 0;
  std::vector<float> cumulativeDistances(path.size(), 0.0f);

  // --- Step 1: Pre-calculate path distances ---
  for (size_t i = 1; i < path.size(); ++i) {
    totalDistance += glm::distance(path[i], path[i - 1]);
    cumulativeDistances[i] = totalDistance;
  }

  // --- Step 2: Generate the continuous vertex pairs for the strip ---
  std::vector<PathVertex> triangleStripVertices;
  for (size_t i = 0; i < path.size(); ++i) {
    glm::vec3 current_point = path[i];
    glm::vec3 prev_point = (i > 0) ? path[i - 1] : current_point;
    glm::vec3 next_point = (i < path.size() - 1) ? path[i + 1] : current_point;

    glm::vec3 dir_to_prev =
        (i > 0) ? glm::normalize(current_point - prev_point) : glm::vec3(0);
    glm::vec3 dir_to_next = (i < path.size() - 1)
                                ? glm::normalize(next_point - current_point)
                                : glm::vec3(0);

    glm::vec3 normal;
    if (i == 0) {
      normal = glm::vec3(-dir_to_next.y, dir_to_next.x, 0.0f);
    } else if (i == path.size() - 1) {
      normal = glm::vec3(-dir_to_prev.y, dir_to_prev.x, 0.0f);
    } else {
      glm::vec3 n1 = glm::vec3(-dir_to_prev.y, dir_to_prev.x, 0.0f);
      glm::vec3 n2 = glm::vec3(-dir_to_next.y, dir_to_next.x, 0.0f);
      normal = glm::normalize(n1 + n2);
    }

    if (glm::length2(normal) < 0.001f) {
      normal = glm::vec3(-dir_to_prev.y, dir_to_prev.x, 0.0f);
    }

    glm::vec3 vertex_a = current_point + normal * halfWidth;
    glm::vec3 vertex_b = current_point - normal * halfWidth;

    triangleStripVertices.emplace_back(vertex_a, cumulativeDistances[i],
                                       totalDistance);
    triangleStripVertices.emplace_back(vertex_b, cumulativeDistances[i],
                                       totalDistance);
  }

  // --- Step 3: Triangulate the vertex strip into the final meshData ---
  meshData.clear();
  // We need at least 4 vertices (2 pairs) to form the first quad.
  for (size_t i = 0; i <= triangleStripVertices.size() - 4; i += 2) {
    // Get references to the four corners of the current quad in the strip
    const auto &v0 = triangleStripVertices[i];     // Top-left
    const auto &v1 = triangleStripVertices[i + 1]; // Bottom-left
    const auto &v2 = triangleStripVertices[i + 2]; // Top-right
    const auto &v3 = triangleStripVertices[i + 3]; // Bottom-right

    // --- THE FIX IS HERE ---
    // Explicitly use all data members when creating the new vertices.

    // Triangle 1: (Top-left, Bottom-left, Top-right)
    meshData.emplace_back(
        new PathVertex(v0.vec.vert, v0.vec.distance, v0.vec.totalDistance));
    meshData.emplace_back(
        new PathVertex(v1.vec.vert, v1.vec.distance, v1.vec.totalDistance));
    meshData.emplace_back(
        new PathVertex(v2.vec.vert, v2.vec.distance, v2.vec.totalDistance));

    // Triangle 2: (Bottom-left, Bottom-right, Top-right)
    meshData.emplace_back(
        new PathVertex(v1.vec.vert, v1.vec.distance, v1.vec.totalDistance));
    meshData.emplace_back(
        new PathVertex(v3.vec.vert, v3.vec.distance, v3.vec.totalDistance));
    meshData.emplace_back(
        new PathVertex(v2.vec.vert, v2.vec.distance, v2.vec.totalDistance));
  }
}

auto Simulator::drawConnection(Object &box, Object &indexBox,
                               CacheAccess access) -> void {
  const CacheSpecs &specs = backend->getCache();
  const uint32_t tagMask = (0xffffffff) << (32 - specs.bits.tag);
  const uint32_t indexMask = ~(tagMask) & (0xffffffff << specs.bits.offset);
  const uint32_t index = (access.orig & indexMask) >> specs.bits.offset;
  const uint32_t tag = (access.orig & tagMask) >> (32 - specs.bits.tag);
  const uint32_t set = (uint32_t)access.block;
  const float width = 2;
  const float endy = sets[set].lookAt(specs.nsets - 1).y - 100;
  const float tagEndPad = 40;
  const float valPad = 30;
  const float pad = 10;
  float endx;

  const auto updatePath =
      [this](size_t i, std::vector<glm::vec3> &path, std::string pathName,
             float width, std::vector<std::unique_ptr<MeshVertex>> &data) {
        data.clear();
        triangulate(path, width, data);

        Object obj = std::move(engine.get(pathObjs[i]));
        assets.register_mesh(pathName, data);

        obj.set_component<"Mesh">(&assets.get_mesh(pathName));
        engine.rmv_object(pathObjs[i]);
        pathObjs[i] = engine.add_object(obj);
      };

  std::vector<glm::vec3> pathIndex = [this, &indexBox, index, set]() {
    std::vector<glm::vec3> path;

    Transform *t = indexBox.get_component<Transform>("Transform");
    glm::vec3 vertex(t->get_position());

    vertex.z = 0;
    vertex.x += t->get_scale().x / 2;
    path.push_back(vertex);

    vertex.y -= 50;

    path.push_back(vertex);

    glm::vec3 setPos = sets[set].lookAt(index, 0);
    vertex.x = setPos.x - 100;
    path.push_back(vertex);

    vertex.y = setPos.y;
    path.push_back(vertex);

    vertex.x = setPos.x;
    path.push_back(vertex);

    return path;
  }();
  std::vector<std::unique_ptr<MeshVertex>> data;

  std::vector<glm::vec3> pathTag = [this, &box, pad, &endx, set, index, endy,
                                    tagEndPad]() {
    std::vector<glm::vec3> path;

    Transform *t = box.get_component<Transform>("Transform");
    glm::vec3 vertex(t->get_position());

    vertex.z = 0;
    vertex.x += t->get_scale().x / 2;
    path.push_back(vertex);

    vertex.y -= 40;

    path.push_back(vertex);

    glm::vec3 setPos = sets[set].lookAt(index, 0);
    vertex.x = setPos.x - 125;
    path.push_back(vertex);

    vertex.y = endy;
    path.push_back(vertex);

    vertex.x = sets[set].lookAt(index, 1).x - pad;
    path.push_back(vertex);

    vertex.y -= tagEndPad;
    path.push_back(vertex);

    endx = vertex.x;

    return path;
  }();

  glm::vec3 valEnd;
  std::vector<glm::vec3> pathVal = [this, &valEnd, set, index, valPad, endy]() {
    std::vector<glm::vec3> path;

    glm::vec3 vertex(sets[set].lookAt(index, 0));

    vertex.z = 0;
    path.push_back(vertex);

    vertex.x += 25;

    path.push_back(vertex);

    vertex.y = endy - valPad;
    path.push_back(vertex);

    vertex.x -= 200;
    path.push_back(vertex);

    valEnd = vertex;

    return path;
  }();

  std::vector<glm::vec3> pathTagBlock = [this, &endx, set, index, tagEndPad,
                                         endy]() {
    std::vector<glm::vec3> path;

    glm::vec3 vertex(sets[set].lookAt(index, 1));

    vertex.z = 0;
    vertex.y -= sets[set].getOff().y / 2.f - 5;
    path.push_back(vertex);

    vertex.y = endy;
    path.push_back(vertex);

    vertex.y -= tagEndPad;
    path.push_back(vertex);

    endx += std::abs(endx - vertex.x) * 0.5f;

    return path;
  }();

  Object &comp = engine.get(symbolObjs[0]);
  Transform *t = comp.get_component<Transform>("Transform");

  glm::vec3 scale = t->get_scale();
  glm::vec3 pos(endx - scale.x * 0.5f, pathTag.back().y - scale.y * 0.5f,
                t->get_position().z);
  t->position(pos);

  std::vector<glm::vec3> pathCompOut = [pos, valEnd, t]() {
    std::vector<glm::vec3> path;

    glm::vec3 vertex(pos);
    vertex += t->get_scale() * 0.5f;

    vertex.z = 0;
    path.push_back(vertex);

    vertex.x -= 200;
    path.push_back(vertex);

    float old = vertex.y;
    vertex.y = valEnd.y - 10;
    if (old == vertex.y) {
      vertex.y += 5;
    }
    path.push_back(vertex);

    old = valEnd.x;
    vertex.x = valEnd.x;
    if (old == vertex.x) {
      vertex.x -= 10;
    }
    path.push_back(vertex);

    return path;
  }();

  const uint32_t COLOR_ON = 0x00ff00ff;
  const uint32_t COLOR_OFF = 0xaaaaaaff;

  Object &andPort = engine.get(symbolObjs[1]);
  t = andPort.get_component<Transform>("Transform");
  scale = t->get_scale();

  t->position(pathVal.back() - glm::vec3(scale.x, scale.y * 0.5f, 0));

  std::vector<glm::vec3> pathAndOut = [&scale, t]() {
    std::vector<glm::vec3> path;

    glm::vec3 vertex(t->get_position());
    vertex += t->get_scale() * 0.5f;

    vertex.z += 5;
    path.push_back(vertex);

    vertex.x -= 150;
    path.push_back(vertex);
    return path;
  }();

  std::vector<std::vector<glm::vec3> *> paths = {
      &pathIndex, &pathTag, &pathVal, &pathTagBlock, &pathCompOut, &pathAndOut,
  };
  const char *pathNames[] = {
      "pathIndex",    "pathTag",     "pathVal",
      "pathTagBlock", "pathCompOut", "pathAndOut",
  };

  for (size_t i = 0; i < pathObjs.size(); i++) {
    updatePath(i, *paths[i], pathNames[i], width, data);

    Object &obj = engine.get(pathObjs[i]);
    obj.show();
    Variable *vars = obj.get_component<Variable>("Variable");
    vars->set("u_progress", 0.0f);

    if (i > 3) {
      if (access.res == AccessResult::HIT) {
        obj.get_component<Color>("Color")->color = Color::hextorgba(COLOR_ON);
      } else {
        obj.get_component<Color>("Color")->color = Color::hextorgba(COLOR_OFF);
      }
    }
  }

  glm::vec2 final = glm::vec2(sets[set].lookAt(index)) -
                    (glm::vec2(engine.get_view()) + glm::vec2(0, 250)) / 2.f;

  glm::vec3 endPath = pathAndOut.back();
  for (auto &id : symbolObjs) {
    engine.get(id).show();
  }

  animator.add_track({{
      {[this, first = true, final](float progress, bool inverse) mutable {
         static glm::vec2 st;
         if (first) {
           engine.getCamera().getPos();
           first = false;
         }

         for (size_t i = 0; i < 2; i++) {
           auto &id = pathObjs[i];
           Object &obj = engine.get(id);

           Variable *vars = obj.get_component<Variable>("Variable");
           if (i == 0) {
             vars->set("u_progress", (float)progress);
           } else {
             vars->set("u_progress", (float)progress / 2.0f);
           }
         }

         if (progress == 1.0) {
           first = true;
         }

         engine.getCamera().setPosition(
             AnimationManager::lerp(st, final, progress));
       },
       5.f},
      {[this, first = true, endy](float progress, bool inverse) mutable {
         static glm::vec2 st;
         static glm::vec2 final;
         if (first) {
           st = engine.getCamera().getPos();
           final =
               glm::vec2(st.x, endy) - glm::vec2(0, engine.get_view().y / 2);
           first = false;
         }
         for (size_t i = 1; i < 4; i++) {
           auto &id = pathObjs[i];
           Object &obj = engine.get(id);

           Variable *vars = obj.get_component<Variable>("Variable");
           if (i == 1) {
             vars->set("u_progress", (float)progress / 2.0f + 0.5f);
           } else {
             vars->set("u_progress", (float)progress);
           }
         }

         if (progress == 1.0) {
           first = true;
         }
         engine.getCamera().setPosition(
             AnimationManager::lerp(st, final, progress));
       },
       5.f},
      {[this](float progress, bool inverse) {
         auto &id = pathObjs[4];
         Object &obj = engine.get(id);

         Variable *vars = obj.get_component<Variable>("Variable");
         vars->set("u_progress", (float)progress);
       },
       1.f},
      {[this](float progress, bool inverse) {
         auto &id = pathObjs[5];
         Object &obj = engine.get(id);

         Variable *vars = obj.get_component<Variable>("Variable");
         vars->set("u_progress", (float)progress);
       },
       1.f},
      {[this, first = true, access, endPath](float progress,
                                             bool inverse) mutable {
         Text &upperLabel = texts[std::get<0>(resultLabel)];
         Text &bottomLabel = texts[std::get<1>(resultLabel)];

         static glm::vec2 cameraPos;
         static glm::vec2 cameraDest;

         if (first) {
           void *data = &backend->report().accesses;
           for (size_t j = 0; j < resLabel.size(); j++) {
             std::string format;
             std::string_view valueStr = texts[resLabel[j].label].getString();
             if (valueStr.find("Rate") == valueStr.npos) {
               format =
                   std::format("{:>{}}", *(discrete_t *)(data), value_digits);
               data = (discrete_t *)data + 1;
             } else {
               size_t decimal_places = value_digits - 2;
               format = std::format("{:>{}.{}f}", *(percentage_t *)data,
                                    value_digits, decimal_places);
               data = (percentage_t *)data + 1;
             }
             texts[this->resLabel[j].value].changeText(engine, assets, format);
           }

           glm::vec3 pos = endPath;
           std::string_view upperStr, bottomStr;
           upperLabel.show(engine);
           bottomStr = "Miss";
           switch (access.res) {
           case AccessResult::HIT:
             upperLabel.changeText(engine, assets, "Hit");
             pos.x -=
                 (float)upperLabel.getLength() * upperLabel.getSpacing() + 5;
             pos.y -= 10;
             upperLabel.setPos(engine, pos);
             bottomLabel.hide(engine);
             break;
           case AccessResult::COMPULSORY_MISS:
             upperStr = "Compulsory";
             break;
           case AccessResult::CONFLICT_MISS:
             upperStr = "Conflict";
             break;
           case AccessResult::CAPACITY_MISS:
             upperStr = "Capacity";
             break;
           case AccessResult::UNKOWN:
             break;
           }

           if (access.res != AccessResult::HIT) {
             bottomStr = "Miss";
             bottomLabel.show(engine);
             upperLabel.changeText(engine, assets, upperStr, "2d_camera");
             bottomLabel.changeText(engine, assets, bottomStr, "2d_camera");

             pos.y += 20;
             pos.x -=
                 (float)upperLabel.getLength() * upperLabel.getSpacing() + 5;
             upperLabel.setPos(engine, pos);
             pos.y -= 40;
             pos.x +=
                 ((float)upperLabel.getLength() * upperLabel.getSpacing() -
                  (float)bottomLabel.getLength() * bottomLabel.getSpacing()) *
                 0.5f;
             bottomLabel.setPos(engine, pos);
           }

           cameraPos = engine.getCamera().getPos();
           cameraDest = cameraPos - glm::vec2{(float)upperLabel.getLength() *
                                                      upperLabel.getSpacing() +
                                                  40,
                                              0};

           first = false;
         }

         upperLabel.foreach ([this, progress](Engine::ID &id) {
           engine.get(id).get_component<Color>("Color")->color.a = progress;
         });
         bottomLabel.foreach ([this, progress](Engine::ID &id) {
           engine.get(id).get_component<Color>("Color")->color.a = progress;
         });

         engine.getCamera().setPosition(AnimationManager::lerp(
             cameraPos, cameraDest, progress * progress));
         if (progress == 1.0) {
           first = true;
         }
       },
       2.f},
      {[this](float progress, bool inverse) {
         Text &upperLabel = texts[std::get<0>(resultLabel)];
         Text &bottomLabel = texts[std::get<1>(resultLabel)];

         if (progress == 1.0) {
           bottomLabel.hide(engine);
           upperLabel.hide(engine);
         }
       },
       2.f},
      {[this, final](float progress, bool inverse) {
         glm::vec2 st = engine.getCamera().getPos();

         engine.getCamera().setPosition(
             AnimationManager::lerp(st, final, progress * progress));
       },
       5.f},
      {[this, set, first = true, index, tag, specs](float progress,
                                                    bool inverse) mutable {
         if (first) {
           std::string val = std::format("{:0{}b}", tag, specs.bits.tag);
           this->sets[set].setVal(engine, assets, index, true);
           this->sets[set].setTag(engine, assets, index, val);
           first = false;
         }

         if (progress == 1.0) {
           first = true;
         }
       },
       2.f},
      {[this, first = true](float progress, bool inverse) mutable {
         static glm::vec2 st;
         static glm::vec2 final(0);
         if (first) {
           st = engine.getCamera().getPos();
           first = false;
         }

         if (progress == 1.0) {
           first = true;
           for (auto &id : pathObjs) {
             Object &obj = engine.get(id);
             obj.hide();
           }

           for (auto &id : symbolObjs) {
             engine.get(id).hide();
           }

           for (auto &idx : fieldBlock) {
             Engine::ID &id = std::get<0>(idx);
             Text &label = texts[std::get<1>(idx)];

             engine.get(id).hide();
             label.foreach ([this](Engine::ID &id) { engine.get(id).hide(); });
           }

           requestNew = true;
         }

         engine.getCamera().setPosition(
             AnimationManager::lerp(st, final, progress));
       },
       5.f},
  }});
}

CacheSet::CacheSet(glm::vec3 pos, glm::vec2 charsize, CacheSpecs &specs,
                   Engine &engine, AssetManager &assets, size_t blocks) {
  this->pos = pos;
  const float yoff = ceilf(charsize.y * 1.5f);
  off.y = yoff;
  off.x = charsize.x;
  size_t ix = 0;
  const discrete_t params[3] = {1, specs.bits.tag, infoSize};
  for (size_t i = 0; i < blocks; i++) {
    float x = pos.x;

    for (discrete_t p : params) {
      if (ix < 3) {
        xoffs[ix++] = x;
      }

      Engine::ID id = engine.object(&assets.get_shader("border_camera"),
                                    &assets.get_mesh("square"));
      Object &object = engine.get(id);

      Variable *vars = new Variable();
      float width = ceilf(charsize.x * (float)p + charsize.x * 0.5f);
      // uniform vec4 u_borderColor = vec4(0.1, 0.1, 0.1, 1.0); // Dark grey
      // uniform vec4 u_fillColor   = vec4(0.0, 0.5, 0.5, 1.0); // Teal
      // uniform vec2 u_rectPixelSize;    // The size of the rect in pixels
      // (e.g., [250, 100]) uniform float u_borderPixelWidth = 10.0; // The
      // desired border width in pixels
      vars->set("u_borderColor", glm::vec4(255, 255, 255, 255));
      vars->set("u_fillColor", Color::hextorgba(0x333333ff));
      vars->set("u_rectPixelSize", glm::vec2(width, yoff));
      vars->set("u_borderPixelWidth", 1.f);
      vars->set("u_progress", 1.f);

      object
          .add_component(new Transform(glm::vec3(x, pos.y, pos.z),
                                       glm::vec3(width, yoff, 1), glm::vec3(0)))
          .add_component(vars);

      x += width;
    }
    xoffs[3] = x;

    pos.y -= yoff;
  }

  valBits.reserve(blocks);
  indexLabels.reserve(blocks);
  std::string s, tagDefault(specs.bits.tag, '0');
  const size_t maxDigits = (size_t)ceilf(log10f((float)blocks));
  s.resize((size_t)maxDigits + 1);
  for (size_t i = 0; i < blocks; i++) {
    glm::vec3 pos = this->getPos(i, 0);

    valBits.emplace_back(engine, assets, "0", 25, "2d_camera");
    Text &bin = valBits.back();

    pos.x += 2;
    pos.z = this->pos.z - 5;
    bin.setPos(engine, pos);

    pos.x -= (float)maxDigits * 25.f + 10.f;

    std::format_to_n(s.data(), (long)s.length(), "{:0{}}", i, maxDigits);
    indexLabels.emplace_back(
        engine, assets, std::string_view(s.data(), maxDigits), 25, "2d_camera");
    Text &index = indexLabels.back();
    index.setPos(engine, pos);

    pos = this->getPos(i, 1);
    pos.x += 2;
    pos.z = this->pos.z - 5;
    tagLabels.emplace_back(engine, assets, tagDefault, 25, "2d_camera");
    Text &tag = tagLabels.back();
    tag.setPos(engine, pos);
    tag.foreach ([&engine](Engine::ID &id) {
      Object &obj = engine.get(id);

      obj.add_component(new Color(0xffffffff));
      obj.hide();
    });
  }
}

void CacheSet::setVal(Engine &engine, AssetManager &assets, size_t line,
                      bool val) {
  this->valBits[line].changeText(engine, assets, val ? "1" : "0");
}

void CacheSet::setTag(Engine &engine, AssetManager &assets, size_t line,
                      std::string_view val) {
  this->tagLabels[line].changeText(engine, assets, val);
  this->tagLabels[line].foreach ([&engine](Engine::ID &id) {
    Object &obj = engine.get(id);
    obj.show();
  });
}

glm::vec3 CacheSet::lookAt(size_t index, size_t col) {
  float bottom = pos.y - off.y * (float)index;
  float y = bottom + (off.y) / 2;

  return glm::vec3(xoffs[col] + (xoffs[col + 1] - xoffs[col]) / 2.f, y, 0);
}

glm::vec3 CacheSet::lookAt(size_t index) {
  float y = valBits[index].getPos().y;

  return glm::vec3(xoffs.front() + (xoffs.back() - xoffs.front()) / 2.f, y, 0);
}

glm::vec3 CacheSet::getPos(size_t index, size_t col) {
  float bottom = pos.y - off.y * (float)index;
  float y = bottom + (off.y - off.x) / 2;

  return {xoffs[col] + 5, y, 0.0};
}

glm::vec2 CacheSet::getOff() { return off; }
float CacheSet::getXOff(size_t i) { return xoffs[i]; }
size_t CacheSet::getInfoSize() { return infoSize; }
