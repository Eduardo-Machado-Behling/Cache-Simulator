#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "Frontend/Simulator/Engine/Components/Component.hpp"

struct Texture : public Component {
  auto bind(Engine* engine) const -> void override;
  auto unbind() const -> void override;
};

#endif // TEXTURE_HPP