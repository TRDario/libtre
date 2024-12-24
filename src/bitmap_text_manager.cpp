#include "../include/tre/bitmap_text_manager.hpp"
#include "../include/tre/renderer_2d.hpp"
#include <numeric>

using namespace tr::angle_literals;
using namespace tr::matrix_operators;

namespace tre {
	BitmapTextManager* _bitmapText{nullptr};

	// Measures the longest string that fits within a certain width.
	std::string_view measureUnformatted(std::string_view text, const BitmapTextManager::GlyphMap& font, float scale,
										float maxWidth) noexcept;

	// Measures the longest string that fits within a certain width.
	std::string_view measureFormatted(std::string_view text, const BitmapTextManager::GlyphMap& font, float scale,
									  float maxWidth) noexcept;

	// Splits a text string into lines.
	std::vector<std::string_view> splitText(std::string_view text, const BitmapTextManager::GlyphMap& font, float scale,
											float maxWidth, bool formatted);

	// Gets the initial offset for text in a textbox.
	float initialOffsetY(const std::vector<std::string_view>& lines, float lineSkip,
						 const BitmapTextManager::Textbox& textbox) noexcept;

	// Gets the initial offset for a line of text.
	float initialUnformattedOffsetX(std::string_view line, const BitmapTextManager::GlyphMap& font, float scale,
									const BitmapTextManager::Textbox& textbox) noexcept;

	// Gets the initial offset for a line of text.
	float initialFormattedOffsetX(std::string_view line, const BitmapTextManager::GlyphMap& font, float scale,
								  const BitmapTextManager::Textbox& textbox) noexcept;
} // namespace tre

std::string_view tre::measureUnformatted(std::string_view text, const BitmapTextManager::GlyphMap& font, float scale,
										 float maxWidth) noexcept
{
	float lineWidth{};
	for (auto it = tr::utf8Begin(text); it != tr::utf8End(text); ++it) {
		if (*it == '\n') {
			return {text.begin(), (const char*)(it)};
		}

		lineWidth += font.at(font.contains(*it) ? *it : '\0').advance * scale;
		if (it != text.begin() && lineWidth > maxWidth) {
			return {text.begin(), (const char*)(it)};
		}
	}
	return text;
}

std::string_view tre::measureFormatted(std::string_view text, const BitmapTextManager::GlyphMap& font, float scale,
									   float maxWidth) noexcept
{
	float lineWidth{};
	for (auto it = tr::utf8Begin(text); it != tr::utf8End(text); ++it) {
		if (*it == '\n') {
			return {text.begin(), (const char*)(it)};
		}
		else if (*it == '\\') {
			if (++it != text.end() && *it == '\\') {
				lineWidth += font.at(font.contains('\\') ? '\\' : '\0').advance * scale;
			}
		}
		else {
			lineWidth += font.at(font.contains(*it) ? *it : '\0').advance * scale;
			if (it != text.begin() && lineWidth > maxWidth) {
				return {text.begin(), (const char*)(it)};
			}
		}
	}
	return text;
}

std::vector<std::string_view> tre::splitText(std::string_view text, const BitmapTextManager::GlyphMap& font,
											 float scale, float maxWidth, bool formatted)
{
	std::vector<std::string_view> lines;

	for (auto it = text.begin(); it != text.end();) {
		auto fit{formatted ? measureFormatted({it, text.end()}, font, scale, maxWidth)
						   : measureUnformatted({it, text.end()}, font, scale, maxWidth)};
		if (fit.end() == text.end()) {
			lines.push_back(fit);
			return lines;
		}
		else {
			const auto whitespaceSearchRange{std::ranges::subrange{fit.begin(), std::next(fit.end())} |
											 std::views::reverse};
			const auto lastWhitespace{std::ranges::find_first_of(whitespaceSearchRange, " \t\n")};
			if (lastWhitespace != whitespaceSearchRange.end()) {
				lines.push_back({it, lastWhitespace.base()});
				it = std::next(lastWhitespace.base());
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
						  const BitmapTextManager::Textbox& textbox) noexcept
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

float tre::initialUnformattedOffsetX(std::string_view line, const BitmapTextManager::GlyphMap& font, float scale,
									 const BitmapTextManager::Textbox& textbox) noexcept
{
	const float textboxLeft{textbox.pos.x - textbox.posAnchor.x};
	const auto  pred{[&](float sum, std::uint32_t codepoint) {
        return sum + font.at(font.contains(codepoint) ? codepoint : '\0').advance * scale;
    }};

	switch (HorizontalAlign(textbox.textAlignment)) {
	case HorizontalAlign::LEFT:
		return textboxLeft;
	case HorizontalAlign::CENTER:
		return (textboxLeft + std::accumulate(tr::utf8Begin(line), tr::utf8End(line), 0.0f, pred)) / 2.0f;
	case HorizontalAlign::RIGHT:
		return textboxLeft + std::accumulate(tr::utf8Begin(line), tr::utf8End(line), 0.0f, pred);
	}
}

float tre::initialFormattedOffsetX(std::string_view line, const BitmapTextManager::GlyphMap& font, float scale,
								   const BitmapTextManager::Textbox& textbox) noexcept
{
	const float textboxLeft{textbox.pos.x - textbox.posAnchor.x};
	const auto  lineWidth{[](std::string_view line, const BitmapTextManager::GlyphMap& font, float scale) {
        float width{};
        for (auto it = line.begin(); it != line.end(); ++it) {
            if (*it == '\\') {
                if (++it != line.end() && *it == '\\') {
                    width += font.at(font.contains('\\') ? '\\' : '\0').advance * scale;
                }
            }
            else {
                width += font.at(font.contains(*it) ? *it : '\0').advance;
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

tre::BitmapTextManager::BitmapTextManager() noexcept
{
	assert(!bitmapTextActive());

#ifndef NDEBUG
	_atlas.setLabel("(tre) Bitmap Text Renderer Atlas");
#endif

	_bitmapText = this;
}

tre::BitmapTextManager::BitmapTextManager(BitmapTextManager&& r) noexcept
	: _atlas{std::move(r._atlas)}
	, _fonts{std::move(r._fonts)}
	, _cachedRotationTransform{std::move(r._cachedRotationTransform)}
{
	if (_bitmapText == &r) {
		_bitmapText = this;
	}
}

tre::BitmapTextManager::~BitmapTextManager() noexcept
{
	if (_bitmapText == this) {
		_bitmapText = nullptr;
	}
}

const tr::Texture2D& tre::BitmapTextManager::texture() const noexcept
{
	return _atlas.texture();
}

const tre::BitmapTextManager::Font& tre::BitmapTextManager::font(std::string_view name) const noexcept
{
	assert(_fonts.contains(name));
	return _fonts.find(name)->second;
}

void tre::BitmapTextManager::addFont(std::string name, tr::SubBitmap texture, std::int32_t lineSkip, GlyphMap glyphs)
{
	if (!_fonts.contains(name)) {
		_atlas.add(name, texture);
		_fonts.emplace(std::move(name), Font{lineSkip, std::move(glyphs)});
	}
}

void tre::BitmapTextManager::loadFont(std::string name, const std::filesystem::path& path)
{
	auto           file{tr::openFileR(path, std::ios::binary)};
	auto           decodingResult{tref::decode(file)};
	tr::BitmapView image{decodingResult.bitmap.data(),
						 {decodingResult.bitmap.width(), decodingResult.bitmap.height()},
						 tr::BitmapFormat::ARGB_8888};
	addFont(name, image, decodingResult.lineSkip, std::move(decodingResult.glyphs));
}

void tre::BitmapTextManager::removeFont(std::string_view name)
{
	auto it{_fonts.find(name)};
	if (it != _fonts.end()) {
		_atlas.remove(name);
		_fonts.erase(it);
	}
}

void tre::BitmapTextManager::clearFonts()
{
	_atlas.clear();
	_fonts.clear();
}

std::optional<tre::BitmapTextManager::GlyphMesh> tre::BitmapTextManager::createGlyphMesh(
	std::uint32_t codepoint, const Font& font, tr::RectF2 fontUV, Style style, glm::vec2 scale, tr::RGBA8 tint,
	glm::vec2 pos, glm::vec2 posAnchor, tr::AngleF rotation)
{
	const auto& glyph{font.glyphs.at(font.glyphs.contains(codepoint) ? codepoint : '\0')};
	if (glyph.width == 0 || glyph.height == 0) {
		return {};
	}

	const auto       size{glm::vec2(glyph.width, glyph.height) * scale};
	const tr::RectF2 uv{fontUV.tl + glm::vec2(glyph.x, glyph.y) / glm::vec2(_atlas.texture().size()),
						glm::vec2(glyph.width, glyph.height) / glm::vec2(_atlas.texture().size())};
	const auto       offset{glm::vec2(glyph.xOffset, glyph.yOffset) * scale};

	Renderer2D::TextureQuad quad;
	if (rotation == 0_degf) {
		tr::fillRectVertices((quad | tr::positions).begin(), pos - posAnchor + offset, size);
	}
	else {
		if (pos != _cachedRotationTransform.pos || rotation != _cachedRotationTransform.rotation) {
			_cachedRotationTransform.pos       = pos;
			_cachedRotationTransform.rotation  = rotation;
			_cachedRotationTransform.transform = tr::rotateAroundPoint2(glm::mat4{1}, pos, rotation);
		}
		tr::fillRectVertices((quad | tr::positions).begin(), pos - posAnchor + offset, size,
							 _cachedRotationTransform.transform);
	}
	if (style == Style::ITALIC) {
		constexpr double TAN_12_5_DEG{0.22169466264};
		const auto       skewOffset{size.y * TAN_12_5_DEG};
		quad[0].pos.x += skewOffset;
		quad[3].pos.x += skewOffset;
	}
	tr::fillRectVertices((quad | tr::uvs).begin(), uv.tl, uv.size);
	for (auto& vertex : quad) {
		vertex.pos   = _cachedRotationTransform.transform * vertex.pos;
		vertex.color = tint;
	}
	return quad;
}

std::optional<tre::BitmapTextManager::GlyphMesh> tre::BitmapTextManager::createGlyphMesh(
	std::uint32_t codepoint, std::string_view font, Style style, glm::vec2 scale, tr::RGBA8 tint, glm::vec2 pos,
	glm::vec2 posAnchor, tr::AngleF rotation)
{
	assert(_fonts.contains(font));
	return createGlyphMesh(codepoint, _fonts.find(font)->second, _atlas[font], style, scale, tint, pos, posAnchor,
						   rotation);
}

tre::BitmapTextManager::Mesh tre::BitmapTextManager::createUnformattedTextMesh(std::string_view text,
																			   std::string_view font, Style style,
																			   glm::vec2 scale, tr::RGBA8 tint,
																			   const Textbox& textbox)
{
	assert(_fonts.contains(font));
	const auto fontIt{_fonts.find(font)};
	const auto fontUV{_atlas[font]};

	Mesh       mesh;
	const auto lines{splitText(text, fontIt->second.glyphs, scale.x, textbox.size.x, false)};
	auto       yOffset{initialOffsetY(lines, fontIt->second.lineSkip, textbox)};
	for (auto& line : lines) {
		auto xOffset{initialUnformattedOffsetX(line, fontIt->second.glyphs, scale.x, textbox)};
		for (auto chr : tr::utf8Range(line)) {
			if (!fontIt->second.glyphs.contains(chr)) {
				chr = '\0';
			}
			const auto glyphMesh{createGlyphMesh(chr, fontIt->second, fontUV, style, scale, tint, textbox.pos,
												 textbox.pos - glm::vec2(xOffset, yOffset), textbox.rotation)};
			if (glyphMesh.has_value()) {
				tr::fillPolygonIndices(std::back_inserter(mesh.indices), 4, mesh.vertices.size());
				mesh.vertices.insert(mesh.vertices.end(), glyphMesh->begin(), glyphMesh->end());
			}
			xOffset += fontIt->second.glyphs.at(chr).advance;
		}
		yOffset += fontIt->second.lineSkip;
	}
	return mesh;
}

tre::BitmapTextManager::Mesh tre::BitmapTextManager::createFormattedTextMesh(std::string_view text,
																			 std::string_view font, glm::vec2 scale,
																			 std::span<tr::RGBA8> colors,
																			 const Textbox&       textbox)
{
	assert(_fonts.contains(font));
	const auto fontIt{_fonts.find(font)};
	const auto fontUV{_atlas[font]};

	Mesh       mesh;
	const auto lines{splitText(text, fontIt->second.glyphs, scale.x, textbox.size.x, true)};
	auto       yOffset{initialOffsetY(lines, fontIt->second.lineSkip, textbox)};
	Style      style{Style::NORMAL};
	tr::RGBA8  tint{255, 255, 255, 255};
	for (auto& line : lines) {
		auto xOffset{initialFormattedOffsetX(line, fontIt->second.glyphs, scale.x, textbox)};
		for (auto it = line.begin(); it != line.end(); ++it) {
			if (*it == '\\') {
				if (++it == line.end()) {
					goto line_end;
				}
				switch (*it) {
				case '\\': {
					const auto chr{fontIt->second.glyphs.contains('\\') ? '\\' : '\0'};
					const auto glyphMesh{createGlyphMesh(chr, fontIt->second, fontUV, style, scale, tint, textbox.pos,
														 textbox.pos - glm::vec2(xOffset, yOffset), textbox.rotation)};
					if (glyphMesh.has_value()) {
						tr::fillPolygonIndices(std::back_inserter(mesh.indices), 4, mesh.vertices.size());
						mesh.vertices.insert(mesh.vertices.end(), glyphMesh->begin(), glyphMesh->end());
					}
					xOffset += fontIt->second.glyphs.at(chr).advance;
				} break;
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
				const auto chr{fontIt->second.glyphs.contains(*it) ? *it : '\0'};
				const auto glyphMesh{createGlyphMesh(chr, fontIt->second, fontUV, style, scale, tint, textbox.pos,
													 textbox.pos - glm::vec2(xOffset, yOffset), textbox.rotation)};
				if (glyphMesh.has_value()) {
					tr::fillPolygonIndices(std::back_inserter(mesh.indices), 4, mesh.vertices.size());
					mesh.vertices.insert(mesh.vertices.end(), glyphMesh->begin(), glyphMesh->end());
				}
				xOffset += fontIt->second.glyphs.at(chr).advance;
			}
		}
	line_end:
		yOffset += fontIt->second.lineSkip;
	}
	return mesh;
}

bool tre::bitmapTextActive() noexcept
{
	return _bitmapText != nullptr;
}

tre::BitmapTextManager& tre::bitmapText() noexcept
{
	assert(bitmapTextActive());
	return *_bitmapText;
}