#include "../include/tre/text_renderer.hpp"
#include "../include/tre/sampler.hpp"
#include <numeric>

namespace tre {
	struct TextPart {
		tr::Bitmap bitmap;
		int line;
	};

	struct MultistyleTextContext {
		tr::TTFont& font;
		std::vector<TextPart> parts;
		int line;
		int lineLeft;
		int maxWidth;
		int outline;
		tr::RGBA8 curTextColor;
		tr::RGBA8 outlineColor;
		std::span<tr::RGBA8> textColors;
		HorizontalAlign alignment;
	};

	glm::ivec2 calculateFinalBitmapSize(const MultistyleTextContext& context) noexcept;

	tr::Bitmap renderOutlinedText(const char* text, const MultistyleTextContext& context);

	bool adjustFit(std::string_view& fit, std::string::iterator& start, std::string::iterator end,
				   MultistyleTextContext& context) noexcept;

	void emplaceTextPart(std::string::iterator end, std::string::iterator stringEnd, std::string_view fit,
						 MultistyleTextContext& context);

	void calculateLineLeft(MultistyleTextContext& context) noexcept;

	std::string_view::iterator handleTextBlock(std::string_view::iterator start, std::string_view::iterator textEnd,
											   MultistyleTextContext& context);

	std::string_view::iterator handleControlSequence(std::string_view::iterator start,
													 std::string_view::iterator textEnd,
													 MultistyleTextContext& context);

	int calculateXOffset(std::vector<TextPart>::iterator it, std::vector<TextPart>::iterator lineEnd,
						 const MultistyleTextContext& context) noexcept;

	tr::Bitmap createFinalBitmap(MultistyleTextContext& context);

	tr::Bitmap renderMultistyleText(std::string_view text, tr::TTFont& font, int size, glm::uvec2 dpi, int maxWidth,
									HorizontalAlign alignment, std::span<tr::RGBA8> textColors, int outline,
									tr::RGBA8 outlineColor);

	glm::vec2 calculatePosAnchor(glm::vec2 textSize, int maxWidth, HorizontalAlign horizontalAlignment,
								 const StaticTextRenderer::Textbox& textbox) noexcept;

	glm::vec2 calculatePosAnchor(glm::vec2 textSize, const DynamicTextRenderer::Textbox& textbox) noexcept;
} // namespace tre

glm::ivec2 tre::calculateFinalBitmapSize(const MultistyleTextContext& context) noexcept
{
	glm::ivec2 bitmapSize{context.maxWidth, context.font.lineSkip()};
	for (auto& part : context.parts) {
		const auto partSize{part.bitmap.size()};
		bitmapSize = max(bitmapSize, {partSize.x, part.line * context.font.lineSkip() + partSize.y});
	}
	return bitmapSize;
}

tr::Bitmap tre::renderOutlinedText(const char* text, const MultistyleTextContext& context)
{
	context.font.setOutline(0);
	auto textBitmap{context.font.render(text, context.curTextColor)};
	context.font.setOutline(context.outline);
	auto outlineBitmap{context.font.render(text, context.outlineColor)};
	outlineBitmap.blit({context.outline, context.outline},
					   textBitmap.sub({{}, outlineBitmap.size() - glm::ivec2{context.outline * 2}}));
	return outlineBitmap;
}

bool tre::adjustFit(std::string_view& fit, std::string::iterator& start, std::string::iterator end,
					MultistyleTextContext& context) noexcept
{
	if (fit != std::string_view{start, end}) {
		const auto lastWhitespace{fit.rfind(' ')};
		if (lastWhitespace != fit.npos) {
			fit = {fit.data(), lastWhitespace};
			start += fit.size() + 1;
			return true;
		}
		else if (context.lineLeft != context.maxWidth) {
			++context.line;
			context.lineLeft = context.maxWidth;
			return false;
		}
		else if (!fit.empty()) {
			start += fit.size();
			return true;
		}
		else {
			fit = std::string_view{start, ++start};
			return true;
		}
	}
	else {
		start += fit.size();
		return true;
	}
}

void tre::emplaceTextPart(std::string::iterator end, std::string::iterator stringEnd, std::string_view fit,
						  MultistyleTextContext& context)
{
	if (end == stringEnd) {
		const auto ptr{std::to_address(end) - fit.size()};
		context.parts.emplace_back(context.outline == 0 ? context.font.render(ptr, context.curTextColor)
														: renderOutlinedText(ptr, context),
								   context.line);
	}
	else {
		std::string copy{fit};
		context.parts.emplace_back(context.outline == 0 ? context.font.render(copy.c_str(), context.curTextColor)
														: renderOutlinedText(copy.c_str(), context),
								   context.line);
	}
}

void tre::calculateLineLeft(MultistyleTextContext& context) noexcept
{
	context.lineLeft -= context.parts.back().bitmap.size().x - 2 * context.outline;
	if (context.lineLeft < 0) {
		++context.line;
		context.lineLeft = context.maxWidth;
	}
}

std::string_view::iterator tre::handleTextBlock(std::string_view::iterator start, std::string_view::iterator textEnd,
												MultistyleTextContext& context)
{
	constexpr std::array<char, 2> DELIMITERS{'\\', '\n'};
	const auto end{std::find_first_of(std::next(start), textEnd, DELIMITERS.begin(), DELIMITERS.end())};
	std::string copy{start, end};
	for (auto it = copy.begin(); it != copy.end();) {
		auto fit{context.font.measure(std::to_address(it), context.lineLeft)};
		if (!adjustFit(fit.text, it, copy.end(), context)) {
			continue;
		}
		emplaceTextPart(it, copy.end(), fit.text, context);
		context.lineLeft -= fit.width - 2 * context.outline;
		if (context.lineLeft < 0) {
			++context.line;
			context.lineLeft = context.maxWidth;
		}
	}
	return end;
}

std::string_view::iterator tre::handleControlSequence(std::string_view::iterator start,
													  std::string_view::iterator textEnd,
													  MultistyleTextContext& context)
{
	if (start == textEnd) {
		return start;
	}

	switch (*start) {
	case '\\':
		return handleTextBlock(start, textEnd, context);
	case 'c':
		if (std::next(start) != textEnd && std::isdigit(*++start) && *start - '0' < context.textColors.size()) {
			context.curTextColor = context.textColors[*start - '0'];
		}
		return std::next(start);
	case 'b':
		context.font.setStyle(context.font.style() ^ tr::TTFont::Style::BOLD);
		return std::next(start);
	case 'i':
		context.font.setStyle(context.font.style() ^ tr::TTFont::Style::ITALIC);
		return std::next(start);
	case 's':
		context.font.setStyle(context.font.style() ^ tr::TTFont::Style::STRIKETHROUGH);
		return std::next(start);
	case 'u':
		context.font.setStyle(context.font.style() ^ tr::TTFont::Style::UNDERLINE);
		return std::next(start);
	default:
		return std::next(start);
	}
}

int tre::calculateXOffset(std::vector<TextPart>::iterator it, std::vector<TextPart>::iterator lineEnd,
						  const MultistyleTextContext& context) noexcept
{
	const auto pred{[&](int sum, auto& part) { return sum + part.bitmap.size().x - 2 * context.outline; }};
	switch (context.alignment) {
	case HorizontalAlign::LEFT:
		return 0;
	case HorizontalAlign::RIGHT:
		return context.maxWidth - std::accumulate(it, lineEnd, 0, pred) - 2 * context.outline;
	case HorizontalAlign::CENTER:
		return (context.maxWidth - std::accumulate(it, lineEnd, 0, pred)) / 2 - context.outline;
	}
}

tr::Bitmap tre::createFinalBitmap(MultistyleTextContext& context)
{
	tr::Bitmap bitmap{calculateFinalBitmapSize(context), context.parts.front().bitmap.format()};
	for (auto it = context.parts.begin(); it != context.parts.end();) {
		auto lineEnd{std::find_if(it, context.parts.end(), [=](auto& part) { return part.line != it->line; })};
		for (int xOffset{calculateXOffset(it, lineEnd, context)}; it != lineEnd; ++it) {
			bitmap.blit({xOffset, it->line * context.font.lineSkip()}, it->bitmap);
			xOffset += it->bitmap.size().x - 2 * context.outline;
		}
	}
	return bitmap;
}

tr::Bitmap tre::renderMultistyleText(std::string_view text, tr::TTFont& font, int size, glm::uvec2 dpi, int maxWidth,
									 HorizontalAlign alignment, std::span<tr::RGBA8> textColors, int outline,
									 tr::RGBA8 outlineColor)
{
	assert(!text.empty());
	assert(!textColors.empty());

	font.setStyle(tr::TTFont::Style::NORMAL);
	font.setOutline(outline);
	font.resize(size, dpi);

	MultistyleTextContext context{font,         {},         0,        maxWidth, maxWidth, outline, textColors.front(),
								  outlineColor, textColors, alignment};
	for (auto it = text.begin(); it != text.end();) {
		if (*it == '\\') {
			it = handleControlSequence(++it, text.end(), context);
		}
		else if (*it == '\n') {
			++context.line;
			context.lineLeft = context.maxWidth;
			++it;
		}
		else {
			it = handleTextBlock(it, text.end(), context);
		}
	}
	return createFinalBitmap(context);
}

tre::StaticTextRenderer::StaticTextRenderer() noexcept
	: _dpi{72, 72}
{
#ifndef NDEBUG
	_atlas.setLabel("(tre) Static Text Renderer Atlas");
#endif
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

	const auto bitmap{renderMultistyleText(text, font, fontSize, _dpi, maxWidth, alignment, textColors,
										   outline.thickness, outline.color)};
	_atlas.add(std::move(name), bitmap);
}

void tre::StaticTextRenderer::removeEntry(std::string_view name) noexcept
{
	_atlas.remove(name);
}

void tre::StaticTextRenderer::addInstance(int priority, std::string_view entry, const Textbox& textbox)
{
	if (_atlas.contains(entry)) {
		_instances[priority].emplace_back(textbox, _atlas[entry], _fixedEntryTextboxInfo.find(entry)->second);
	}
}

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

void tre::StaticTextRenderer::forward(Renderer2D& renderer)
{
	for (auto& [priority, instances] : _instances) {
		for (std::size_t i = 0; i < instances.size(); ++i) {
			const auto& [textbox, uv, fixedInfo]{instances[i]};
			const glm::vec2 size{uv.size * glm::vec2(_atlas.texture().size()) / glm::vec2(_dpi) * 72.0f};
			const glm::vec2 posAnchor{calculatePosAnchor(size, fixedInfo.width, fixedInfo.textAlignment, textbox)};
			const Renderer2D::TextureRef texture{_atlas.texture(), bilinearSampler()};

			renderer.addTexturedRotatedRectangle(priority, textbox.pos, posAnchor, size, textbox.rotation, texture, uv,
												 textbox.tint);
		}
	}
	_instances.clear();
}

tre::DynamicTextRenderer::DynamicTextRenderer() noexcept
	: _dpi{72, 72}
{
#ifndef NDEBUG
	_atlas.setLabel("(tre) Dynamic Text Renderer Atlas");
#endif
}

void tre::DynamicTextRenderer::setDPI(unsigned int dpi) noexcept
{
	setDPI({dpi, dpi});
}

void tre::DynamicTextRenderer::setDPI(glm::uvec2 dpi) noexcept
{
	assert(dpi.x > 0 && dpi.y > 0);
	assert(_textboxes.empty());
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

	auto name{std::format("{}x{}", priority, _textboxes[priority].size())};
	if (outline.thickness != 0) {
		font.setOutline(0);
		const auto textBitmap{font.renderWrapped(text, textColor, textbox.size.x)};
		font.setOutline(outline.thickness);
		auto outlineBitmap{font.renderWrapped(text, outline.color, textbox.size.x)};
		outlineBitmap.blit({outline.thickness, outline.thickness},
						   textBitmap.sub({{}, outlineBitmap.size() - glm::ivec2{outline.thickness * 2}}));
		_atlas.add(std::move(name), outlineBitmap);
	}
	else {
		_atlas.add(std::move(name), font.renderWrapped(text, textColor, textbox.size.x));
	}

	_textboxes[priority].push_back(textbox);
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

	const auto align{HorizontalAlign(int(textbox.textAlignment) % 3)};
	const auto bitmap{renderMultistyleText(text, font, fontSize, _dpi, textbox.size.x, align, textColors,
										   outline.thickness, outline.color)};
	_atlas.add(std::format("{}x{}", priority, _textboxes[priority].size()), bitmap);
	_textboxes[priority].push_back(textbox);
}

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

void tre::DynamicTextRenderer::forward(Renderer2D& renderer)
{
	for (auto& [priority, textboxes] : _textboxes) {
		for (std::size_t i = 0; i < textboxes.size(); ++i) {
			const auto& textbox{textboxes[i]};
			const auto name{std::format("{}x{}", priority, i)};
			const auto texture{_atlas[name]};
			const glm::vec2 size{texture.size * glm::vec2(_atlas.texture().size()) / glm::vec2(_dpi) * 72.0f};
			const glm::vec2 posAnchor{calculatePosAnchor(size, textbox)};

			renderer.addTexturedRotatedRectangle(priority, textbox.pos, posAnchor, size, textbox.rotation,
												 {_atlas.texture(), bilinearSampler()}, texture, {255, 255, 255, 255});
		}
	}
	_atlas.clear();
	_textboxes.clear();
}