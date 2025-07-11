#ifndef FONT_HPP
#define FONT_HPP

#include "Frontend/Simulator/Engine/Components/Mesh.hpp"
#include <cstddef>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

struct Font {
  struct Table {
    char tag[4];
    uint32_t checksum;
    uint32_t offset;
    uint32_t length;
  };

  struct Vertex final : public MeshVertex {
    Vertex(glm::vec2 &p0, glm::vec2 &p1, glm::vec2 &p2);

    auto operator()() -> void override;
    auto size() -> int override;

    auto equal(const MeshVertex *other) const -> bool override;
    auto hash() const -> size_t override;
	auto setData(void* data) -> void* override;

    union __U {
      struct __S {
        glm::vec3 begin;
        glm::vec3 control;
        glm::vec3 end;
      } named;
      glm::vec3 arr[3];
    } vec;
  };

  struct GlyphData {
	struct Point {
		glm::vec2 coords;
		bool onCurve;

		Point(glm::vec2 coords, bool onCurve) : coords(coords), onCurve(onCurve) {}
	};
	
    std::vector<std::vector<Point>> contours;
    std::vector<Vertex> triagulated;
    uint32_t UnicodeValue = 1337;
    uint32_t GlyphIndex = 1337;
    uint32_t AdvanceWidth = 1337;
    int LeftSideBearing = 1337;

    int16_t MinX = 1337;
    int16_t MaxX = 1337;
    int16_t MinY = 1337;
    int16_t MaxY = 1337;

    int16_t Width = MaxX - MinX;
    int16_t Height = MaxY - MinY;
  };

  Font(std::filesystem::path file);

  auto operator[](uint32_t i) -> const GlyphData &;

private:
  std::unordered_map<std::string, Table> tables;
  std::vector<GlyphData> glyph_data;
  std::unordered_map<uint32_t, GlyphData &> glyph_lookup;
};

std::ostream &operator<<(std::ostream &os, const Font::Table &p);
std::ostream &operator<<(std::ostream &os, const Font::GlyphData &p);

#endif
