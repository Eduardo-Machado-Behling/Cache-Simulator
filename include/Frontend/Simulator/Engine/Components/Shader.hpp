#ifndef SHADER_HPP
#define SHADER_HPP

#include "Frontend/Simulator/Engine/Components/Component.hpp"
#include <cstdint>
#include <functional>
#include <string_view>

struct Shader : public Component {
  Shader(std::string_view name, std::string_view vertex_src,
         std::string_view frag_src);
  ~Shader();

  auto bind(Engine *engine) const -> void override;
  auto unbind() const -> void override;
  auto getID() const -> uint32_t;

  template <typename T>
  auto set(std::string_view name, const T &data) const -> bool;

  template <typename T>
  auto set_unsafe(std::string_view name, const T &data) const -> bool;

  bool operator==(const Shader &other) noexcept;

  friend struct std::hash<Shader>;
  friend bool operator==(const Shader &lhs, const Shader &rhs) noexcept;

private:
  uint32_t ID;
  std::string name;
};

bool operator==(const Shader &lhs, const Shader &rhs) noexcept;

namespace std {
template <> struct hash<Shader> {
  std::size_t operator()(const Shader &obj) const {
    std::size_t h1 = std::hash<uint32_t>()(obj.ID);
    return h1;
  }
};
} // namespace std

#endif // SHADER_HPP
