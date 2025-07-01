#ifndef FONT_HPP
#define FONT_HPP

#include <glm/glm.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

struct Font {
  struct Table {
    char tag[4];
    uint32_t checksum;
    uint32_t offset;
    uint32_t length;
  };

  struct GlyphData {
    std::vector<std::vector<glm::vec2>> contours;
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

  auto operator[](uint32_t i) -> const GlyphData&;

private:
  std::unordered_map<std::string, Table> tables;
  std::vector<GlyphData> glyph_data;
  std::unordered_map<uint32_t, GlyphData&> glyph_lookup;
};

std::ostream &operator<<(std::ostream &os, const Font::Table &p);
std::ostream &operator<<(std::ostream &os, const Font::GlyphData &p);

#endif
