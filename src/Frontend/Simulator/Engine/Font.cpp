#include "Frontend/Simulator/Engine/Font.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <span>
#include <string_view>
#include <vector>

struct GlyphMap {
  uint32_t index;
  uint32_t unicode;
};

static auto flip(uint8_t *data, size_t bytes) -> void;
static auto populateGlyphs(std::ifstream &ffile, uint32_t loc)
    -> std::vector<GlyphMap>;

static auto GetAllGlyphLocations(std::ifstream &reader, uint32_t num_glyphs,
                                 uint32_t bytes_per_entry,
                                 uint32_t loca_table_location,
                                 uint32_t glyf_table_location)
    -> std::vector<uint32_t>;

static auto ReadGlyph(std::ifstream &ffile, uint32_t loc, uint32_t index)
    -> Font::GlyphData;
static auto ReadAllGlyphs(std::ifstream &reader,
                          std::vector<uint32_t> &glyph_locations,
                          std::vector<GlyphMap> &mappings,
                          std::vector<Font::GlyphData> &glyphs) -> void;

template <typename T> static auto flip(T *data) -> void {
  flip(reinterpret_cast<uint8_t *>(data), sizeof(*data));
}

template <bool BigEndian = true, typename T>
static auto file_read(std::ifstream &ffile, T &data) -> void {
  ffile.read(reinterpret_cast<char *>(&data), sizeof(data));

  if constexpr (sizeof(data) > 1 && BigEndian) {
    flip(&data);
  }
}

template <uint8_t i> constexpr auto mask() -> uint8_t {
  static_assert(i < 8, "Invalid bit index");
  return static_cast<uint8_t>(0x1 << i);
}

template <uint8_t i> static auto bit(uint8_t data) -> bool {
  constexpr uint8_t __mask = mask<i>();
  return data & __mask;
}

template <bool readingX>
static auto readCoords(std::ifstream &ffile, std::span<uint8_t> flags,
                       std::vector<int16_t> &coords) {
  constexpr uint8_t offset_size_flag = readingX ? 1 : 2;
  constexpr uint8_t offset_sign_flag = readingX ? 4 : 5;
  coords.reserve(flags.size());

  int lastOnCurve = 2;
  for (size_t i = 0; i < flags.size(); i++) {
    int16_t last_coord = coords.empty() ? 0 : coords.back();
    uint8_t flag = flags[i];     // Pegue o flag por valor, não referência.
    bool onCurve = bit<0>(flag); // Você pode usar isso depois.
    int16_t res = last_coord;

    // Caso 1: Vetor curto (offset de 1 byte)
    if (bit<offset_size_flag>(flag)) {
      uint8_t offset;
      // Use nossa função auxiliar para ler e já inverter o endianness se
      // necessário
      file_read(ffile, offset);
      int16_t sign = bit<offset_sign_flag>(flag) ? 1 : -1;
      res = static_cast<int16_t>(offset * sign + last_coord);
    }
    // Caso 3: Mesma que a anterior
    // Se o bit de "mesma que a anterior" está setado...
    else if (!bit<offset_sign_flag>(flag)) {
      int16_t offset;
      // A função file_read já cuida do flip de endianness.
      file_read(ffile, offset);
      res = offset + last_coord;
    }

    if (onCurve == lastOnCurve) {
      coords.push_back((last_coord + res) >> 1);
    }

    coords.push_back(res);

    lastOnCurve = onCurve;
  }
}

static auto ReadGlyph(std::ifstream &ffile, uint32_t loc, uint32_t index)
    -> Font::GlyphData {
  ffile.seekg(loc);
  std::cout << "tell: " << ffile.tellg() << " | loc = " << loc << '\n';
  Font::GlyphData g;
  g.GlyphIndex = index;
  int16_t countour_amount;
  file_read(ffile, countour_amount);

  if (countour_amount <= 0) {
    return g;
  }

  file_read(ffile, g.MinX);
  file_read(ffile, g.MinY);
  file_read(ffile, g.MaxX);
  file_read(ffile, g.MaxY);
  g.Width = g.MaxX - g.MinX;
  g.Height = g.MaxY - g.MinY;

  std::vector<uint16_t> contours;
  contours.reserve(static_cast<uint16_t>(countour_amount));
  g.contours.reserve(static_cast<uint16_t>(countour_amount));

  uint16_t countour = 0;
  for (uint16_t i = 0; i < countour_amount; i++) {
    file_read(ffile, countour);
    contours.push_back(countour);
  }

  uint16_t instructions;
  file_read(ffile, instructions);
  ffile.ignore(instructions);

  size_t flags_amount = contours.back() + 1;
  std::vector<uint8_t> flags(flags_amount);
  for (size_t i = 0; i < flags_amount; i++) {
    uint8_t flag;
    file_read(ffile, flag);
    flags[i] = flag;

    if (bit<3>(flag)) {
      uint8_t next;
      file_read(ffile, next);
      for (int r = 0; r < next; r++) {
        flags[++i] = flag;
      }
    }
  }

  std::vector<int16_t> x, y;
  x.reserve(flags_amount);
  y.reserve(flags_amount);

  readCoords<true>(ffile, flags, x);
  readCoords<false>(ffile, flags, y);

  std::vector<glm::vec<2, int16_t>> vertex_data;
  vertex_data.reserve(x.size());
  for (size_t i = 0; i < x.size(); i++) {
    vertex_data.emplace_back(x[i], y[i]);
  }

  uint16_t start = 0;
  for (auto end : contours) {
    g.contours.emplace_back();
    std::vector<glm::vec2> &con = g.contours.back();
    con.reserve(end - start + 1);

    for (uint16_t i = start; i < end; i++) {
      con.emplace_back(vertex_data[i]);
      uint16_t ni = (i + 1) % end;
      if (bit<0>(flags[i]) == bit<0>(flags[ni]))
        con.push_back(glm::vec2(vertex_data[i] + vertex_data[ni]) / 2.f);
    }
    con.emplace_back(vertex_data[start]);

    start = end;
  }

  return g;
}

struct Ranges {
  uint16_t offset;
  uint32_t read_loc;
};

static auto populateGlyphs(std::ifstream &ffile, uint32_t loc)
    -> std::vector<GlyphMap> {
  std::vector<GlyphMap> glyph_map;
  ffile.seekg(loc);

  uint16_t version;
  uint16_t num_sub_tables;

  file_read(ffile, version);
  file_read(ffile, num_sub_tables);

  uint32_t cmap_subtable_offset = 0;
  int16_t selected_unicode_version_id = -1;

  for (int i = 0; i < num_sub_tables; i++) {
    int16_t platform_id;
    int16_t platform_specific_id;
    uint32_t offset;

    file_read(ffile, platform_id);
    file_read(ffile, platform_specific_id);
    file_read(ffile, offset);

    // Unicode encoding
    if (platform_id == 0) {
      switch (platform_specific_id) {
      case 0:
      case 1:
      case 3:
      case 4:
        if (platform_specific_id > selected_unicode_version_id) {
          cmap_subtable_offset = offset;
          selected_unicode_version_id = platform_specific_id;
        }
        break;

      default:
        if (platform_id == 3 && selected_unicode_version_id == -1) {
          if (platform_specific_id == 1 || platform_specific_id == 10) {
            cmap_subtable_offset = offset;
          }
        }
        break;
      }
    }
  }

  if (cmap_subtable_offset == 0) {
    // TODO: exception
    //
    // throw new Exception("Font does not contain supported character map type
    // (TODO)");
    std::abort();
  }

  ffile.seekg(loc + cmap_subtable_offset);
  uint16_t format;
  bool has_read_missing_char_glyph = false;

  file_read(ffile, format);

  if (format != 12 && format != 4) {
    // TODO:
    // throw new Exception("Font cmap format not supported (TODO): " + format);
    //
    std::abort();
  }

  // ---- Parse Format 4 ----
  if (format == 4) {
    uint16_t length;
    uint16_t language_code;
    // Number of contiguous segments of character codes
    uint16_t seg_count_2X;

    file_read(ffile, length);
    file_read(ffile, language_code);
    file_read(ffile, seg_count_2X);

    uint16_t seg_count = seg_count_2X / 2;
    ffile.ignore(6); // Skip: searchRange, entrySelector, rangeShift

    // Ending character code for each segment (last = 2^16 - 1)
    std::vector<uint16_t> end_codes;
    end_codes.reserve(seg_count);
    for (int i = 0; i < seg_count; i++) {
      uint16_t end_code;
      file_read(ffile, end_code);
      end_codes.push_back(end_code);
    }

    ffile.ignore(2); // Reserved pad

    std::vector<uint16_t> start_codes;
    start_codes.reserve(seg_count);

    for (int i = 0; i < seg_count; i++) {
      uint16_t start_code;
      file_read(ffile, start_code);
      start_codes.push_back(start_code);
    }

    std::vector<uint16_t> id_deltas;
    id_deltas.reserve(seg_count);
    for (int i = 0; i < seg_count; i++) {
      uint16_t id_delta;
      file_read(ffile, id_delta);
      id_deltas.push_back(id_delta);
    }

    std::vector<Ranges> id_range_offsets;
    id_range_offsets.reserve(seg_count);

    for (int i = 0; i < seg_count; i++) {
      uint16_t offset;
      std::ifstream::pos_type read_loc = ffile.tellg();

      file_read(ffile, offset);
      id_range_offsets.emplace_back(offset, read_loc);
    }

    for (size_t i = 0; i < start_codes.size(); i++) {
      uint16_t end_code = end_codes[i];
      uint16_t curr_code = start_codes[i];

      if (curr_code == 65535)
        break; // not sure about this (hack to avoid out of bounds on a specific
               // font)

      while (curr_code <= end_code) {
        uint16_t glyph_index;
        // If idRangeOffset is 0, the glyph index can be calculated directly
        if (id_range_offsets[i].offset == 0) {
          glyph_index = (curr_code + id_deltas[i]) % 65536;
        }
        // Otherwise, glyph index needs to be looked up from an array
        else {
          std::ifstream::pos_type reader_location_old = ffile.tellg();
          uint32_t range_offset_location =
              id_range_offsets[i].read_loc + id_range_offsets[i].offset;
          uint32_t glyph_index_array_location =
              2 * (curr_code - start_codes[i]) + range_offset_location;

          ffile.seekg(glyph_index_array_location);

          file_read(ffile, glyph_index);

          if (glyph_index != 0) {
            glyph_index = (glyph_index + id_deltas[i]) % 65536;
          }

          ffile.seekg(reader_location_old);
        }

        glyph_map.emplace_back(glyph_index, curr_code);
        has_read_missing_char_glyph |= glyph_index == 0;
        curr_code++;
      }
    }
  }
  // ---- Parse Format 12 ----
  else if (format == 12) {
    ffile.ignore(
        10); // Skip: reserved, subtableByteLengthInlcudingHeader, languageCode
    uint32_t num_groups;
    file_read(ffile, num_groups);

    for (uint32_t i = 0; i < num_groups; i++) {
      uint32_t start_char_code;
      uint32_t end_char_code;
      uint32_t start_glyph_index;

      file_read(ffile, start_char_code);
      file_read(ffile, end_char_code);
      file_read(ffile, start_glyph_index);

      uint32_t num_chars = end_char_code - start_char_code + 1;
      for (uint32_t charCodeOffset = 0; charCodeOffset < num_chars;
           charCodeOffset++) {
        uint32_t char_code = start_char_code + charCodeOffset;
        uint32_t glyph_index = start_glyph_index + charCodeOffset;

        glyph_map.emplace_back(glyph_index, char_code);
        has_read_missing_char_glyph |= glyph_index == 0;
      }
    }
  }

  if (!has_read_missing_char_glyph) {
    glyph_map.emplace_back(0u, 65535u);
  }

  return glyph_map;
}

std::ostream &operator<<(std::ostream &os, const Font::Table &p) {
  os << "Table: [" << "tag=\"" << std::string_view(p.tag, 4)
     << "\", checksum=" << p.checksum << ", offset=" << p.offset
     << ", length=" << p.length << "]";

  return os;
}

std::ostream &operator<<(std::ostream &os, const Font::GlyphData &p) {
  os << "Glyph:\n";
  for (auto &countour : p.contours) {
    os << "\tContour:\n";
    for (auto &vec : countour) {
      os << "\t\t" << vec.x << ", " << vec.y << '\n';
    }
  }

  return os;
}

Font::Font(std::filesystem::path file) {
  std::ifstream ffile(file.string(), std::ios::binary);

  ffile.ignore(4);
  uint16_t numTables;
  ffile.read(reinterpret_cast<char *>(&numTables),
             sizeof numTables); // binary input
  flip(&numTables);
  ffile.ignore(6);

  Table table;
  for (uint16_t i = 0; i < numTables; i++) {
    ffile.read(reinterpret_cast<char *>(&table), sizeof table); // binary input
    flip(&table.checksum);
    flip(&table.offset);
    flip(&table.length);

    std::string key(table.tag, 4);
    tables.insert({key, table});
    std::cout << "Table: " << table << " | key=\"" << key
              << "\", table: " << tables[key] << '\n';
  }

  Table &glyph_table_location = tables["glyf"];
  Table &loca_table_location = tables["loca"];
  Table &cmap_location = tables["cmap"];

  // ---- Read Head Table ----
  ffile.seekg(tables["head"].offset);
  ffile.ignore(18);
  // Design units to Em size (range from 64 to 16384)
  uint16_t unitsPerEm;
  file_read(ffile, unitsPerEm);
  ffile.ignore(30);
  // Number of bytes used by the offsets in the 'loca' table (for looking up
  // glyph locations)
  int16_t flag;
  file_read(ffile, flag);
  uint32_t bytes_per_entry = (flag == 0 ? 2 : 4);

  // --- Read 'maxp' table ---
  ffile.seekg(tables["maxp"].offset);
  ffile.ignore(4);
  uint16_t num_glyphs;
  file_read(ffile, num_glyphs);

  auto glyph_map = populateGlyphs(ffile, tables["cmap"].offset);
  auto glyph_locations = GetAllGlyphLocations(
      ffile, num_glyphs, bytes_per_entry, loca_table_location.offset,
      glyph_table_location.offset);

  ReadAllGlyphs(ffile, glyph_locations, glyph_map, glyph_data);

  struct Layout {
    uint32_t advance;
    int left;
  };

  std::vector<Layout> layout_data;
  layout_data.resize(num_glyphs);

  // Get number of metrics from the 'hhea' table
  ffile.seekg(tables["hhea"].offset);
  ffile.ignore(8); // unused: version, ascent, descent

  int16_t lineGap;
  int16_t advanceWidthMax;

  file_read(ffile, lineGap);
  file_read(ffile, advanceWidthMax);

  ffile.ignore(22); // unused: minLeftSideBearing, minRightSideBearing,
                    // xMaxExtent, caretSlope/Offset, reserved, metricDataFormat

  int16_t numAdvanceWidthMetrics;
  file_read(ffile, numAdvanceWidthMetrics);

  // Get the advance width and leftsideBearing metrics from the 'hmtx' table
  ffile.seekg(tables["hmtx"].offset);
  uint32_t lastAdvanceWidth = 0;

  for (int i = 0; i < numAdvanceWidthMetrics; i++) {
    int16_t advanceWidth;
    int16_t leftSideBearing;

    file_read(ffile, advanceWidth);
    file_read(ffile, leftSideBearing);

    lastAdvanceWidth = static_cast<uint32_t>(advanceWidth);

    layout_data.emplace_back(advanceWidth, leftSideBearing);
  }

  // Some fonts have a run of monospace characters at the end
  int numRem = num_glyphs - numAdvanceWidthMetrics;

  for (int i = 0; i < numRem; i++) {
    int16_t leftSideBearing;
    file_read(ffile, leftSideBearing);

    layout_data.emplace_back(lastAdvanceWidth, leftSideBearing);
  }

  // Apply
  for (auto &c : glyph_data) {
    c.AdvanceWidth = layout_data[c.GlyphIndex].advance;
    c.LeftSideBearing = layout_data[c.GlyphIndex].left;

    glyph_lookup.insert({c.UnicodeValue, c});
  }
}

auto Font::operator[](uint32_t i) -> const Font::GlyphData & {
  if (glyph_lookup.contains(i)) {
    return glyph_lookup.at(i);
  } else {
    return glyph_lookup.at(0);
  }
}

static auto GetAllGlyphLocations(std::ifstream &reader, uint32_t num_glyphs,
                                 uint32_t bytes_per_entry,
                                 uint32_t loca_table_location,
                                 uint32_t glyf_table_location)
    -> std::vector<uint32_t> {
  std::vector<uint32_t> all_glyph_locations;
  all_glyph_locations.reserve(num_glyphs);
  bool is_two_byte_entry = bytes_per_entry == 2;

  for (uint32_t glyph_index = 0; glyph_index < num_glyphs; glyph_index++) {
    reader.seekg(loca_table_location + glyph_index * bytes_per_entry);
    // If 2-byte format is used, the stored location is half of actual location
    // (so multiply by 2)
    uint32_t glyph_data_offset;

    if (is_two_byte_entry) {
      uint16_t u16;
      file_read(reader, u16);
      glyph_data_offset = u16 * 2u;
    } else {
      file_read(reader, glyph_data_offset);
    }
    all_glyph_locations.push_back(glyf_table_location + glyph_data_offset);
  }

  return all_glyph_locations;
}

static auto ReadAllGlyphs(std::ifstream &reader,
                          std::vector<uint32_t> &glyph_locations,
                          std::vector<GlyphMap> &mappings,
                          std::vector<Font::GlyphData> &glyphs) -> void {
  glyphs.reserve(mappings.size());

  for (uint32_t i = 0; i < mappings.size(); i++) {
    auto mapping = mappings[i];

    Font::GlyphData glyphData =
        ReadGlyph(reader, glyph_locations[mapping.index], mapping.index);
    glyphData.UnicodeValue = mapping.unicode;
    glyphs.push_back(glyphData);
  }
}

static auto flip(uint8_t *data, size_t bytes) -> void {
  for (size_t i = 0, j = bytes - 1; i < j; i++, j--) {
    data[i] = data[j] ^ data[i];
    data[j] = data[i] ^ data[j];
    data[i] = data[j] ^ data[i];
  }
}
