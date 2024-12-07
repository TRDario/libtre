#include "../include/tre/dynamic_text_renderer.hpp"
#include "../include/tre/renderer_2d.hpp"
#include "../include/tre/sampler.hpp"

using namespace tr::matrix_operators;

namespace tre {
	DynamicTextRenderer* _dynamicTextRenderer{nullptr};

	glm::vec2 calculatePosAnchor(glm::vec2 textSize, const DynamicTextRenderer::Textbox& textbox) noexcept;
} // namespace tre

glm::vec2 tre::calculatePosAnchor(glm::vec2 textSize, const DynamicTextRenderer::Textbox& textbox) noexcept
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

tre::DynamicTextRenderer::DynamicTextRenderer() noexcept
	: _dpi{72, 72}
{
	assert(!dynamicTextRendererActive());
	_dynamicTextRenderer = this;

#ifndef NDEBUG
	_atlas.setLabel("(tre) Dynamic Text Renderer Atlas");
#endif
}

tre::DynamicTextRenderer::~DynamicTextRenderer() noexcept
{
	_dynamicTextRenderer = nullptr;
}

void tre::DynamicTextRenderer::setDPI(unsigned int dpi) noexcept
{
	setDPI({dpi, dpi});
}

void tre::DynamicTextRenderer::setDPI(glm::uvec2 dpi) noexcept
{
	assert(dpi.x > 0 && dpi.y > 0);
	_dpi = dpi;
}

void tre::DynamicTextRenderer::addUnformatted(int priority, const char* text, tr::TTFont& font, int fontSize,
											  tr::TTFont::Style style, tr::RGBA8 textColor, TextOutline outline,
											  const Textbox& textbox)
{
	if (std::string_view{text}.empty()) {
		return;
	}

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
	addToRenderer(priority, name, textbox);
}

void tre::DynamicTextRenderer::addUnformatted(int priority, const std::string& text, tr::TTFont& font, int fontSize,
											  tr::TTFont::Style style, tr::RGBA8 textColor, TextOutline outline,
											  const Textbox& textbox)
{
	addUnformatted(priority, text.c_str(), font, fontSize, style, textColor, outline, textbox);
}

void tre::DynamicTextRenderer::addFormatted(int priority, std::string_view text, tr::TTFont& font, int fontSize,
											tr::RGBA8 textColor, TextOutline outline, const Textbox& textbox)
{
	addFormatted(priority, text, font, fontSize, {&textColor, 1}, outline, textbox);
}

void tre::DynamicTextRenderer::addFormatted(int priority, std::string_view text, tr::TTFont& font, int fontSize,
											std::span<tr::RGBA8> textColors, TextOutline outline,
											const Textbox& textbox)
{
	if (text.empty()) {
		return;
	}

	const auto name{std::to_string(_atlas.size())};
	const auto align{HorizontalAlign(int(textbox.textAlignment) % 3)};
	const auto bitmap{renderMultistyleText(text, font, fontSize, _dpi, textbox.size.x, align, textColors, outline)};
	_atlas.add(name, bitmap);
	addToRenderer(priority, name, textbox);
}

void tre::DynamicTextRenderer::newFrame()
{
	_atlas.clear();
}

void tre::DynamicTextRenderer::addToRenderer(int priority, const std::string& name, const Textbox& textbox)
{
	const tr::RectF2    texture{_atlas[name]};
	const glm::vec2     size{texture.size * glm::vec2(_atlas.texture().size()) / glm::vec2(_dpi) * 72.0f};
	const glm::vec2     posAnchor{calculatePosAnchor(size, textbox)};
	constexpr tr::RGBA8 TINT{255, 255, 255, 255};
	renderer2D().addTexturedRotatedRectangle(priority, textbox.pos, posAnchor, size, textbox.rotation,
											 {_atlas.texture(), bilinearSampler()}, texture, TINT);
}

bool tre::dynamicTextRendererActive() noexcept
{
	return _dynamicTextRenderer != nullptr;
}

tre::DynamicTextRenderer& tre::dynamicTextRenderer() noexcept
{
	assert(dynamicTextRendererActive());
	return *_dynamicTextRenderer;
}