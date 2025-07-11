#include "Frontend/Simulator/Engine/Font.hpp"
#include <cstring>
#include <glm/ext/quaternion_geometric.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <glm/ext/vector_float2.hpp>
#include <glm/gtx/hash.hpp> // Include the hash header
#include <glm/gtx/io.hpp>
#include <iostream>
#include <queue>
#include <span>
#include <string_view>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

struct GlyphMap {
  uint32_t index;
  uint32_t unicode;
};

struct Edge {
  std::tuple<size_t, size_t> verts;
  std::vector<glm::vec2> *vertices;

  Edge(size_t v1, size_t v2) : verts(v1, v2) {}

  template <size_t i>
  constexpr auto get(std::vector<Font::GlyphData::Point> &vertices) const
      -> glm::vec2 & {
    return vertices[std::get<i>(verts)].coords;
  }

  template <size_t i> constexpr auto get() const -> size_t {
    return std::get<i>(verts);
  }

  friend auto operator==(const Edge &a, const Edge &b) -> bool;
  friend std::hash<Edge>;
};

static auto orientation(glm::vec2 a, glm::vec2 b, glm::vec2 c) -> float;

struct Triangle {
  size_t verts[3];
  Edge edges[3];
  std::vector<Font::GlyphData::Point> *vertices;

  struct Circum {
    glm::vec2 center;
    double radius_sq;
  } circum;

  Triangle(size_t a, size_t b, size_t c,
           std::vector<Font::GlyphData::Point> &verts)
      : verts{a, b, c}, edges{{a, b}, {b, c}, {c, a}}, vertices(&verts) {
    glm::vec2 &p0 = verts[a].coords;
    glm::vec2 &p1 = verts[b].coords;
    glm::vec2 &p2 = verts[c].coords;

    const float ax = p1.x - p0.x;
    const float ay = p1.y - p0.y;
    const float bx = p2.x - p0.x;
    const float by = p2.y - p0.y;

    const float m = p1.x * p1.x - p0.x * p0.x + p1.y * p1.y - p0.y * p0.y;
    const float u = p2.x * p2.x - p0.x * p0.x + p2.y * p2.y - p0.y * p0.y;
    const float s = 1.f / (2.f * (ax * by - ay * bx));

    circum.center.x = ((p2.y - p0.y) * m + (p0.y - p1.y) * u) * s;
    circum.center.y = ((p0.x - p2.x) * m + (p1.x - p0.x) * u) * s;

    float D = 2.f * (p0.x * (p1.y - p2.y) + p1.x * (p2.y - p0.y) +
                     p2.x * (p0.y - p1.y));
    if (std::abs(D) < 1e-10)
      return; // Avoid division by zero for degenerate triangles

    glm::vec2 radius_vec = p0 - this->circum.center;
    this->circum.radius_sq =
        radius_vec.x * radius_vec.x + radius_vec.y * radius_vec.y;
  }

  auto isIn(glm::vec2 &point) -> bool {
    glm::vec2 &v1 = this->vertices[0][this->verts[0]].coords;
    glm::vec2 &v2 = this->vertices[0][this->verts[1]].coords;
    glm::vec2 &v3 = this->vertices[0][this->verts[2]].coords;

    if (orientation(v1, v2, v3) < 0) {
      std::swap(v2, v3);
    }

    float o1 = orientation(v1, v2, point);
    float o2 = orientation(v2, v3, point);
    float o3 = orientation(v3, v1, point);

    return (o1 >= 0 && o2 >= 0 && o3 >= 0);
  }

  constexpr auto operator[](size_t i) const -> glm::vec2 & {
    switch (i) {
    case 0:
      return vertices[0][verts[0]].coords;
    case 1:
      return vertices[0][verts[1]].coords;
    case 2:
      return vertices[0][verts[2]].coords;

    default:
      throw std::out_of_range(
          "Index out of range for this tuple-based object.");
    }
  }
};

static auto cdt(std::vector<std::vector<Font::GlyphData::Point>> &contours,
                std::vector<Font::Vertex> &trigs) -> void;

static auto contour_constrain(
    std::vector<std::vector<Font::GlyphData::Point>> &contours,
    std::unordered_map<std::tuple<size_t, size_t>, size_t> &mapping,
    std::vector<Font::GlyphData::Point> &vertices,
    std::vector<Triangle> &triangles, std::vector<Font::Vertex> &trigs) -> void;

static auto
carving_holes(std::vector<std::vector<Font::GlyphData::Point>> &contours,
              std::vector<Font::Vertex> &trigs) -> void;
static auto get_vec2(std::vector<std::vector<Font::GlyphData::Point>> &contours,
                     const std::tuple<size_t, size_t> &index) -> glm::vec2 &;
static auto createSuperTriangule(std::vector<Font::GlyphData::Point> &vertices,
                                 std::vector<Triangle> &trigs) -> size_t;
static auto
triangulate(std::vector<std::vector<Font::GlyphData::Point>> &contours,
            std::unordered_map<std::tuple<size_t, size_t>, size_t> &mapping,
            std::vector<Font::GlyphData::Point> &vertices,
            std::vector<Triangle> &triangles, std::vector<Font::Vertex> &trigs)
    -> void;
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

template <class T> inline void hash_combine(std::size_t &seed, const T &v) {
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std {
template <typename... Ts> struct hash<std::tuple<Ts...>> {
  size_t operator()(const std::tuple<Ts...> &t) const {
    size_t seed = 0;
    apply_hash(seed, t);
    return seed;
  }

private:
  template <size_t I = 0, typename... Args>
  inline typename std::enable_if<I == sizeof...(Args), void>::type
  apply_hash(size_t &, const std::tuple<Args...> &) const {}

  template <size_t I = 0, typename... Args>
      inline typename std::enable_if <
      I<sizeof...(Args), void>::type
      apply_hash(size_t &seed, const std::tuple<Args...> &t) const {
    hash_combine(seed, std::get<I>(t));
    apply_hash<I + 1>(seed, t);
  }
};

template <> struct hash<Edge> {
  auto operator()(const Edge &edge) const noexcept -> size_t {
    auto v1 = std::get<0>(edge.verts);
    auto v2 = std::get<1>(edge.verts);
    if (v1 > v2)
      std::swap(v1, v2);

    // Now, hash the sorted (canonical) tuple
    return std::hash<std::tuple<size_t, size_t>>{}(std::make_tuple(v1, v2));
  }
};
} // namespace std

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

Font::Vertex::Vertex(glm::vec2 &p0, glm::vec2 &p1, glm::vec2 &p2) {
  this->vec.named.begin = glm::vec3(p0.x, p0.y, 0);
  this->vec.named.control = glm::vec3(p1.x, p1.y, 0);
  this->vec.named.end = glm::vec3(p2.x, p2.y, 0);
}
auto Font::Vertex::operator()() -> void {
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, size(),
                        (void *)offsetof(Font::Vertex::__U::__S, begin));

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, size(),
                        (void *)offsetof(Font::Vertex::__U::__S, control));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, size(),
                        (void *)offsetof(Font::Vertex::__U::__S, end));
}

auto Font::Vertex::size() -> int { return sizeof(this->vec); }

auto Font::Vertex::equal(const MeshVertex *other) const -> bool {
  const Font::Vertex *vert = reinterpret_cast<const Font::Vertex *>(other);
  return vec.named.begin == vert->vec.named.begin &&
         vec.named.control == vert->vec.named.control &&
         vec.named.end == vert->vec.named.end;
}

auto Font::Vertex::hash() const -> size_t {
  size_t seed = 0;
  // Iterate through the union as an array
  for (const auto &vec : vec.arr) {
    hash_combine(seed, vec);
  }

  return seed;
}
auto Font::Vertex::setData(void *data) -> void * {
  std::memcpy(data, &vec, sizeof(vec));
  return static_cast<uint8_t *>(data) + sizeof(vec);
}

static auto ReadGlyph(std::ifstream &ffile, uint32_t loc, uint32_t index)
    -> Font::GlyphData {
  ffile.seekg(loc);
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
  std::vector<std::vector<Font::GlyphData::Point>> contours_vec;
  contours.reserve(static_cast<uint16_t>(countour_amount));
  contours_vec.reserve(static_cast<uint16_t>(countour_amount));

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
    contours_vec.emplace_back();
    auto &con = contours_vec.back();
    con.reserve(end - start + 1);

    for (uint16_t i = start; i < end; i++) {
      uint16_t ni = (i + 1) % end;
      bool onCurve[2] = {bit<0>(flags[i]), bit<0>(flags[ni])};

      con.emplace_back(glm::vec2(vertex_data[i]), onCurve[0]);
      if (onCurve[0] == onCurve[1])
        con.emplace_back(glm::vec2(vertex_data[i] + vertex_data[ni]) / 2.f,
                         onCurve[0]);
    }
    con.emplace_back(glm::vec2(vertex_data[start]), bit<0>(flags[start]));

    start = end;
  }

  std::cout << "Trigulation of " << (char)g.UnicodeValue << '\n';
  cdt(contours_vec, g.triagulated);

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
    // throw new Exception("Font cmap format not supported (TODO): " +
    // format);
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
        break; // not sure about this (hack to avoid out of bounds on a
               // specific font)

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
    ffile.ignore(10); // Skip: reserved, subtableByteLengthInlcudingHeader,
                      // languageCode
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
      os << "\t\t" << vec.coords.x << ", " << vec.coords.y << " | "
         << vec.onCurve << '\n';
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
    ffile.read(reinterpret_cast<char *>(&table),
               sizeof table); // binary input
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
    // If 2-byte format is used, the stored location is half of actual
    // location (so multiply by 2)
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

auto operator==(const Edge &a, const Edge &b) -> bool {
  return (std::get<0>(a.verts) == std::get<0>(b.verts) &&
          std::get<1>(a.verts) == std::get<1>(b.verts)) ||
         (std::get<0>(a.verts) == std::get<1>(b.verts) &&
          std::get<1>(a.verts) == std::get<0>(b.verts));
}

static auto orientation(glm::vec2 a, glm::vec2 b, glm::vec2 c) -> float {
  return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

// static auto inCircumcircle(glm::vec2 &d, glm::vec2 &a, glm::vec2 &b,
//                            glm::vec2 &c) -> bool {
//   // This determinant-based test is robust but its sign depends on the
//   // winding order of vertices a, b, c. If (a,b,c) is clockwise, the
//   // sign of the result is flipped.
//   // To fix this, we ensure the vertices are passed to the calculation
//   // in a consistent counter-clockwise (CCW) order.
//
//   if (orientation(a, b, c) < 0)
//     std::swap(b, c);
//
//   glm::vec2 ad = a - d;
//   glm::vec2 bd = b - d;
//   glm::vec2 cd = c - d;
//
//   float ab_det = ad.x * bd.y - bd.x * ad.y;
//   float bc_det = bd.x * cd.y - cd.x * bd.y;
//   float ca_det = cd.x * ad.y - ad.x * cd.y;
//
//   float a_sq = ad.x * ad.x + ad.y * ad.y;
//   float b_sq = bd.x * bd.x + bd.y * bd.y;
//   float c_sq = cd.x * cd.x + cd.y * cd.y;
//
//   return (a_sq * bc_det + b_sq * ca_det + c_sq * ab_det) > 0;
// }

static auto get_vec2(std::vector<std::vector<Font::GlyphData::Point>> &contours,
                     const std::tuple<size_t, size_t> &index) -> glm::vec2 & {
  return contours[std::get<0>(index)][std::get<1>(index)].coords;
}

// return the index of the triangle in trigs
static auto createSuperTriangule(std::vector<Font::GlyphData::Point> &vertices,
                                 std::vector<Triangle> &trigs) -> size_t {
  glm::vec2 min = vertices[0].coords;
  glm::vec2 max = min;

  const size_t num_glyph_verts = vertices.size();

  for (size_t i = 1; i < vertices.size(); ++i) {
    glm::vec2 &p = vertices[i].coords;
    min.x = std::min(min.x, p.x);
    max.x = std::max(max.x, p.x);

    min.y = std::min(min.y, p.y);
    max.y = std::max(max.y, p.y);
  }

  glm::vec2 d = max - min;
  float dmax = std::max(d.x, d.y);
  glm::vec2 mid = (min + max) * 0.5f;

  glm::vec2 p1(mid.x - 20 * dmax, mid.y - dmax);
  glm::vec2 p2(mid.x, mid.y + 20 * dmax);
  glm::vec2 p3(mid.x + 20 * dmax, mid.y - dmax);

  vertices.emplace_back(p1, true);
  vertices.emplace_back(p2, true);
  vertices.emplace_back(p3, true);

  if (orientation(p1, p2, p3) < 0) {
    std::swap(vertices[1], vertices[2]);
  }

  // constructor: v1 index, v2 index, v3 index, vertices vector&
  trigs.emplace_back(num_glyph_verts, num_glyph_verts + 1, num_glyph_verts + 2,
                     vertices);
  return 0;
}

static auto calculate_centroid(std::vector<glm::vec2> &contour) -> glm::vec2 {
  float area = 0.0;
  glm::vec2 c(0.f, 0.f);

  for (size_t i = 0; i < contour.size(); i++) {
    glm::vec2 &p1 = contour[i];
    glm::vec2 &p2 = contour[(i + 1) % contour.size()];

    float cross = p1.x * p2.y - p2.x * p1.y;
    area += cross;

    c += (p1 + p2) * cross;
  }

  if (std::abs(area) < 1e-10) {
    glm::vec2 sum(0.f, 0.f);
    for (auto &p : contour) {
      sum += p;
    }

    return sum / (float)contour.size();
  }

  float total_area = area * 0.5f;

  return c / (6.f * total_area);
}

static auto cdt(std::vector<std::vector<Font::GlyphData::Point>> &contours,
                std::vector<Font::Vertex> &trigs) -> void {
  std::unordered_map<std::tuple<size_t, size_t>, size_t> mapping;
  std::vector<Triangle> triangles;
  std::vector<Font::GlyphData::Point> vertices;

  triangulate(contours, mapping,  vertices, triangles, trigs);
  contour_constrain(contours, mapping,  vertices, triangles, trigs);
  carving_holes(contours, trigs);

  std::cout << "Trigulation: \n";
  for (auto const &trig : triangles) {
    auto &p0 = vertices[trig.verts[0]];
    auto &p1 = vertices[trig.verts[1]];
    auto &p2 = vertices[trig.verts[2]];

    trigs.emplace_back(trig[0], trig[1], trig[2]);
    std::cout << "\ttrig: " << trig[0] << p0.onCurve << ", " << trig[1]
              << p1.onCurve << ", " << trig[2] << p2.onCurve << "\n";
  }
  std::cout << std::endl;
}

static auto
triangulate(std::vector<std::vector<Font::GlyphData::Point>> &contours,
            std::unordered_map<std::tuple<size_t, size_t>, size_t> &mapping,
            std::vector<Font::GlyphData::Point> &vertices,
			std::vector<Triangle> &triangles,
            std::vector<Font::Vertex> &trigs) -> void {
  constexpr double eps = 1e-4;

  std::vector<std::tuple<size_t, size_t>> points;

  std::cout << "points: \n";
  for (size_t i = 0; i < contours.size(); i++) {
    auto &contour = contours[i];
	std::cout << "\tContour:" << '\n';
    for (size_t j = 0; j < contour.size(); j++) {
      points.emplace_back(std::make_tuple(i, j));
      mapping.emplace(std::make_tuple(i, j), vertices.size());
      vertices.emplace_back(contour[j]);
      std::cout << "\t\t" << contour[j].coords << ", " << contour[j].onCurve
                << '\n';
    }
  }
  std::cout << std::endl;

  createSuperTriangule(vertices, triangles);

  std::vector<Edge> edges;
  std::vector<Triangle> good_trigs;
  for (auto const &pidx : points) {
    glm::vec2 &pt = get_vec2(contours, pidx);
    edges.clear();
    good_trigs.clear();
    // for (auto const &tri : triangles) {
    for (size_t tri_id = 0; tri_id < triangles.size(); tri_id++) {
      const Triangle &tri = triangles[tri_id];
      /* Check if the point is inside the triangle circumcircle. */
      const auto dist =
          (tri.circum.center.x - pt.x) * (tri.circum.center.x - pt.x) +
          (tri.circum.center.y - pt.y) * (tri.circum.center.y - pt.y);
      if ((dist - tri.circum.radius_sq) <= eps) {
        edges.push_back(tri.edges[0]);
        edges.push_back(tri.edges[1]);
        edges.push_back(tri.edges[2]);
      } else {
        good_trigs.push_back(tri);
      }
    }

    std::unordered_map<Edge, size_t> edge_count;
    for (auto &edge : edges) {
      edge_count[edge]++;
    }

    edges.clear();
    for (auto &[edge, count] : edge_count) {
      if (count == 1)
        edges.push_back(edge);
    }

    for (auto &edge : edges) {
      if (std::abs(orientation(pt, edge.get<0>(vertices),
                               edge.get<1>(vertices))) > 1e-10)
        good_trigs.emplace_back(mapping.at(pidx), edge.get<0>(), edge.get<1>(),
                                vertices);
    }

    triangles = good_trigs;
  }

  const size_t n_vertices = vertices.size();
  const size_t v0_super = n_vertices - 3;
  const size_t v1_super = n_vertices - 2;
  const size_t v2_super = n_vertices - 1;

  // remove super
  std::erase_if(triangles, [&](const Triangle &t) {
    return (t.verts[0] == v0_super || t.verts[0] == v1_super ||
            t.verts[0] == v2_super || t.verts[1] == v0_super ||
            t.verts[1] == v1_super || t.verts[1] == v2_super ||
            t.verts[2] == v0_super || t.verts[2] == v1_super ||
            t.verts[2] == v2_super);
  });

}

// Helper to check if point q lies on segment pr
static auto on_segment(glm::vec2 p, glm::vec2 q, glm::vec2 r) -> bool {
  return (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
          q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y));
}

// Main function to check if line segment p1q1 intersects p2q2
static auto intersects(glm::vec2 p1, glm::vec2 q1, glm::vec2 p2, glm::vec2 q2)
    -> bool {
  float o1 = orientation(p1, q1, p2);
  float o2 = orientation(p1, q1, q2);
  float o3 = orientation(p2, q2, p1);
  float o4 = orientation(p2, q2, q1);

  // General case
  if (o1 != 0 && o2 != 0 && o3 != 0 && o4 != 0) {
    if ((o1 > 0) != (o2 > 0) && (o3 > 0) != (o4 > 0)) {
      return true;
    }
  }

  // Special Cases for collinear points are complex and a common source of
  // bugs. For simplicity, this example will focus on the general case. A
  // production-ready implementation would need to handle collinear
  // intersections robustly.

  return false;
}

// ================================================================
// PHASE 2: ENFORCE CONTOUR CONSTRAINTS
// ================================================================
static auto contour_constrain(
    std::vector<std::vector<Font::GlyphData::Point>> &contours,
    std::unordered_map<std::tuple<size_t, size_t>, size_t> &mapping,
    std::vector<Font::GlyphData::Point> &vertices,
	std::vector<Triangle> &triangles,
    std::vector<Font::Vertex> &trigs) -> void {
  std::cout << "\n--- Starting Phase 2: Enforcing Constraints ---\n";

  // 2a. Gather all contour edges that must exist.
  std::unordered_set<Edge> constraint_edges;
  for (size_t i = 0; i < contours.size(); ++i) {
    auto &contour = contours[i];
    // We only constrain "on-curve" segments for sharp edges
    for (size_t j = 0; j < contour.size(); ++j) {
      if (contour[j].onCurve && contour[(j + 1) % contour.size()].onCurve) {
        size_t p1_idx = mapping.at({i, j});
        size_t p2_idx = mapping.at({i, (j + 1) % contour.size()});
        constraint_edges.emplace(p1_idx, p2_idx);
      }
    }
  }
  std::cout << "Found " << constraint_edges.size() << " constraint edges.\n";

  // 2b. Build a map of all edges in the current triangulation for quick
  // checking.
  std::unordered_set<Edge> current_edges;
  for (const auto &tri : triangles) {
    current_edges.insert(tri.edges[0]);
    current_edges.insert(tri.edges[1]);
    current_edges.insert(tri.edges[2]);
  }

  // 2c. The Edge Recovery Loop
  for (const auto &constraint : constraint_edges) {
    std::cout << "\nChecking constraint edge (" << constraint.get<0>() << ", "
              << constraint.get<1>() << ")...\n";

    if (current_edges.count(constraint)) {
      std::cout << "  OK: Edge already exists in triangulation.\n";
      continue;
    }

    std::cout << "  Action: Edge must be recovered.\n";

    // --- Edge Flipping (Simplified Logic) ---
    // A full edge-flipping implementation is very complex. This logic
    // demonstrates a simpler "remove and re-triangulate" approach for the
    // intersected region.

    glm::vec2 p1 = vertices[constraint.get<0>()].coords;
    glm::vec2 p2 = vertices[constraint.get<1>()].coords;

    std::vector<size_t> bad_triangle_indices;
    std::vector<Edge> new_boundary_edges;

    // Find all triangles that this constraint edge intersects
    for (size_t i = 0; i < triangles.size(); ++i) {
      const auto &tri = triangles[i];
      glm::vec2 t_p0 = vertices[tri.verts[0]].coords;
      glm::vec2 t_p1 = vertices[tri.verts[1]].coords;
      glm::vec2 t_p2 = vertices[tri.verts[2]].coords;

      // A simple check: does the constraint intersect any edge of the
      // triangle?
      if (intersects(p1, p2, t_p0, t_p1) || intersects(p1, p2, t_p1, t_p2) ||
          intersects(p1, p2, t_p2, t_p0)) {
        std::cout << "    - Found intersecting triangle " << i << " ("
                  << tri.verts[0] << "," << tri.verts[1] << "," << tri.verts[2]
                  << ")\n";
        bad_triangle_indices.push_back(i);
        new_boundary_edges.push_back(tri.edges[0]);
        new_boundary_edges.push_back(tri.edges[1]);
        new_boundary_edges.push_back(tri.edges[2]);
      }
    }

    if (bad_triangle_indices.empty()) {
      std::cout << "  Warning: Could not find any triangles intersecting the "
                   "constraint.\n";
      continue;
    }

    // Remove the bad triangles
    std::sort(bad_triangle_indices.rbegin(), bad_triangle_indices.rend());
    for (size_t index : bad_triangle_indices) {
      triangles.erase(triangles.begin() + (long)index);
    }

    // Find the boundary of the new hole
    std::unordered_map<Edge, int> edge_counts;
    for (const auto &edge : new_boundary_edges) {
      edge_counts[edge]++;
    }

    std::vector<Edge> hole_boundary;
    for (const auto &[edge, count] : edge_counts) {
      if (count == 1) {
        hole_boundary.push_back(edge);
      }
    }

    // Re-triangulate the hole (this is a simplification). A robust
    // solution would use a dedicated polygon triangulation algorithm here.
    // For now, we just add the constraint edge back in, which is not a
    // complete triangulation but enforces the edge. A better approach would
    // be to recursively call a triangulation function on the vertices of the
    // `hole_boundary`.

    std::cout << "    - Removed " << bad_triangle_indices.size()
              << " triangles, leaving a hole with " << hole_boundary.size()
              << " edges.\n";
    // NOTE: This part is a placeholder for a real polygon triangulation
    // algorithm. This simplification will leave gaps in your mesh.
  }

  // After this loop, many constraints might be in the mesh, but there will be
  // holes. A full implementation requires a robust polygon triangulation step
  // inside the loop.

  // The rest of your `triangulate` function...
  // const size_t n_vertices = ...
}

static auto
carving_holes(std::vector<std::vector<Font::GlyphData::Point>> &contours,
              std::vector<Font::Vertex> &trigs) -> void {}

// std::unordered_map<Edge, std::vector<size_t>> edge_to_trigid;
// for (size_t i = 0; i < triangles.size(); i++) {
//   Edge edge1(std::get<0>(triangles[i].verts),
//              std::get<1>(triangles[i].verts));
//   Edge edge2(std::get<1>(triangles[i].verts),
//              std::get<2>(triangles[i].verts));
//   Edge edge3(std::get<2>(triangles[i].verts),
//              std::get<0>(triangles[i].verts));
//   edge_to_trigid[edge1].push_back(i);
//   edge_to_trigid[edge2].push_back(i);
//   edge_to_trigid[edge3].push_back(i);
// }
//
// std::unordered_set<size_t> trigid_remove;
// std::unordered_set<Edge> hole_edges;
//
// // Iterate through hole contours (all but the first one)
// for (size_t i = 1; i < contours.size(); i++) {
//   auto &hole = contours[i];
//
//   for (size_t j = 0; j < hole.size(); j++) {
//     std::tuple id(i, j);
//     std::tuple nid(i, (j + 1) % hole.size());
//     hole_edges.emplace(points[id], points[nid]);
//   }
//
//   glm::vec2 seed_point = calculate_centroid(hole);
//   size_t start = triangles.size() - 1;
//
//   std::queue<size_t> q;
//   std::unordered_set<size_t> visited;
//   std::unordered_set<size_t> triangles_to_remove;
//
//   q.push(start);
//   visited.insert(start);
//
//   // 2. Loop while there are triangles to visit
//   while (!q.empty()) {
//     // Get the triangle at the front of the queue
//     size_t current_tri_idx = q.front();
//     q.pop();
//
//     triangles_to_remove.insert(current_tri_idx);
//     const Triangle &current_tri = triangles[current_tri_idx];
//
//     // 3. Create edges for the current triangle
//     Edge tri_edges[3] = {
//         Edge(std::get<0>(current_tri.verts),
//         std::get<1>(current_tri.verts)),
//         Edge(std::get<1>(current_tri.verts),
//         std::get<2>(current_tri.verts)),
//         Edge(std::get<2>(current_tri.verts),
//         std::get<0>(current_tri.verts))};
//
//     // 4. Explore neighbors through each edge
//     for (const auto &edge : tri_edges) {
//       // If the edge is part of a "hole," don't cross it
//       if (hole_edges.contains(edge)) {
//         continue;
//       }
//
//       // Find triangles that share this edge
//       // The .at() will throw if the edge isn't in the map; use .find() for
//       // safety
//       if (edge_to_trigid.count(edge)) {
//         for (size_t neighbor_idx : edge_to_trigid.at(edge)) {
//           // If the neighbor is new, add it to the queue and mark as
//           visited if (neighbor_idx != current_tri_idx &&
//               !visited.count(neighbor_idx)) {
//             visited.insert(neighbor_idx);
//             q.push(neighbor_idx);
//           }
//         }
//       }
//     }
//   }
// }
//
// for (size_t index : trigid_remove) {
//   triangles.erase(triangles.begin() + (long)index);
// }
//
// for (auto &tri : triangles) {
//   trigs.emplace_back();
//   Font::Vertex &vert = trigs.back();
//
//   glm::vec2 &p1 = tri[0];
//   glm::vec2 &p2 = tri[1];
//   glm::vec2 &p3 = tri[2];
//
//   vert.vec.named.begin = glm::vec3(p1.x, p1.y, 0);
//   vert.vec.named.control = glm::vec3(p2.x, p2.y, 0);
//   vert.vec.named.end = glm::vec3(p3.x, p3.y, 0);
// }
// } // namespace std
