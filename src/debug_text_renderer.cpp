#include "../include/tre/debug_text_renderer.hpp"
#include "../include/tre/renderer_base.hpp"

#include <GL/gl.h>

namespace tre {
#include "../resources/debug_text.frag.spv.hpp"
#include "../resources/debug_text.vert.spv.hpp"
#include "../resources/debug_text_font.bmp.hpp"

	constexpr std::array<glm::u8vec2, 4> GLYPH_VERTICES{{{0, 0}, {0, 1}, {1, 1}, {1, 0}}};
} // namespace tre

using VtxAttrF = tr::VertexAttributeF;

tre::DebugTextRenderer::DebugTextRenderer()
	: _shaderPipeline{{tr::asBytes(DEBUG_TEXT_VERT_SPV), tr::ShaderType::VERTEX},
					  {tr::asBytes(DEBUG_TEXT_FRAG_SPV), tr::ShaderType::FRAGMENT}},
	  _shaderGlyphBuffer{0, 256 * sizeof(ShaderGlyph), tr::ShaderBuffer::Access::WRITE_ONLY},
	  _font{tr::Bitmap{tr::asBytes(DEBUG_TEXT_FONT_BMP)}, tr::NO_MIPMAPS, tr::TextureFormat::R8},
	  _vertexBuffer{tr::asBytes(GLYPH_VERTICES)},
	  _vertexFormat{std::initializer_list<tr::VertexAttribute>{{VtxAttrF{VtxAttrF::Type::UI8, 2, false, 0}}}},
	  _columnLimit{255},
	  _leftLine{0},
	  _rightLine{0}
{
	assert(glGetError() != GL_INVALID_OPERATION);
	_sampler.setMagFilter(tr::MagFilter::NEAREST);
	assert(glGetError() != GL_INVALID_OPERATION);
	_sampler.setMinFilter(tr::MinFilter::LINEAR);
	assert(glGetError() != GL_INVALID_OPERATION);
	_textureUnit.setSampler(_sampler);
	assert(glGetError() != GL_INVALID_OPERATION);
	_textureUnit.setTexture(_font);
	assert(glGetError() != GL_INVALID_OPERATION);
	_shaderPipeline.fragmentShader().setUniform(2, _textureUnit);
	assert(glGetError() != GL_INVALID_OPERATION);
	setScale(1.0f);

#ifndef NDEBUG
	_shaderPipeline.setLabel("tre::DebugTextRenderer Pipeline");
	_shaderPipeline.vertexShader().setLabel("tre::DebugTextRenderer Vertex Shader");
	_shaderPipeline.fragmentShader().setLabel("tre::DebugTextRenderer Fragment Shader");
	_shaderGlyphBuffer.setLabel("tre::DebugTextRenderer Shader Glyph Buffer");
	_font.setLabel("tre::DebugTextRenderer Font Texture");
	_sampler.setLabel("tre::DebugTextRenderer Sampler");
	_vertexBuffer.setLabel("tre::DebugTextRenderer Vertex Buffer");
	_vertexFormat.setLabel("tre::DebugTextRenderer Vertex Format");
#endif
}

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
	_rightLine = 0;
}

void tre::DebugTextRenderer::rightAlignLine(std::size_t begin, std::size_t end) noexcept
{
	for (auto i = begin; i < end; ++i) {
		_shaderGlyphs[i].pos.x = end - i;
	}
}

void tre::DebugTextRenderer::trimTrailingWhitespace(DebugTextContext& context) noexcept
{
	auto lineEnd = context.wordStart - 1;
	while (lineEnd > context.lineStart && _shaderGlyphs[lineEnd].chr != ' ') {
		--lineEnd;
	}

	if (lineEnd != context.wordStart) {
		_shaderGlyphs.erase(_shaderGlyphs.begin() + lineEnd, _shaderGlyphs.begin() + context.wordStart);
		context.wordStart = lineEnd;
	}
}

void tre::DebugTextRenderer::moveCurrentWordToNextLine(DebugTextContext& context) noexcept
{
	for (auto it = _shaderGlyphs.begin() + context.wordStart; it != _shaderGlyphs.end(); ++it) {
		it->pos = {context.lineLength++, context.line};
	}
	context.lineStart = context.wordStart;
}

void tre::DebugTextRenderer::breakAtLastWhitespace(DebugTextContext& context) noexcept
{
	trimTrailingWhitespace(context);
	if (context.alignment == Align::RIGHT) {
		rightAlignLine(context.lineStart, context.wordStart);
	}
	moveCurrentWordToNextLine(context);
}

void tre::DebugTextRenderer::breakOverlongWord(DebugTextContext& context) noexcept
{
	if (context.alignment == Align::RIGHT) {
		rightAlignLine(context.lineStart, context.lineStart + _columnLimit);
	}
	_shaderGlyphs.back().pos = {context.lineLength++, context.line};

	context.lineStart += _columnLimit;
	context.wordStart = context.lineStart;
}

void tre::DebugTextRenderer::handleColumnLimit(DebugTextContext& context) noexcept
{
	context.lineLength = 0;
	++context.line;

	if (context.wordStart > context.lineStart) {
		breakAtLastWhitespace(context);
	}
	else {
		breakOverlongWord(context);
	}
}

void tre::DebugTextRenderer::writeCharacter(char chr, DebugTextContext& context)
{
	if (context.lineLength == _columnLimit && chr == ' ') {
		handleNewline(context);
	}
	else {
		_shaderGlyphs.push_back({{context.lineLength, context.line},
								 context.alignment == Align::RIGHT,
								 chr,
								 context.textColor,
								 context.backgroundColor});

		if (_shaderGlyphs.size() - context.textStart > 1) {
			const auto prev{_shaderGlyphs[_shaderGlyphs.size() - 2].chr};
			if (prev == ' ' && chr != ' ') {
				context.wordStart = _shaderGlyphs.size() - 1;
			}
		}

		if (++context.lineLength > _columnLimit) {
			handleColumnLimit(context);
		}
	}
}

void tre::DebugTextRenderer::handleNewline(DebugTextContext& context)
{
	if (context.alignment == Align::RIGHT) {
		rightAlignLine(context.lineStart, _shaderGlyphs.size());
	}

	context.lineLength = 0;
	context.wordStart = _shaderGlyphs.size();
	context.lineStart = _shaderGlyphs.size();
	++context.line;
}

void tre::DebugTextRenderer::handleControlSequence(std::string_view::iterator& it, std::string_view::iterator end,
												   DebugTextContext& context, tr::RGBA8 textColor,
												   tr::RGBA8 backgroundColor, std::span<tr::RGBA8> extraColors)
{
	switch (*it) {
	case 'b':
		if (std::next(it) != end && std::isdigit(*++it) && *it - '0' < extraColors.size()) {
			context.backgroundColor = extraColors[*it - '0'];
		}
		break;
	case 'B':
		context.backgroundColor = backgroundColor;
		break;
	case 'c':
		if (std::next(it) != end && std::isdigit(*++it) && *it - '0' < extraColors.size()) {
			context.textColor = extraColors[*it - '0'];
		}
		break;
	case 'C':
		context.textColor = textColor;
		break;
	case 'n':
		handleNewline(context);
		break;
	case '\\':
		writeCharacter('\\', context);
		break;
	default:
		break;
	}
}

void tre::DebugTextRenderer::write(std::string_view text, tr::RGBA8 textColor, tr::RGBA8 backgroundColor,
								   std::span<tr::RGBA8> extraColors, Align alignment)
{
	auto& line{alignment == Align::RIGHT ? _rightLine : _leftLine};
	const auto oldSize{_shaderGlyphs.size()};
	DebugTextContext context{line, alignment, textColor, backgroundColor, 0, oldSize, oldSize, oldSize};

	for (auto it = text.begin(); it != text.end(); ++it) {
		if (*it == '\\') {
			if (std::next(it) != text.end()) {
				handleControlSequence(++it, text.end(), context, textColor, backgroundColor, extraColors);
			}
		}
		else {
			writeCharacter(*it, context);
		}
	}
	handleNewline(context);
}

void tre::DebugTextRenderer::draw(tr::GLContext& glContext, tr::BasicFramebuffer& target)
{
	if (!_shaderGlyphs.empty()) {
		if (lastRendererID() != ID) {
			setupContext(glContext);
			setLastRendererID(ID);
		}
		glContext.setFramebuffer(target);

		if (_shaderGlyphBuffer.arrayCapacity() < _shaderGlyphs.size() * sizeof(ShaderGlyph)) {
			const auto newCapacity{std::bit_ceil(_shaderGlyphs.size() * sizeof(ShaderGlyph))};
			_shaderGlyphBuffer = tr::ShaderBuffer(0, newCapacity, tr::ShaderBuffer::Access::WRITE_ONLY);
#ifndef NDEBUG
			_shaderGlyphBuffer.setLabel("Glyph Buffer (tre Debug Text)");
#endif
		}
		_shaderGlyphBuffer.setArray(tr::rangeBytes(_shaderGlyphs));
		_shaderPipeline.vertexShader().setUniform(0, glm::vec2(target.viewport().size));
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