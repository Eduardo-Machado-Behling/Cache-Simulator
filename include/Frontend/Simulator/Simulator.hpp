#ifndef HELLOWORLD_HPP
#define HELLOWORLD_HPP

#include "Backend/Backend.hpp"
#include "Frontend/Frontend.hpp"
#include "Frontend/Simulator/Engine/AnimationManager.hpp"
#include "Frontend/Simulator/Engine/AssetManager.hpp"
#include "Frontend/Simulator/Engine/Engine.hpp"
#include "common/CacheAccess.hpp"
#include "common/CacheReport.hpp"
#include "common/CacheSpecs.hpp"

#include <array>
#include <chrono>
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <string_view>

struct Text {
  Text(Engine &engine, AssetManager &assets, std::string_view string,
       float spacing, std::string_view shader = "2d");
  Text(std::span<Engine::ID> span, std::string_view str, float spacing);

  void populate(Engine &engine, AssetManager &assets, std::string &string,
                std::string_view shader);
  void foreach (std::function<void(Engine::ID &)> callback);
  void foreach (std::function<void(Engine::ID &, size_t)> callback);

  std::span<Engine::ID> getSpan(size_t begin, size_t end);
  Text getSubText(size_t begin, size_t amount);

  void changeText(Engine &engine, AssetManager &assets,
                  std::string_view newText, std::string_view shader = "2d");
  float getSpacing();

  void setPos(Engine &engine, glm::vec3 pos);

  glm::vec3 getPos();
  size_t getLength();
  std::string_view getString();

private:
  std::vector<Engine::ID> objs;
  glm::vec3 position;
  std::string string;
  float spacing;
};

struct CacheSet {
  CacheSet(glm::vec3 pos, glm::vec2 charsize, CacheSpecs &specs, Engine &engine,
           AssetManager &assets, size_t blocks);

  glm::vec3 lookAt(size_t index, size_t col);
  glm::vec3 lookAt(size_t index);
  glm::vec3 getPos(size_t index, size_t col);
  glm::vec2 getOff();

  void setVal(Engine &engine, AssetManager &assets, size_t line, bool val);
  void setTag(Engine &engine, AssetManager &assets, size_t line,
              std::string_view val);
  float getXOff(size_t i);
  size_t getInfoSize();

private:
  std::vector<Engine::ID> objs;
  std::vector<Text> valBits;
  std::vector<Text> indexLabels;
  std::vector<Text> tagLabels;
  std::array<float, 4> xoffs;
  glm::vec3 pos;
  glm::vec2 off;
  size_t infoSize = 16;
};

class Simulator : public Frontend {
public:
  Simulator(std::unique_ptr<Backend> &backend);
  auto tick(Backend *backend, std::queue<addr_t> &addrs) -> void override;
  auto halted() -> bool override;

private:
  struct HistCycleInfo {
    glm::vec3 bottom;

    size_t begin;
    size_t amount;
    size_t end;
    size_t i = 0;
  };

  struct ReportLabelEntry {
    Text *label;
    Text *value;
    void *valuePtr;
  };

  auto decomposeAddr(std::queue<addr_t> &addrs, HistCycleInfo &info, Text &top,
                     Backend *backend, CacheAccess access) -> void;
  auto populateAddrs(std::queue<addr_t> &addrs) -> HistCycleInfo;
  auto populateBottom(std::queue<addr_t> &addrs, HistCycleInfo &populateBottom)
      -> void;
  auto showField(Text &bin, discrete_t tagSize, CacheAccess access) -> void;
  auto drawConnection(Object &box, Object &indexBox, CacheAccess access)
      -> void;

  GLFWwindow *window;
  Engine engine;
  AssetManager assets;
  AnimationManager animator;
  bool requestNew = true;
  Backend *backend;

  using floating_seconds = std::chrono::duration<float>;
  std::chrono::time_point<std::chrono::steady_clock> start;
  std::vector<Text> texts;
  std::vector<CacheSet> sets;
  std::array<std::pair<Engine::ID, size_t>, 2> fieldBlock;
  std::array<Engine::ID, 6> pathObjs;
  std::array<Engine::ID, 2> symbolObjs;
  std::vector<ReportLabelEntry> resLabel;
  const size_t value_digits = 6;
};

#endif // HELLOWORLD_HPP
