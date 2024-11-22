#include "../include/tre/debug_text_renderer.hpp"
#include "../include/tre/renderer_base.hpp"

void tre::DebugTextRenderer::setScale(float scale) noexcept
{
	_shaderPipeline.vertexShader().setUniform(1, scale);
}

void tre::DebugTextRenderer::setColumnLimit(std::uint8_t columns) noexcept
{
	_columnLimit = columns;
}

void tre::DebugTextRenderer::clear() noexcept
{
	_shaderGlyphs.clear();
	_leftLine = 0;
}

void tre::DebugTextRenderer::writeRegularChar(char chr, std::uint8_t& line, std::uint8_t& lineLength,
											  std::optional<decltype(_shaderGlyphs)::iterator> lineStart,
											  std::optional<decltype(_shaderGlyphs)::iterator> wordStart,
											  tr::RGBA8 textColor, tr::RGBA8 backgroundColor, Align alignment)
{
	if (lineLength >= 80) {
		lineLength = 0;
		++line;

		if (wordStart.has_value()) {
			if (wordStart != lineStart) {
				for (auto& glyph : std::ranges::subrange{*wordStart, _shaderGlyphs.end()}) {
					glyph.pos = {lineLength++, glyph.pos.y + 1};
				}
			}

			if (alignment == Align::RIGHT) {
				for (auto it = *lineStart; it != *wordStart; ++it) {
					it->pos.x = std::distance(it, *wordStart);
				}
			}
		}

		lineStart = wordStart;
	}

	if (chr != ' ') {
		_shaderGlyphs.push_back({{lineLength, line}, alignment == Align::RIGHT, chr, textColor, backgroundColor});
		if (!lineStart.has_value()) {
			lineStart = _shaderGlyphs.end() - 1;
		}
		if (!wordStart.has_value()) {
			wordStart = _shaderGlyphs.end() - 1;
		}
	}
	else {
		wordStart.reset();
	}
	++lineLength;
}

void tre::DebugTextRenderer::write(std::string_view text, tr::RGBA8 textColor, tr::RGBA8 backgroundColor,
								   std::span<tr::RGBA8> extraColors, Align alignment)
{
	if (text.empty()) {
		return;
	}

	std::uint8_t& line{alignment == Align::LEFT ? _leftLine : _rightLine};
	const auto oldGlyphsSize{_shaderGlyphs.size()};

	std::uint8_t lineLength = 0;
	std::optional<decltype(_shaderGlyphs)::iterator> lineStart;
	std::optional<decltype(_shaderGlyphs)::iterator> wordStart;
	tr::RGBA8 curTextColor{textColor};
	tr::RGBA8 curBgColor{backgroundColor};

	for (auto it = text.begin(); it != text.end(); ++it) {
		if (*it == '\\') {
			++it;
			switch (*it) {
			case 'b':
				++it;
				if (std::isdigit(*it) && *it - '0' < extraColors.size()) {
					curBgColor = extraColors[*it - '0'];
				}
				break;
			case 'B':
				curBgColor = backgroundColor;
				break;
			case 'c':
				++it;
				if (std::isdigit(*it) && *it - '0' < extraColors.size()) {
					curTextColor = extraColors[*it - '0'];
				}
				break;
			case 'C':
				curTextColor = textColor;
				break;
			case 'i':
				++it;
				if (std::isdigit(*it)) {
					for (int i = 0; i < *it - '0'; ++i) {
						writeRegularChar(' ', line, lineLength, lineStart, wordStart, curTextColor, curBgColor,
										 alignment);
					}
				}
				break;
			case 'n':
				if (alignment == Align::RIGHT && lineStart.has_value()) {
					for (auto it = *lineStart; it != _shaderGlyphs.end(); ++it) {
						it->pos.x = std::distance(it, _shaderGlyphs.end());
					}
				}

				lineLength = 0;
				wordStart.reset();
				lineStart.reset();
				++line;
				break;
			case 't':
				for (int i = 0; i < 4; ++i) {
					writeRegularChar(' ', line, lineLength, lineStart, wordStart, curTextColor, curBgColor, alignment);
				}
			default:
				break;
			}
		}
		else {
			writeRegularChar(*it, line, lineLength, lineStart, wordStart, curTextColor, curBgColor, alignment);
		}
	}
	++line;
}

void tre::DebugTextRenderer::draw(tr::GLContext& glContext, tr::BasicFramebuffer& target)
{
	if (!_shaderGlyphs.empty()) {
		if (lastRendererID() != ID) {
			setupContext(glContext);
			setLastRendererID(ID);
		}
		glContext.setFramebuffer(target);
		const auto targetSize{target.size()};

		if (_shaderGlyphBuffer.arrayCapacity() < _shaderGlyphs.size() * sizeof(ShaderGlyph)) {
			const auto newCapacity{std::bit_ceil(_shaderGlyphs.size() * sizeof(ShaderGlyph))};
			_shaderGlyphBuffer = tr::ShaderBuffer(0, newCapacity, tr::ShaderBuffer::Access::WRITE_ONLY);
#ifndef NDEBUG
			_shaderGlyphBuffer.setLabel("Glyph Buffer (tre Debug Text)");
#endif
		}
		_shaderGlyphBuffer.setArray(tr::asBytes(_shaderGlyphs));
		_shaderPipeline.vertexShader().setUniform(0, glm::ortho<float>(0, targetSize.x, targetSize.y, 0));
		_shaderPipeline.vertexShader().setStorageBuffer(0, _shaderGlyphBuffer);
		glContext.drawInstances(tr::Primitive::TRI_FAN, 0, 4, _shaderGlyphs.size());
	}
}

void tre::DebugTextRenderer::setupContext(tr::GLContext& glContext) noexcept
{
	glContext.useDepthTest(false);
	glContext.useScissorTest(false);
	glContext.useStencilTest(false);
	glContext.useFaceCulling(false);

	glContext.useBlending(true);
	glContext.setBlendingMode(tr::ALPHA_BLENDING);

	glContext.setShaderPipeline(_shaderPipeline);
	glContext.setVertexFormat(_vertexFormat);
	glContext.setVertexBuffer(_vertexBuffer, 0, sizeof(glm::u8vec2));
}