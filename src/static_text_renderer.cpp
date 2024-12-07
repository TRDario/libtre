#include "../include/tre/static_text_renderer.hpp"
#include "../include/tre/renderer_2d.hpp"
#include "../include/tre/sampler.hpp"

namespace tre {
	StaticTextRenderer* _staticTextRenderer{nullptr};

	glm::vec2 calculatePosAnchor(glm::vec2 textSize, int maxWidth, HorizontalAlign horizontalAlignment,
								 const StaticTextRenderer::Textbox& textbox) noexcept;
} // namespace tre

glm::vec2 tre::calculatePosAnchor(glm::vec2 textSize, int maxWidth, HorizontalAlign horizontalAlignment,
								  const StaticTextRenderer::Textbox& textbox) noexcept
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

tre::StaticTextRenderer::StaticTextRenderer() noexcept
	: _dpi{72, 72}
{
	assert(!staticTextRendererActive());
	_staticTextRenderer = this;

#ifndef NDEBUG
	_atlas.setLabel("(tre) Static Text Renderer Atlas");
#endif
}

tre::StaticTextRenderer::~StaticTextRenderer() noexcept
{
	_staticTextRenderer = nullptr;
}

void tre::StaticTextRenderer::setDPI(unsigned int dpi) noexcept
{
	setDPI({dpi, dpi});
}

void tre::StaticTextRenderer::setDPI(glm::uvec2 dpi) noexcept
{
	if (dpi != _dpi) {
		assert(dpi.x > 0 && dpi.y > 0);
		_dpi = dpi;
		_atlas.clear();
	}
}

void tre::StaticTextRenderer::newUnformattedEntry(std::string name, const char* text, tr::TTFont& font, int fontSize,
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

void tre::StaticTextRenderer::newUnformattedEntry(std::string name, const std::string& text, tr::TTFont& font,
												  int fontSize, tr::TTFont::Style style, tr::RGBA8 textColor,
												  TextOutline outline, int maxWidth, HorizontalAlign alignment)
{
	newUnformattedEntry(std::move(name), text.c_str(), font, fontSize, style, textColor, outline, maxWidth, alignment);
}

void tre::StaticTextRenderer::newFormattedEntry(std::string name, std::string_view text, tr::TTFont& font, int fontSize,
												tr::RGBA8 textColor, TextOutline outline, int maxWidth,
												HorizontalAlign alignment)
{
	newFormattedEntry(std::move(name), text, font, fontSize, {&textColor, 1}, outline, maxWidth, alignment);
}

void tre::StaticTextRenderer::newFormattedEntry(std::string name, std::string_view text, tr::TTFont& font, int fontSize,
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

void tre::StaticTextRenderer::removeEntry(std::string_view name) noexcept
{
	_atlas.remove(name);
}

void tre::StaticTextRenderer::addInstance(int priority, std::string_view entry, const Textbox& textbox)
{
	if (!_atlas.contains(entry)) {
		return;
	}

	const auto& uv{_atlas[entry]};
	const auto& fixedInfo{_fixedEntryTextboxInfo.find(entry)->second};

	const glm::vec2              size{uv.size * glm::vec2(_atlas.texture().size()) / glm::vec2(_dpi) * 72.0f};
	const glm::vec2              posAnchor{calculatePosAnchor(size, fixedInfo.width, fixedInfo.textAlignment, textbox)};
	const Renderer2D::TextureRef texture{_atlas.texture(), bilinearSampler()};

	renderer2D().addTexturedRotatedRectangle(priority, textbox.pos, posAnchor, size, textbox.rotation, texture, uv,
											 textbox.tint);
}

bool tre::staticTextRendererActive() noexcept
{
	return _staticTextRenderer != nullptr;
}

tre::StaticTextRenderer& tre::staticTextRenderer() noexcept
{
	assert(staticTextRendererActive());
	return *_staticTextRenderer;
}