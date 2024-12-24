#include "../include/tre/text.hpp"
#include <numeric>

namespace tre {
	struct TextPart {
		tr::Bitmap bitmap;
		int        line;
	};

	struct MultistyleTextContext {
		tr::TTFont&           font;
		std::vector<TextPart> parts;
		int                   line;
		int                   lineLeft;
		int                   maxWidth;
		TextOutline           outline;
		tr::RGBA8             curTextColor;
		std::span<tr::RGBA8>  textColors;
		HorizontalAlign       alignment;
	};

	// Renders outlined text.
	tr::Bitmap renderOutlinedText(const char* text, const MultistyleTextContext& ctx);

	// Adjusts the point at which a string is broken for things like breaking at whitespaces.
	bool adjustFit(std::string_view& fit, std::string::iterator& start, std::string::iterator end,
				   MultistyleTextContext& ctx) noexcept;

	// Renders a section of the text with a single style.
	void createTextPart(std::string::iterator end, std::string::iterator stringEnd, std::string_view fit,
						MultistyleTextContext& ctx);

	// Handles the rendering of text parts for a block of text with a single consistent style.
	std::string_view::iterator handleTextBlock(std::string_view::iterator start, std::string_view::iterator textEnd,
											   MultistyleTextContext& ctx);

	// Handles the parsing of escaped control sequences and sets the context state accordingly.
	std::string_view::iterator handleControlSequence(std::string_view::iterator start,
													 std::string_view::iterator textEnd, MultistyleTextContext& ctx);

	// Calculates the starting X offset for a text part.
	int calculateStartingX(std::vector<TextPart>::iterator it, std::vector<TextPart>::iterator lineEnd,
						   const MultistyleTextContext& ctx) noexcept;

	// Determines the size of the output bitmap.
	glm::ivec2 calculateBitmapSize(const MultistyleTextContext& ctx) noexcept;

	// Stitches together the final output bitmap.
	tr::Bitmap createBitmap(MultistyleTextContext& ctx);
} // namespace tre

tr::Bitmap tre::renderOutlinedText(const char* text, const MultistyleTextContext& ctx)
{
	ctx.font.setOutline(0);
	auto textBitmap{ctx.font.render(text, ctx.curTextColor)};
	ctx.font.setOutline(ctx.outline.thickness);
	auto outlineBitmap{ctx.font.render(text, ctx.outline.color)};
	outlineBitmap.blit({ctx.outline.thickness, ctx.outline.thickness},
					   textBitmap.sub({{}, outlineBitmap.size() - glm::ivec2{ctx.outline.thickness * 2}}));
	return outlineBitmap;
}

bool tre::adjustFit(std::string_view& fit, std::string::iterator& start, std::string::iterator end,
					MultistyleTextContext& ctx) noexcept
{
	if (fit != std::string_view{start, end}) {
		const auto lastWhitespace{fit.rfind(' ')};
		if (lastWhitespace != fit.npos) {
			fit = {fit.data(), lastWhitespace};
			start += fit.size() + 1;
			return true;
		}
		else if (ctx.lineLeft != ctx.maxWidth) {
			++ctx.line;
			ctx.lineLeft = ctx.maxWidth;
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

void tre::createTextPart(std::string::iterator end, std::string::iterator stringEnd, std::string_view fit,
						 MultistyleTextContext& ctx)
{
	if (end == stringEnd) {
		const auto ptr{std::to_address(end) - fit.size()};
		ctx.parts.emplace_back(ctx.outline.thickness == 0 ? ctx.font.render(ptr, ctx.curTextColor)
														  : renderOutlinedText(ptr, ctx),
							   ctx.line);
	}
	else {
		std::string copy{fit};
		ctx.parts.emplace_back(ctx.outline.thickness == 0 ? ctx.font.render(copy.c_str(), ctx.curTextColor)
														  : renderOutlinedText(copy.c_str(), ctx),
							   ctx.line);
	}
}

std::string_view::iterator tre::handleTextBlock(std::string_view::iterator start, std::string_view::iterator textEnd,
												MultistyleTextContext& ctx)
{
	const auto  end{std::ranges::find_first_of(std::ranges::subrange{std::next(start), textEnd}, "\n\\")};
	std::string copy{start, end};
	for (auto it = copy.begin(); it != copy.end();) {
		auto fit{ctx.font.measure(std::to_address(it), ctx.lineLeft)};
		if (!adjustFit(fit.text, it, copy.end(), ctx)) {
			continue;
		}
		createTextPart(it, copy.end(), fit.text, ctx);
		ctx.lineLeft -= fit.width - 2 * ctx.outline.thickness;
		if (ctx.lineLeft < 0) {
			++ctx.line;
			ctx.lineLeft = ctx.maxWidth;
		}
	}
	return end;
}

std::string_view::iterator tre::handleControlSequence(std::string_view::iterator start,
													  std::string_view::iterator textEnd, MultistyleTextContext& ctx)
{
	if (start == textEnd) {
		return start;
	}

	switch (*start) {
	case '\\':
		return handleTextBlock(start, textEnd, ctx);
	case '!':
		ctx.curTextColor = ctx.textColors[0];
		break;
	case 'c':
		if (std::next(start) != textEnd && std::isdigit(*++start) && *start - '0' < ctx.textColors.size()) {
			ctx.curTextColor = ctx.textColors[*start - '0'];
		}
		break;
	case 'b':
		ctx.font.setStyle(ctx.font.style() ^ tr::TTFont::Style::BOLD);
		break;
	case 'i':
		ctx.font.setStyle(ctx.font.style() ^ tr::TTFont::Style::ITALIC);
		break;
	case 's':
		ctx.font.setStyle(ctx.font.style() ^ tr::TTFont::Style::STRIKETHROUGH);
		break;
	case 'u':
		ctx.font.setStyle(ctx.font.style() ^ tr::TTFont::Style::UNDERLINE);
		break;
	default:
		break;
	}
	return std::next(start);
}

int tre::calculateStartingX(std::vector<TextPart>::iterator it, std::vector<TextPart>::iterator lineEnd,
							const MultistyleTextContext& ctx) noexcept
{
	const auto pred{[&](int sum, auto& part) { return sum + part.bitmap.size().x - 2 * ctx.outline.thickness; }};
	switch (ctx.alignment) {
	case HorizontalAlign::LEFT:
		return 0;
	case HorizontalAlign::CENTER:
		return (ctx.maxWidth - std::accumulate(it, lineEnd, 0, pred)) / 2 - ctx.outline.thickness;
	case HorizontalAlign::RIGHT:
		return ctx.maxWidth - std::accumulate(it, lineEnd, 0, pred) - 2 * ctx.outline.thickness;
	}
}

glm::ivec2 tre::calculateBitmapSize(const MultistyleTextContext& ctx) noexcept
{
	glm::ivec2 bitmapSize{ctx.maxWidth, ctx.font.lineSkip()};
	for (auto& part : ctx.parts) {
		const auto partSize{part.bitmap.size()};
		bitmapSize = max(bitmapSize, {partSize.x, part.line * ctx.font.lineSkip() + partSize.y});
	}
	return bitmapSize;
}

tr::Bitmap tre::createBitmap(MultistyleTextContext& ctx)
{
	tr::Bitmap bitmap{calculateBitmapSize(ctx), ctx.parts.front().bitmap.format()};
	for (auto it = ctx.parts.begin(); it != ctx.parts.end();) {
		auto lineEnd{std::find_if(it, ctx.parts.end(), [=](auto& part) { return part.line != it->line; })};
		for (int x{calculateStartingX(it, lineEnd, ctx)}; it != lineEnd; ++it) {
			bitmap.blit({x, it->line * ctx.font.lineSkip()}, it->bitmap);
			x += it->bitmap.size().x - 2 * ctx.outline.thickness;
		}
	}
	return bitmap;
}

tr::Bitmap tre::renderMultistyleText(std::string_view text, tr::TTFont& font, int size, glm::uvec2 dpi, int maxWidth,
									 HorizontalAlign alignment, std::span<tr::RGBA8> textColors, TextOutline outline)
{
	assert(!text.empty());
	assert(!textColors.empty());
	font.setStyle(tr::TTFont::Style::NORMAL);
	font.setOutline(outline.thickness);
	font.resize(size, dpi);

	maxWidth = maxWidth * dpi.x / 72;
	MultistyleTextContext ctx{font, {}, 0, maxWidth, maxWidth, outline, textColors.front(), textColors, alignment};
	for (auto it = text.begin(); it != text.end();) {
		if (*it == '\\') {
			it = handleControlSequence(++it, text.end(), ctx);
		}
		else if (*it == '\n') {
			++ctx.line;
			ctx.lineLeft = ctx.maxWidth;
			++it;
		}
		else {
			it = handleTextBlock(it, text.end(), ctx);
		}
	}
	return createBitmap(ctx);
}