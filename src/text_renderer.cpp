#include "../include/tre/text_renderer.hpp"
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
		AlignHorizontal alignment;
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
									AlignHorizontal alignment, std::span<tr::RGBA8> textColors, int outline,
									tr::RGBA8 outlineColor);
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
	const auto end{std::find(std::next(start), textEnd, '\\')};
	std::string copy{start, end};
	for (auto it = copy.begin(); it != copy.end();) {
		auto fit{context.font.measure(std::to_address(it), context.lineLeft).text};
		if (!adjustFit(fit, it, copy.end(), context)) {
			continue;
		}
		emplaceTextPart(it, copy.end(), fit, context);
		calculateLineLeft(context);
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
	case 'n':
		++context.line;
		context.lineLeft = context.maxWidth;
		return std::next(start);
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
	case AlignHorizontal::LEFT:
		return 0;
	case AlignHorizontal::RIGHT:
		return context.maxWidth - accumulate(it, lineEnd, 0, pred);
	case AlignHorizontal::CENTER:
		return context.maxWidth - (accumulate(it, lineEnd, 0, pred)) / 2;
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
									 AlignHorizontal alignment, std::span<tr::RGBA8> textColors, int outline,
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
		else {
			it = handleTextBlock(it, text.end(), context);
		}
	}
	return createFinalBitmap(context);
}

void tre::DynamicTextRenderer::setDPI(unsigned int dpi) noexcept
{
	setDPI(dpi, dpi);
}

void tre::DynamicTextRenderer::setDPI(unsigned int x, unsigned int y) noexcept
{
	assert(x > 0 && y > 0);
	assert(_textboxes.empty());
	_dpi = {x, y};
}

void tre::DynamicTextRenderer::addUnformatted(int priority, const char* text, tr::TTFont& font, int fontSize,
											  tr::TTFont::Style style, tr::RGBA8 textColor, TextOutline outline,
											  const Textbox& textbox)
{
	font.resize(fontSize, _dpi);
	font.setStyle(style);
	font.setWrapAlignment(tr::TTFont::WrapAlignment(int(textbox.textAlign) % 3));

	if (outline.thickness != 0) {
		font.setOutline(0);
		const auto textBitmap{font.renderWrapped(text, textColor, textbox.size.x)};
		font.setOutline(outline.thickness);
		auto outlineBitmap{font.renderWrapped(text, outline.color, textbox.size.x)};
		outlineBitmap.blit({outline.thickness, outline.thickness},
						   textBitmap.sub({{}, outlineBitmap.size() - glm::ivec2{outline.thickness * 2}}));
		_atlas.add(std::format("{}x{}", priority, _textboxes[priority].size()), outlineBitmap);
	}
	else {
		_atlas.add(std::to_string(_textboxes.size()), font.renderWrapped(text, textColor, textbox.size.x));
	}

	_textboxes[priority].push_back(textbox);
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
	const auto align{AlignHorizontal(int(textbox.textAlign) % 3)};
	const auto bitmap{renderMultistyleText(text, font, fontSize, _dpi, textbox.size.x, align, textColors,
										   outline.thickness, outline.color)};
	_atlas.add(std::format("{}x{}", priority, _textboxes[priority].size()), bitmap);
	_textboxes[priority].push_back(textbox);
}

void tre::DynamicTextRenderer::forwardUpToPriority(Renderer2D& renderer, int minPriority)
{
	const std::ranges::subrange range{_textboxes.lower_bound(minPriority), _textboxes.end()};
	for (auto& [priority, textboxes] : range) {
		for (std::size_t i = 0; i < textboxes.size(); ++i) {
			const auto& texture{_atlas[std::format("{}x{}", priority, i)]};
			glm::vec2 size{texture.size * glm::vec2(_atlas.texture().size()) / glm::vec2(_dpi) * 72.0f};

			glm::vec2 tl = textboxes[i].pos - textboxes[i].posAnchor;
			switch (textboxes[i].textAlign) {
			case Align::TOP_LEFT:
				break;
			case Align::TOP_CENTER:
				tl.x += (textboxes[i].size.x - size.x) / 2.0f;
				break;
			case Align::TOP_RIGHT:
				tl.x += textboxes[i].size.x - size.x;
				break;
			case Align::CENTER_LEFT:
				tl.y += (textboxes[i].size.y - size.y) / 2.0f;
				break;
			case Align::CENTER:
				tl += (textboxes[i].size - size) / 2.0f;
				break;
			case Align::CENTER_RIGHT:
				tl.x += textboxes[i].size.x - size.x;
				tl.y += (textboxes[i].size.y - size.y) / 2.0f;
				break;
			case Align::BOTTOM_LEFT:
				tl.y += textboxes[i].size.y - size.y;
				break;
			case Align::BOTTOM_CENTER:
				tl.x += (textboxes[i].size.x - size.x) / 2.0f;
				tl.y += textboxes[i].size.y - size.y;
				break;
			case Align::BOTTOM_RIGHT:
				tl += textboxes[i].size - size;
				break;
			}
		}
	}
}

void tre::DynamicTextRenderer::forward(Renderer2D& renderer)
{
	forwardUpToPriority(renderer, std::numeric_limits<int>::min());
}