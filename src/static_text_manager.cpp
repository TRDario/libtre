#include "../include/tre/static_text_manager.hpp"

namespace tre {
	StaticTextManager* _staticText{nullptr};

	glm::vec2 calculatePosAnchor(glm::vec2 textSize, int maxWidth, HorizontalAlign horizontalAlignment,
								 const StaticTextManager::Textbox& textbox) noexcept;
} // namespace tre

glm::vec2 tre::calculatePosAnchor(glm::vec2 textSize, int maxWidth, HorizontalAlign horizontalAlignment,
								  const StaticTextManager::Textbox& textbox) noexcept
{
	const glm::vec2 textBoxSize{maxWidth, textbox.height};
	switch (Align(int(horizontalAlignment) + int(textbox.textAlignment))) {
	case Align::TOP_LEFT:
		return textbox.posAnchor;
	case Align::TOP_CENTER:
		return {textbox.posAnchor.x - (textBoxSize.x - textSize.x) / 2.0f, textbox.posAnchor.y};
	case Align::TOP_RIGHT:
		return {textbox.posAnchor.x - textBoxSize.x + textSize.x, textbox.posAnchor.y};
	case Align::CENTER_LEFT:
		return {textbox.posAnchor.x, textbox.posAnchor.y - (textBoxSize.y - textSize.y) / 2.0f};
	case Align::CENTER:
		return textbox.posAnchor - (textBoxSize - textSize) / 2.0f;
	case Align::CENTER_RIGHT:
		return {textbox.posAnchor.x - textBoxSize.x + textSize.x,
				textbox.posAnchor.y - (textBoxSize.y - textSize.y) / 2.0f};
	case Align::BOTTOM_LEFT:
		return {textbox.posAnchor.x, textbox.posAnchor.y - textBoxSize.y + textSize.y};
	case Align::BOTTOM_CENTER:
		return {textbox.posAnchor.x - (textBoxSize.x - textSize.x) / 2.0f,
				textbox.posAnchor.y - textBoxSize.y + textSize.y};
	case Align::BOTTOM_RIGHT:
		return textbox.posAnchor - textBoxSize + textSize;
	}
}

tre::StaticTextManager::StaticTextManager() noexcept
	: _dpi{72, 72}
{
	assert(!staticTextActive());
	_staticText = this;

#ifndef NDEBUG
	_atlas.setLabel("(tre) Static Text Renderer Atlas");
#endif
}

tre::StaticTextManager::StaticTextManager(StaticTextManager&& r) noexcept
	: _atlas{std::move(r._atlas)}, _fixedEntryTextboxInfo{std::move(r._fixedEntryTextboxInfo)}, _dpi{r._dpi}
{
	if (_staticText == &r) {
		_staticText = this;
	}
}

tre::StaticTextManager::~StaticTextManager() noexcept
{
	if (_staticText == this) {
		_staticText = nullptr;
	}
}

const tr::Texture2D& tre::StaticTextManager::texture() const noexcept
{
	return _atlas.texture();
}

void tre::StaticTextManager::setDPI(unsigned int dpi) noexcept
{
	setDPI({dpi, dpi});
}

void tre::StaticTextManager::setDPI(glm::uvec2 dpi) noexcept
{
	if (dpi != _dpi) {
		assert(dpi.x > 0 && dpi.y > 0);
		_dpi = dpi;
		_atlas.clear();
	}
}

void tre::StaticTextManager::newUnformattedEntry(std::string name, const char* text, tr::TTFont& font, int fontSize,
												 tr::TTFont::Style style, tr::RGBA8 textColor, TextOutline outline,
												 int maxWidth, HorizontalAlign alignment)
{
	assert(!_atlas.contains(name));

	if (std::string_view{text}.empty()) {
		return;
	}

	font.resize(fontSize, _dpi);
	font.setStyle(style);
	font.setWrapAlignment(tr::TTFont::WrapAlignment(alignment));

	if (outline.thickness != 0) {
		font.setOutline(0);
		const auto textBitmap{font.renderWrapped(text, textColor, maxWidth)};
		font.setOutline(outline.thickness);
		auto outlineBitmap{font.renderWrapped(text, outline.color, maxWidth)};
		outlineBitmap.blit({outline.thickness, outline.thickness},
						   textBitmap.sub({{}, outlineBitmap.size() - glm::ivec2{outline.thickness * 2}}));
		_atlas.add(std::move(name), outlineBitmap);
	}
	else {
		_atlas.add(std::move(name), font.renderWrapped(text, textColor, maxWidth));
	}
}

void tre::StaticTextManager::newUnformattedEntry(std::string name, const std::string& text, tr::TTFont& font,
												 int fontSize, tr::TTFont::Style style, tr::RGBA8 textColor,
												 TextOutline outline, int maxWidth, HorizontalAlign alignment)
{
	newUnformattedEntry(std::move(name), text.c_str(), font, fontSize, style, textColor, outline, maxWidth, alignment);
}

void tre::StaticTextManager::newFormattedEntry(std::string name, std::string_view text, tr::TTFont& font, int fontSize,
											   tr::RGBA8 textColor, TextOutline outline, int maxWidth,
											   HorizontalAlign alignment)
{
	newFormattedEntry(std::move(name), text, font, fontSize, {&textColor, 1}, outline, maxWidth, alignment);
}

void tre::StaticTextManager::newFormattedEntry(std::string name, std::string_view text, tr::TTFont& font, int fontSize,
											   std::span<tr::RGBA8> textColors, TextOutline outline, int maxWidth,
											   HorizontalAlign alignment)
{
	assert(!_atlas.contains(name));

	if (text.empty()) {
		return;
	}

	const auto bitmap{renderMultistyleText(text, font, fontSize, _dpi, maxWidth, alignment, textColors, outline)};
	_atlas.add(std::move(name), bitmap);
}

void tre::StaticTextManager::removeEntry(std::string_view name) noexcept
{
	_atlas.remove(name);
}

tre::Renderer2D::TextureQuad tre::StaticTextManager::createMesh(std::string_view entry, const Textbox& textbox) noexcept
{
	assert(_atlas.contains(entry));

	const auto&     uv{_atlas[entry]};
	const auto&     fixedInfo{_fixedEntryTextboxInfo.find(entry)->second};
	const glm::vec2 size{uv.size * glm::vec2(_atlas.texture().size()) / glm::vec2(_dpi) * 72.0f};
	const glm::vec2 posAnchor{calculatePosAnchor(size, fixedInfo.width, fixedInfo.textAlignment, textbox)};

	Renderer2D::TextureQuad quad;
	tr::fillRotatedRectangleVertices((quad | tr::positions).begin(), textbox.pos, posAnchor, size, textbox.rotation);
	tr::fillRectVertices((quad | tr::uvs).begin(), uv.tl, uv.size);
	std::ranges::fill(quad | tr::colors, textbox.tint);
	return quad;
}

bool tre::staticTextActive() noexcept
{
	return _staticText != nullptr;
}

tre::StaticTextManager& tre::staticText() noexcept
{
	assert(staticTextActive());
	return *_staticText;
}