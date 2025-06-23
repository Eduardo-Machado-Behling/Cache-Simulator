#include "Frontend/Simulator/Engine/Font.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>
#include <span>

static auto flip(uint8_t* data, size_t bytes) -> void;

template <typename T>
static auto flip(T* data) -> void {
	flip(reinterpret_cast<uint8_t*>(data), sizeof(*data));
}

template <bool BigEndian = true, typename T>
static auto file_read(std::ifstream& ffile, T& data) -> void {
	ffile.read(reinterpret_cast<char*>(&data), sizeof(data));

	if constexpr (sizeof(data) > 1 && BigEndian){
		flip(&data);
	}
}

template <uint8_t i>
constexpr auto mask() -> uint8_t {
    static_assert(i < 8, "Invalid bit index");
    return static_cast<uint8_t>(0x1 << i);
}

template <uint8_t i>
static auto bit(uint8_t data) -> bool{
	constexpr uint8_t __mask = mask<i>();
	return data & __mask;
}

template <bool readingX>
static auto readCoords(std::ifstream& ffile, std::span<uint8_t> flags, std::vector<int16_t>& coords){
	constexpr uint8_t offset_size_flag = readingX ? 1 : 2;
	constexpr uint8_t offset_sign_flag = readingX ? 4 : 5;
	coords.reserve(flags.size());

	for (size_t i = 0; i < flags.size(); i++) {
		int16_t last_coord = coords.empty() ? 0 : coords.back();
		uint8_t flag = flags[i]; // Pegue o flag por valor, não referência.
		// bool onCurve = bit<0>(flag); // Você pode usar isso depois.

		// Caso 1: Vetor curto (offset de 1 byte)
		if (bit<offset_size_flag>(flag)) {
			uint8_t offset;
			// Use nossa função auxiliar para ler e já inverter o endianness se necessário
			file_read(ffile, offset); 
			int16_t sign = bit<offset_sign_flag>(flag) ? 1 : -1;
			coords.push_back(static_cast<int16_t>(offset) * sign + last_coord);
		} 
		// Caso 3: Mesma que a anterior
		// Se o bit de "mesma que a anterior" está setado...
		else if (bit<offset_sign_flag>(flag)) {
			// Apenas adicione a coordenada anterior novamente.
			coords.push_back(last_coord);
		}
		// Caso 2: Vetor longo (offset de 2 bytes)
		// Só chegamos aqui se não for nem vetor curto, nem "mesma que a anterior".
		else {
			int16_t offset;
			// A função file_read já cuida do flip de endianness.
			file_read(ffile, offset);
			coords.push_back(offset + last_coord);
		}
	}
}

static auto readGlyph(std::ifstream& ffile, uint32_t loc) -> Font::GlyphData {
	ffile.seekg(loc);
	std::cout << "tell: " << ffile.tellg() << " | loc = " << loc << '\n';
	Font::GlyphData g;
	int16_t countour_amount;
	file_read(ffile, countour_amount);
	
	if(countour_amount <= 0){
		return g;
	}

	g.contour_end_points.reserve(static_cast<uint16_t>(countour_amount));
	ffile.ignore(8); //bbox
					
	uint16_t countour = 0;
	for (uint16_t i = 0; i < countour_amount; i++) {
		file_read(ffile, countour);
		g.contour_end_points.push_back(countour);
	}

	uint16_t instructions;
	file_read(ffile, instructions);
	ffile.ignore(instructions);

	size_t flags_amount = g.contour_end_points.back() + 1;
	std::vector<uint8_t> flags(flags_amount);
	for (size_t i = 0; i < flags_amount; i++) {
		uint8_t flag;
		file_read(ffile, flag);
		flags[i] = flag;

		if(bit<3>(flag)){
			uint8_t next;
			file_read(ffile, next);
			for (int r = 0; r < next; r++) {
				flags[++i] = flag;
			}
		}
	}

	readCoords<true>(ffile, flags, g.x);
	readCoords<false>(ffile, flags, g.y);

	return g;
}

std::ostream& operator<<(std::ostream& os, const Font::Table& p){
	os << "Table: [" << "tag=\"" << std::string_view(p.tag, 4) 
	   << "\", checksum=" << p.checksum << ", offset=" << p.offset
	   << ", length=" << p.length << "]";

	return os;
}
std::ostream &operator<<(std::ostream &os, const Font::GlyphData &p){
	os << "Glyph:\n";
	for (auto& countour: p.contour_end_points) {
		os << "\tContour: " << countour << '\n';
	}

	for (size_t i = 0; i < p.x.size(); i++) {
		os << "\t" << i << ": (" << p.x[i] << ", " << p.y[i] << ")\n";
	}

	return os;
}

Font::Font(std::filesystem::path file){
	std::ifstream ffile(file.string(), std::ios::binary);

	ffile.ignore(4);
	uint16_t numTables;
	ffile.read(reinterpret_cast<char*>(&numTables), sizeof numTables); // binary input
	flip(&numTables);
	ffile.ignore(6);

	Table table;
	for (uint16_t i = 0; i < numTables; i++) {
		ffile.read(reinterpret_cast<char*>(&table), sizeof table); // binary input
		flip(&table.checksum);
		flip(&table.offset);
		flip(&table.length);

		std::string key( table.tag, 4);
		tables.insert({key, table});
		std::cout << "Table: " << table << " | key=\"" << key << "\", table: " << tables[key] << '\n';
	}

	std::cout << readGlyph(ffile, tables["glyf"].offset) << std::endl;
}




static auto flip(uint8_t* data, size_t bytes) -> void {
	for (size_t i = 0, j = bytes - 1; i < j; i++, j--) {
		data[i] = data[j] ^ data[i];
		data[j] = data[i] ^ data[j];
		data[i] = data[j] ^ data[i];
	}
}
