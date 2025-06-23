#ifndef FONT_HPP
#define FONT_HPP

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
    std::vector<int16_t> x;
    std::vector<int16_t> y;
	std::vector<uint16_t> contour_end_points;
  };

  Font(std::filesystem::path file);

private:
  std::unordered_map<std::string, Table> tables;
};

std::ostream &operator<<(std::ostream &os, const Font::Table &p);
std::ostream &operator<<(std::ostream &os, const Font::GlyphData &p);

#endif
