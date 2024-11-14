#include "../include/tre/renderer_2d.hpp"

#include "../include/tre/renderer_base.hpp"
#include "../resources/renderer_2d.frag.spv.hpp"
#include "../resources/renderer_2d.vert.spv.hpp"

#include <boost/container_hash/hash.hpp>
#include <cstddef>
#include <ranges>
#include <tr/draw_geometry.hpp>
#include <tr/geometry.hpp>
#include <tr/gl_context.hpp>
#include <tr/index_buffer.hpp>
#include <tr/shader.hpp>
#include <tr/texture.hpp>
#include <tr/vertex.hpp>
#include <tr/vertex_buffer.hpp>

using namespace tr::matrix_operators;

namespace tre {
	inline constexpr glm::vec2 UNTEXTURED_UV{-100, -100};
	inline constexpr tr::RectI2 NO_SCISSOR_BOX{{-1, -1}, {-1, -1}};
} // namespace tre

tre::Renderer2D::Renderer2D()
	: _vertexShader{{(const std::byte*)(RENDERER_2D_VERT_SPV), RENDERER_2D_VERT_SPV_len}, tr::ShaderType::VERTEX},
	  _fragmentShader{{(const std::byte*)(RENDERER_2D_FRAG_SPV), RENDERER_2D_FRAG_SPV_len}, tr::ShaderType::FRAGMENT},
	  _shaderPipeline{_vertexShader, _fragmentShader},
	  _blendMode{tr::ALPHA_BLENDING},
	  _scissorBox{NO_SCISSOR_BOX}
{
	setFieldSize({1, 1});
}

void tre::Renderer2D::setFieldSize(glm::vec2 size) noexcept
{
	_fieldSize = size;
	_vertexShader.setUniform(0, _fieldSize);
}

void tre::Renderer2D::setBlendingMode(tr::BlendMode blendMode) noexcept
{
	_blendMode = blendMode;
	if (lastRendererID() == ID) {
		setLastRendererID(NO_RENDERER);
	}
}

void tre::Renderer2D::setScissorBox(std::optional<tr::RectI2> scissorBox) noexcept
{
	_scissorBox = scissorBox.value_or(NO_SCISSOR_BOX);
	if (lastRendererID() == ID) {
		setLastRendererID(NO_RENDERER);
	}
}

void tre::Renderer2D::addUntexturedRect(int priority, const tr::RectF2& rect, tr::RGBA8 color)
{
	Rectangle data;
	tr::fillRectVertices((data | tr::positions).begin(), rect.tl, rect.size);
	for (auto& vertex : data) {
		vertex.uv = UNTEXTURED_UV;
		vertex.color = color;
	}

	_renderGraph[priority][std::nullopt].emplace_back(data);
}

void tre::Renderer2D::addUntexturedRect(int priority, const tr::RectF2& rect, std::array<tr::RGBA8, 4> colors)
{
	Rectangle data;
	tr::fillRectVertices((data | tr::positions).begin(), rect.tl, rect.size);
	for (int i = 0; i < 4; ++i) {
		data[i].uv = UNTEXTURED_UV;
		data[i].color = colors[i];
	}

	_renderGraph[priority][std::nullopt].emplace_back(data);
}

void tre::Renderer2D::addTexturedRect(int priority, const tr::RectF2& rect, TextureRef texture, const tr::RectF2& uv,
									  tr::RGBA8 tint)
{
	Rectangle data;
	tr::fillRectVertices((data | tr::positions).begin(), rect.tl, rect.size);
	tr::fillRectVertices((data | tr::uvs).begin(), uv.tl, uv.size);
	std::ranges::fill(data | tr::colors, tint);

	_renderGraph[priority][texture].emplace_back(data);
}

void tre::Renderer2D::addTexturedRect(int priority, const tr::RectF2& rect, TextureRef texture,
									  std::array<glm::vec2, 4> uvs, std::array<tr::RGBA8, 4> tints)
{
	Rectangle data;
	tr::fillRectVertices((data | tr::positions).begin(), rect.tl, rect.size);
	for (int i = 0; i < 4; i++) {
		data[i].uv = uvs[i];
		data[i].color = tints[i];
	}

	_renderGraph[priority][texture].emplace_back(data);
}

void tre::Renderer2D::addUntexturedRotatedRectangle(int priority, glm::vec2 pos, glm::vec2 posAnchor, glm::vec2 size,
													tr::AngleF rotation, tr::RGBA8 color)
{
	Rectangle data;
	const auto transform{tr::rotateAroundPoint2(glm::mat4{1}, pos, rotation)};
	tr::fillRectVertices((data | tr::positions).begin(), pos - posAnchor, size);
	for (auto& vertex : data) {
		vertex.pos = transform * vertex.pos;
		vertex.uv = UNTEXTURED_UV;
		vertex.color = color;
	}

	_renderGraph[priority][std::nullopt].emplace_back(data);
}

void tre::Renderer2D::addUntexturedRotatedRectangle(int priority, glm::vec2 pos, glm::vec2 posAnchor, glm::vec2 size,
													tr::AngleF rotation, std::array<tr::RGBA8, 4> colors)
{
	Rectangle data;
	const auto transform{tr::rotateAroundPoint2(glm::mat4{1}, pos, rotation)};
	tr::fillRectVertices((data | tr::positions).begin(), pos - posAnchor, size);
	for (int i = 0; i < 4; ++i) {
		data[i].pos = transform * data[i].pos;
		data[i].uv = UNTEXTURED_UV;
		data[i].color = colors[i];
	}

	_renderGraph[priority][std::nullopt].emplace_back(data);
}

void tre::Renderer2D::addTexturedRotatedRectangle(int priority, glm::vec2 pos, glm::vec2 posAnchor, glm::vec2 size,
												  tr::AngleF rotation, TextureRef texture, const tr::RectF2& uv,
												  tr::RGBA8 tint)
{
	Rectangle data;
	const auto transform{tr::rotateAroundPoint2(glm::mat4{1}, pos, rotation)};
	tr::fillRectVertices((data | tr::positions).begin(), pos - posAnchor, size);
	tr::fillRectVertices((data | tr::uvs).begin(), uv.tl, uv.size);
	for (int i = 0; i < 4; ++i) {
		data[i].pos = transform * data[i].pos;
		data[i].color = tint;
	}

	_renderGraph[priority][texture].emplace_back(data);
}

void tre::Renderer2D::addTexturedRotatedRectangle(int priority, glm::vec2 pos, glm::vec2 posAnchor, glm::vec2 size,
												  tr::AngleF rotation, TextureRef texture, std::array<glm::vec2, 4> uvs,
												  std::array<tr::RGBA8, 4> tints)
{
	Rectangle data;
	const auto transform{tr::rotateAroundPoint2(glm::mat4{1}, pos, rotation)};
	tr::fillRectVertices((data | tr::positions).begin(), pos - posAnchor, size);
	for (int i = 0; i < 4; ++i) {
		data[i].pos = transform * data[i].pos;
		data[i].uv = uvs[i];
		data[i].color = tints[i];
	}

	_renderGraph[priority][texture].emplace_back(data);
}

void tre::Renderer2D::addUntexturedPolygon(int priority, const tr::CircleF& circle, int vertexCount,
										   tr::AngleF rotation, tr::RGBA8 color)
{
	VertexFan data(vertexCount);
	tr::fillPolygonVertices((data | tr::positions).begin(), vertexCount, circle, rotation);
	for (auto& vertex : data) {
		vertex.uv = UNTEXTURED_UV;
		vertex.color = color;
	}

	_renderGraph[priority][std::nullopt].emplace_back(std::move(data));
}

void tre::Renderer2D::addUntexturedCircle(int priority, const tr::CircleF& circle, tr::RGBA8 color)
{
	addUntexturedPolygon(priority, circle, tr::smoothPolygonVerticesCount(circle.r), tr::rads(0), color);
}

void tre::Renderer2D::addUntexturedPolygonFan(int priority, std::span<tr::ClrVtx2> vertices)
{
	if (vertices.size() < 3) {
		return;
	}

	VertexFan data(vertices.size());
	for (std::size_t i = 0; i < data.size(); ++i) {
		data[i].pos = vertices[i].pos;
		data[i].uv = UNTEXTURED_UV;
		data[i].color = vertices[i].color;
	}

	_renderGraph[priority][std::nullopt].emplace_back(std::move(data));
}

void tre::Renderer2D::addTexturedPolygonFan(int priority, std::vector<Vertex> vertices, TextureRef texture)
{
	if (vertices.size() < 3) {
		return;
	}

	_renderGraph[priority][texture].emplace_back(std::move(vertices));
}

void tre::Renderer2D::setupContext(tr::GLContext& glContext) noexcept
{
	glContext.useDepthTest(false);
	glContext.useFaceCulling(false);

	glContext.useBlending(true);
	glContext.setBlendingMode(_blendMode);

	if (_scissorBox != NO_SCISSOR_BOX) {
		glContext.useScissorTest(true);
		glContext.setScissorBox(_scissorBox);
	}
	else {
		glContext.useScissorTest(false);
	}

	glContext.useStencilTest(false);

	glContext.setShaderPipeline(_shaderPipeline);
	glContext.setVertexFormat(tr::TintVtx2::vertexFormat());
}

void tre::Renderer2D::writeToVertexIndexVectors(const Primitive& primitive, std::uint16_t& index)
{
	std::visit(tr::overloaded{[&](const Triangle& triangle) {
								  _vertices.insert(_vertices.end(), triangle.begin(), triangle.end());
								  _indices.insert(_indices.end(), {index++, index++, index++});
							  },
							  [&](const Rectangle& rectangle) {
								  _vertices.insert(_vertices.end(), rectangle.begin(), rectangle.end());
								  tr::fillPolygonIndices(back_inserter(_indices), 4, index);
								  index += 4;
							  },
							  [&](const VertexFan& fan) {
								  _vertices.insert(_vertices.end(), fan.begin(), fan.end());
								  tr::fillPolygonIndices(back_inserter(_indices), fan.size(), index);
								  index += fan.size();
							  },
							  [&](const RawData& data) {
								  const auto offset{std::views::transform([&](auto idx) { return idx + index; })};
								  _vertices.insert(_vertices.end(), data.first.begin(), data.first.end());
								  std::ranges::copy(data.second | offset, back_inserter(_indices));
								  index += data.first.size();
							  }},
			   primitive);
}

void tre::Renderer2D::drawUpToPriority(tr::GLContext& glContext, tr::BasicFramebuffer& target, int minPriority)
{
	if (lastRendererID() != ID) {
		setupContext(glContext);
	}
	glContext.setFramebuffer(target);

	const auto end{std::ranges::find_if(_renderGraph, [=](auto& v) { return v.first < minPriority; })};
	for (auto& priority : std::ranges::subrange{_renderGraph.begin(), end} | std::views::values) {
		for (auto& [texture, primitives] : priority) {
			_vertices.clear();
			_indices.clear();
			std::uint16_t index{0};

			for (auto& primitive : primitives) {
				writeToVertexIndexVectors(primitive, index);
			}

			if (texture.has_value()) {
				_textureUnit.setTexture(texture->first);
				_textureUnit.setSampler(texture->second);
			}
			_vertexBuffer.set(tr::rangeBytes(_vertices));
			_indexBuffer.set(_indices);
			glContext.setVertexBuffer(_vertexBuffer, 0, sizeof(tr::TintVtx2));
			glContext.setIndexBuffer(_indexBuffer);
			glContext.drawIndexed(tr::Primitive::TRIS, 0, _indexBuffer.size());
		}
	}
	_renderGraph.erase(_renderGraph.begin(), end);
	setLastRendererID(ID);
}

void tre::Renderer2D::draw(tr::GLContext& glContext, tr::BasicFramebuffer& target)
{
	drawUpToPriority(glContext, target, std::numeric_limits<int>::min());
}

std::size_t tre::Renderer2D::TextureRefHash::operator()(const std::optional<TextureRef>& texture) const noexcept
{
	if (!texture.has_value()) {
		return std::hash<std::nullptr_t>{}(nullptr);
	}
	else {
		std::size_t result{0};
		result ^= std::hash<tr::Texture2D>{}(texture->first) + 0x9e'37'79'b9 + (result << 6) + (result >> 2);
		result ^= std::hash<tr::Sampler>{}(texture->second) + 0x9e'37'79'b9 + (result << 6) + (result >> 2);
		return result;
	}
}
