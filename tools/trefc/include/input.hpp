#pragma once
#include "common.hpp"
#include <unordered_map>

struct Glyph {
	std::uint16_t x, y;
	std::uint16_t width, height;
	std::int16_t xOffset, yOffset;
	std::int16_t advance;
};

struct FontInfo {
	std::int32_t lineSkip;
	std::unordered_map<std::uint32_t, Glyph> glyphs;
};

Expected<FontInfo, ErrorCode> loadFontInfo(std::string_view path);