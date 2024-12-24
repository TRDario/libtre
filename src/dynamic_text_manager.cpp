#include "../include/tre/dynamic_text_manager.hpp"
#include "../include/tre/renderer_2d.hpp"

using namespace tr::matrix_operators;

namespace tre {
	DynamicTextManager* _dynamicText{nullptr};

	glm::vec2 calculatePosAnchor(glm::vec2 textSize, const DynamicTextManager::Textbox& textbox) noexcept;
} // namespace tre

glm::vec2 tre::calculatePosAnchor(glm::vec2 textSize, const DynamicTextManager::Textbox& textbox) noexcept
{
	switch (textbox.textAlignment) {
	case Align::TOP_LEFT:
		return textbox.posAnchor;
	case Align::TOP_CENTER:
		return {textbox.posAnchor.x - (textbox.size.x - textSize.x) / 2.0f, textbox.posAnchor.y};
	case Align::TOP_RIGHT:
		return {textbox.posAnchor.x - textbox.size.x + textSize.x, textbox.posAnchor.y};
	case Align::CENTER_LEFT:
		return {textbox.posAnchor.x, textbox.posAnchor.y - (textbox.size.y - textSize.y) / 2.0f};
	case Align::CENTER:
		return textbox.posAnchor - (textbox.size - textSize) / 2.0f;
	case Align::CENTER_RIGHT:
		return {textbox.posAnchor.x - textbox.size.x + textSize.x,
				textbox.posAnchor.y - (textbox.size.y - textSize.y) / 2.0f};
	case Align::BOTTOM_LEFT:
		return {textbox.posAnchor.x, textbox.posAnchor.y - textbox.size.y + textSize.y};
	case Align::BOTTOM_CENTER:
		return {textbox.posAnchor.x - (textbox.size.x - textSize.x) / 2.0f,
				textbox.posAnchor.y - textbox.size.y + textSize.y};
	case Align::BOTTOM_RIGHT:
		return textbox.posAnchor - textbox.size + textSize;
	}
}

tre::DynamicTextManager::DynamicTextManager() noexcept
	: _atlas{{256, 256}}, _dpi{72, 72} // Pre-allocate atlas to make texture() always usable.
{
	assert(!dynamicTextActive());
	_dynamicText = this;

#ifndef NDEBUG
	_atlas.setLabel("(tre) Dynamic Text Renderer Atlas");
#endif
}

tre::DynamicTextManager::~DynamicTextManager() noexcept
{
	_dynamicText = nullptr;
}

const tr::Texture2D& tre::DynamicTextManager::texture() const noexcept
{
	return _atlas.texture();
}

void tre::DynamicTextManager::setDPI(unsigned int dpi) noexcept
{
	setDPI({dpi, dpi});
}

void tre::DynamicTextManager::setDPI(glm::uvec2 dpi) noexcept
{
	assert(dpi.x > 0 && dpi.y > 0);
	_dpi = dpi;
}

tre::Renderer2D::TextureQuad tre::DynamicTextManager::createUnformatted(const char* text, tr::TTFont& font,
																		int fontSize, tr::TTFont::Style style,
																		tr::RGBA8 textColor, TextOutline outline,
																		const Textbox& textbox)
{
	assert(!std::string_view{text}.empty());

	font.resize(fontSize, _dpi);
	font.setStyle(style);
	font.setWrapAlignment(tr::TTFont::WrapAlignment(int(textbox.textAlignment) % 3));

	auto name{std::to_string(_atlas.size())};
	if (outline.thickness != 0) {
		font.setOutline(0);
		const auto textBitmap{font.renderWrapped(text, textColor, textbox.size.x)};
		font.setOutline(outline.thickness);
		auto outlineBitmap{font.renderWrapped(text, outline.color, textbox.size.x)};
		outlineBitmap.blit({outline.thickness, outline.thickness},
						   textBitmap.sub({{}, outlineBitmap.size() - glm::ivec2{outline.thickness * 2}}));
		_atlas.add(name, outlineBitmap);
	}
	else {
		_atlas.add(name, font.renderWrapped(text, textColor, textbox.size.x));
	}
	return createMesh(name, textbox);
}

tre::Renderer2D::TextureQuad tre::DynamicTextManager::createUnformatted(const std::string& text, tr::TTFont& font,
																		int fontSize, tr::TTFont::Style style,
																		tr::RGBA8 textColor, TextOutline outline,
																		const Textbox& textbox)
{
	return createUnformatted(text.c_str(), font, fontSize, style, textColor, outline, textbox);
}

tre::Renderer2D::TextureQuad tre::DynamicTextManager::createFormatted(std::string_view text, tr::TTFont& font,
																	  int fontSize, tr::RGBA8 textColor,
																	  TextOutline outline, const Textbox& textbox)
{
	return createFormatted(text, font, fontSize, {&textColor, 1}, outline, textbox);
}

tre::Renderer2D::TextureQuad tre::DynamicTextManager::createFormatted(std::string_view text, tr::TTFont& font,
																	  int fontSize, std::span<tr::RGBA8> textColors,
																	  TextOutline outline, const Textbox& textbox)
{
	assert(!text.empty());

	const auto name{std::to_string(_atlas.size())};
	const auto align{HorizontalAlign(int(textbox.textAlignment) % 3)};
	const auto bitmap{renderMultistyleText(text, font, fontSize, _dpi, textbox.size.x, align, textColors, outline)};
	_atlas.add(name, bitmap);
	return createMesh(name, textbox);
}

void tre::DynamicTextManager::newFrame() noexcept
{
	_atlas.clear();
}

tre::Renderer2D::TextureQuad tre::DynamicTextManager::createMesh(const std::string& name, const Textbox& textbox)
{
	const tr::RectF2 texture{_atlas[name]};
	const glm::vec2  size{texture.size * glm::vec2(_atlas.texture().size()) / glm::vec2(_dpi) * 72.0f};
	const glm::vec2  posAnchor{calculatePosAnchor(size, textbox)};

	Renderer2D::TextureQuad quad;
	tr::fillRotatedRectangleVertices((quad | tr::positions).begin(), textbox.pos, posAnchor, size, textbox.rotation);
	tr::fillRectVertices((quad | tr::uvs).begin(), texture.tl, texture.size);
	std::ranges::fill(quad | tr::colors, tr::RGBA8{255, 255, 255, 255});
	return quad;
}

bool tre::dynamicTextActive() noexcept
{
	return _dynamicText != nullptr;
}

tre::DynamicTextManager& tre::dynamicText() noexcept
{
	assert(dynamicTextActive());
	return *_dynamicText;
}