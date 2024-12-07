#include "tref.hpp"
#include <array>
#include <qoi.h>
#include <vector>

tref::OutputBitmap::OutputBitmap(std::byte* data, unsigned int width, unsigned int height) noexcept
	: _data{data}, _width{width}, _height{height}
{
}

tref::OutputBitmap::~OutputBitmap() noexcept
{
	std::free(_data);
}

std::span<const std::byte> tref::OutputBitmap::data() const noexcept
{
	return {_data, _width * _height * 4};
}

unsigned int tref::OutputBitmap::width() const noexcept
{
	return _width;
}

unsigned int tref::OutputBitmap::height() const noexcept
{
	return _height;
}

tref::DecodingResult tref::decode(std::istream& is)
{
	std::array<char, 4> magic;
	is.read(magic.data(), 4);
	if (std::string_view{magic.data(), 4} != "TREF") {
		throw DecodingError{"Invalid .tref file header."};
	}

	std::int32_t  lineSkip;
	std::uint32_t nglyphs;
	is.read((char*)(&lineSkip), sizeof(lineSkip));
	is.read((char*)(&nglyphs), sizeof(nglyphs));
	GlyphMap glyphs;
	for (std::uint32_t i = 0; i < nglyphs; ++i) {
		std::uint32_t codepoint;
		Glyph         glyph;

		is.read((char*)(&codepoint), sizeof(codepoint));
		is.read((char*)(&glyph), sizeof(glyph));
		glyphs.emplace(codepoint, glyph);
	}
	std::vector<char> qoiImage;
	std::copy(std::istreambuf_iterator<char>{is}, std::istreambuf_iterator<char>{}, std::back_inserter(qoiImage));

	qoi_desc desc;
	auto     decodedImage{qoi_decode(qoiImage.data(), qoiImage.size(), &desc, 4)};
	if (decodedImage == nullptr) {
		throw DecodingError{"Failed to decode QOI data."};
	}
	return {lineSkip, std::move(glyphs), OutputBitmap{(std::byte*)(decodedImage), desc.width, desc.height}};
}

void tref::encode(std::ostream& os, std::int32_t lineSkip, GlyphMap glyphs, InputBitmap bitmap)
{
	qoi_desc   desc{bitmap.width, bitmap.height, 4, QOI_SRGB};
	int        qoiImageSize;
	const auto qoiImage{qoi_encode(bitmap.data, &desc, &qoiImageSize)};
	if (qoiImage == nullptr) {
		throw EncodingError{"Failed to encode QOI data."};
	}

	os.write("TREF", 4);
	os.write((const char*)(&lineSkip), sizeof(lineSkip));
	auto nglyphs{std::uint32_t(glyphs.size())};
	os.write((const char*)(&nglyphs), sizeof(nglyphs));
	for (auto& [codepoint, glyph] : glyphs) {
		os.write((const char*)(&codepoint), sizeof(codepoint));
		os.write((const char*)(&glyph), sizeof(glyph));
	}
	os.write((const char*)(qoiImage), qoiImageSize);
}