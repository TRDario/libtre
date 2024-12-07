#include "../include/tre/bitmap_text_renderer.hpp"
#include "../include/tre/renderer_2d.hpp"
#include "../include/tre/sampler.hpp"
#include <numeric>

using namespace tr::matrix_operators;

namespace tre {
	BitmapTextRenderer* _bitmapTextRenderer{nullptr};

	// Measures the longest string that fits within a certain width.
	std::string_view measureUnformatted(std::string_view text, const BitmapTextRenderer::GlyphMap& font, float scale,
										float maxWidth) noexcept;

	// Measures the longest string that fits within a certain width.
	std::string_view measureFormatted(std::string_view text, const BitmapTextRenderer::GlyphMap& font, float scale,
									  float maxWidth) noexcept;

	// Splits a text string into lines.
	std::vector<std::string_view> splitText(std::string_view text, const BitmapTextRenderer::GlyphMap& font,
											float scale, float maxWidth, bool formatted);

	// Gets the initial offset for text in a textbox.
	float initialOffsetY(const std::vector<std::string_view>& lines, float lineSkip,
						 const BitmapTextRenderer::Textbox& textbox) noexcept;

	// Gets the initial offset for a line of text.
	float initialUnformattedOffsetX(std::string_view line, const BitmapTextRenderer::GlyphMap& font, float scale,
									const BitmapTextRenderer::Textbox& textbox) noexcept;

	// Gets the initial offset for a line of text.
	float initialFormattedOffsetX(std::string_view line, const BitmapTextRenderer::GlyphMap& font, float scale,
								  const BitmapTextRenderer::Textbox& textbox) noexcept;
} // namespace tre

std::string_view tre::measureUnformatted(std::string_view text, const BitmapTextRenderer::GlyphMap& font, float scale,
										 float maxWidth) noexcept
{
	float lineWidth{};
	for (auto it = tr::utf8Begin(text); it != tr::utf8End(text); ++it) {
		if (*it == '\n') {
			return {text.begin(), (const char*)(it)};
		}

		lineWidth += font.at(*it).advance * scale;
		if (it != text.begin() && lineWidth > maxWidth) {
			return {text.begin(), (const char*)(it)};
		}
	}
	return text;
}

std::string_view tre::measureFormatted(std::string_view text, const BitmapTextRenderer::GlyphMap& font, float scale,
									   float maxWidth) noexcept
{
	float lineWidth{};
	for (auto it = tr::utf8Begin(text); it != tr::utf8End(text); ++it) {
		if (*it == '\n') {
			return {text.begin(), (const char*)(it)};
		}
		else if (*it == '\\') {
			if (++it != text.end() && *it == '\\') {
				lineWidth += font.at('\\').advance * scale;
			}
		}
		else {
			lineWidth += font.at(*it).advance * scale;
			if (it != text.begin() && lineWidth > maxWidth) {
				return {text.begin(), (const char*)(it)};
			}
		}
	}
	return text;
}

std::vector<std::string_view> tre::splitText(std::string_view text, const BitmapTextRenderer::GlyphMap& font,
											 float scale, float maxWidth, bool formatted)
{
	std::vector<std::string_view> lines;

	for (auto it = text.begin(); it != text.end();) {
		auto fit{formatted ? measureFormatted(text, font, scale, maxWidth)
						   : measureUnformatted(text, font, scale, maxWidth)};
		if (fit.end() == text.end()) {
			lines.push_back(fit);
			return lines;
		}
		else {
			fit = {fit.begin(), std::next(fit.end())};
			auto end{std::ranges::find_first_of(fit | std::views::reverse, " \t\n")};
			if (end != fit.rend()) {
				lines.push_back({it, end.base()});
				it = std::next(end.base());
			}
			else {
				lines.push_back(fit);
				it = fit.end();
			}
		}
	}
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
	__assume(false);
#else // GCC, Clang
	__builtin_unreachable();
#endif
}

float tre::initialOffsetY(const std::vector<std::string_view>& lines, float lineSkip,
						  const BitmapTextRenderer::Textbox& textbox) noexcept
{
	float textboxTop{textbox.pos.y - textbox.posAnchor.y};
	switch (VerticalAlign(textbox.textAlignment)) {
	case VerticalAlign::TOP:
		return textboxTop;
	case VerticalAlign::CENTER:
		return textboxTop + (textbox.size.y - (lines.size() * lineSkip)) / 2.0f;
	case VerticalAlign::BOTTOM:
		return textboxTop + textbox.size.y - (lines.size() * lineSkip);
	}
}

float tre::initialUnformattedOffsetX(std::string_view line, const BitmapTextRenderer::GlyphMap& font, float scale,
									 const BitmapTextRenderer::Textbox& textbox) noexcept
{
	const float textboxLeft{textbox.pos.x - textbox.posAnchor.x};
	const auto  pred{[&](float sum, std::uint32_t chr) { return sum + font.at(chr).advance * scale; }};

	switch (HorizontalAlign(textbox.textAlignment)) {
	case HorizontalAlign::LEFT:
		return textboxLeft;
	case HorizontalAlign::CENTER:
		return (textboxLeft + std::accumulate(tr::utf8Begin(line), tr::utf8End(line), 0.0f, pred)) / 2.0f;
	case HorizontalAlign::RIGHT:
		return textboxLeft + std::accumulate(tr::utf8Begin(line), tr::utf8End(line), 0.0f, pred);
	}
}

float tre::initialFormattedOffsetX(std::string_view line, const BitmapTextRenderer::GlyphMap& font, float scale,
								   const BitmapTextRenderer::Textbox& textbox) noexcept
{
	const float textboxLeft{textbox.pos.x - textbox.posAnchor.x};
	const auto  lineWidth{[](std::string_view line, const BitmapTextRenderer::GlyphMap& font, float scale) {
        float width{};
        for (auto it = line.begin(); it != line.end(); ++it) {
            if (*it == '\\') {
                if (++it != line.end() && *it == '\\') {
                    width += font.at('\\').advance * scale;
                }
            }
            else {
                width += font.at(*it).advance;
            }
        }
        return width;
    }};

	switch (HorizontalAlign(textbox.textAlignment)) {
	case HorizontalAlign::LEFT:
		return textboxLeft;
	case HorizontalAlign::CENTER:
		return (textboxLeft + lineWidth(line, font, scale)) / 2.0f;
	case HorizontalAlign::RIGHT:
		return textboxLeft + lineWidth(line, font, scale);
	}
}

tre::BitmapFontLoadError::BitmapFontLoadError(std::string path, const char* message) noexcept
	: FileError{std::move(path)}, _message{message}
{
}

const char* tre::BitmapFontLoadError::what() const noexcept
{
	static std::string str;
	str = std::format("Failed to bitmap font ({}): {}", _message, path());
	return str.c_str();
}

tre::BitmapTextRenderer::BitmapTextRenderer() noexcept
{
	assert(!bitmapTextRendererActive());
	_bitmapTextRenderer = this;

#ifndef NDEBUG
	_atlas.setLabel("(tre) Bitmap Text Renderer Atlas");
#endif
}

tre::BitmapTextRenderer::~BitmapTextRenderer() noexcept
{
	_bitmapTextRenderer = nullptr;
}

void tre::BitmapTextRenderer::addFont(std::string name, tr::SubBitmap texture, std::int32_t lineSkip, GlyphMap glyphs)
{
	if (!_fonts.contains(name)) {
		_fonts.emplace(std::move(name), Font{lineSkip, std::move(glyphs)});
		_atlas.add(name, texture);
	}
}

void tre::BitmapTextRenderer::loadFont(std::string name, const std::filesystem::path& path)
{
	auto file{tr::openFileR(path, std::ios::binary)};

	auto magic{tr::readBinary<std::array<char, 4>>(file)};
	if (std::string_view{magic.data(), 4} != "TREF") {
		throw BitmapFontLoadError{path.string(), "invalid font file"};
	}

	auto     lineSkip{tr::readBinary<std::int32_t>(file)};
	auto     nglyphs{tr::readBinary<std::uint32_t>(file)};
	GlyphMap glyphs;
	for (std::uint32_t i = 0; i < nglyphs; ++i) {
		glyphs.emplace(tr::readBinary<std::uint32_t>(file), tr::readBinary<Glyph>(file));
	}
	auto       imageData{tr::flushBinary<std::vector<char>>(file)};
	tr::Bitmap image{tr::rangeBytes(imageData)};
	addFont(name, image, lineSkip, std::move(glyphs));
}

void tre::BitmapTextRenderer::removeFont(std::string_view name)
{
	auto it{_fonts.find(name)};
	if (it != _fonts.end()) {
		_atlas.remove(name);
		_fonts.erase(it);
	}
}

void tre::BitmapTextRenderer::clearFonts()
{
	_atlas.clear();
	_fonts.clear();
}

void tre::BitmapTextRenderer::addGlyph(int priority, std::uint32_t codepoint, const Font& font, tr::RectF2 fontUV,
									   Style style, glm::vec2 scale, tr::RGBA8 tint, glm::vec2 pos, glm::vec2 posAnchor,
									   tr::AngleF rotation)
{
	const auto&      glyph{font.glyphs.at(codepoint)};
	const auto       size{glm::vec2(glyph.size) * scale};
	const tr::RectF2 uv{fontUV.tl + glm::vec2(glyph.tl) / glm::vec2(_atlas.texture().size()),
						glm::vec2(glyph.size) / glm::vec2(_atlas.texture().size())};
	const auto       offset{glm::vec2(glyph.offset) * scale};

	if (style == Style::ITALIC) {
		constexpr double TAN_12_5_DEG{0.22169466264};
		const auto       skewOffset{size.y * TAN_12_5_DEG};

		std::vector<Renderer2D::Vertex> data;
		const auto                      transform{tr::rotateAroundPoint2(glm::mat4{1}, pos, rotation)};
		tr::fillRectVertices((data | tr::positions).begin(), pos - posAnchor + offset, size);
		tr::fillRectVertices((data | tr::uvs).begin(), uv.tl, uv.size);
		for (int i = 0; i < 4; ++i) {
			data[i].pos   = transform * data[i].pos;
			data[i].color = tint;
		}
		data[0].pos.x += skewOffset;
		data[3].pos.x += skewOffset;
		renderer2D().addTexturedPolygonFan(priority, std::move(data), {_atlas.texture(), bilinearSampler()});
	}
	else {
		renderer2D().addTexturedRotatedRectangle(priority, pos, posAnchor, size, rotation,
												 {_atlas.texture(), bilinearSampler()}, uv, tint);
	}
}

void tre::BitmapTextRenderer::addGlyph(int priority, std::uint32_t codepoint, std::string_view font, Style style,
									   glm::vec2 scale, tr::RGBA8 tint, glm::vec2 pos, glm::vec2 posAnchor,
									   tr::AngleF rotation)
{
	auto it{_fonts.find(font)};
	if (it != _fonts.end()) {
		addGlyph(priority, codepoint, it->second, _atlas[font], style, scale, tint, pos, posAnchor, rotation);
	}
}

void tre::BitmapTextRenderer::addUnformatted(int priority, std::string_view text, std::string_view font, Style style,
											 glm::vec2 scale, tr::RGBA8 tint, const Textbox& textbox)
{
	auto fontIt{_fonts.find(font)};
	if (fontIt == _fonts.end()) {
		return;
	}
	auto fontUV{_atlas[font]};

	auto lines{splitText(text, fontIt->second.glyphs, scale.x, textbox.size.x, false)};
	auto yOffset{initialOffsetY(lines, fontIt->second.lineSkip, textbox)};
	for (auto& line : lines) {
		auto xOffset{initialUnformattedOffsetX(line, fontIt->second.glyphs, scale.x, textbox)};
		for (auto chr : tr::utf8Range(line)) {
			auto& glyph{fontIt->second.glyphs.at(chr)};
			if (glyph.size != glm::u16vec2{0, 0}) {
				addGlyph(priority, chr, fontIt->second, fontUV, style, scale, tint, textbox.pos,
						 textbox.pos - glm::vec2(xOffset, yOffset), tr::degs(0));
			}
			xOffset += glyph.advance;
		}

		yOffset += fontIt->second.lineSkip;
	}
}

void tre::BitmapTextRenderer::addFormatted(int priority, std::string_view text, std::string_view font, glm::vec2 scale,
										   std::span<tr::RGBA8> colors, const Textbox& textbox)
{
	assert(!colors.empty());

	auto fontIt{_fonts.find(font)};
	if (fontIt == _fonts.end()) {
		return;
	}
	auto fontUV{_atlas[font]};

	auto      lines{splitText(text, fontIt->second.glyphs, scale.x, textbox.size.x, true)};
	auto      yOffset{initialOffsetY(lines, fontIt->second.lineSkip, textbox)};
	Style     style{Style::NORMAL};
	tr::RGBA8 tint{255, 255, 255, 255};
	for (auto& line : lines) {
		auto xOffset{initialUnformattedOffsetX(line, fontIt->second.glyphs, scale.x, textbox)};
		for (auto it = line.begin(); it != line.end(); ++it) {
			if (*it == '\\') {
				if (++it == line.end()) {
					goto line_end;
				}
				switch (*it) {
				case '\\':
					addGlyph(priority, '\\', fontIt->second, fontUV, style, scale, tint, textbox.pos,
							 textbox.pos - glm::vec2(xOffset, yOffset), tr::degs(0));
					xOffset += fontIt->second.glyphs.at('\\').advance;
					break;
				case '!':
					tint = {255, 255, 255, 255};
					break;
				case 'c':
					if (++it == text.end()) {
						goto line_end;
					}
					if (std::isdigit(*it) && *it - '0' < colors.size()) {
						tint = colors[*it - '0'];
					}
					break;
				case 'i':
					style = style == Style::NORMAL ? Style::ITALIC : Style::NORMAL;
					break;
				default:
					break;
				}
			}
			else {
				auto& glyph{fontIt->second.glyphs.at(*it)};
				if (glyph.size != glm::u16vec2{0, 0}) {
					addGlyph(priority, *it, fontIt->second, fontUV, style, scale, tint, textbox.pos,
							 textbox.pos - glm::vec2(xOffset, yOffset), tr::degs(0));
				}
				xOffset += glyph.advance;
			}
		}
	line_end:
		yOffset += fontIt->second.lineSkip;
	}
}

bool tre::bitmapTextRendererActive() noexcept
{
	return _bitmapTextRenderer != nullptr;
}

tre::BitmapTextRenderer& tre::bitmapTextRenderer() noexcept
{
	assert(bitmapTextRendererActive());
	return *_bitmapTextRenderer;
}