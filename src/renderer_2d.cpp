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

using namespace tr::angle_literals;
using namespace tr::matrix_operators;

namespace tre {
	inline constexpr glm::vec2  UNTEXTURED_UV{-100, -100};
	inline constexpr tr::RectI2 NO_SCISSOR_BOX{{-1, -1}, {-1, -1}};

	Renderer2D* _renderer2D{nullptr};
} // namespace tre

tre::Renderer2D::Renderer2D()
	: _shaderPipeline{{tr::asBytes(RENDERER_2D_VERT_SPV), tr::ShaderType::VERTEX},
					  {tr::asBytes(RENDERER_2D_FRAG_SPV), tr::ShaderType::FRAGMENT}}
	, _blendMode{tr::ALPHA_BLENDING}
	, _scissorBox{NO_SCISSOR_BOX}
{
	assert(!renderer2DActive());
	_renderer2D = this;

#ifndef NDEBUG
	_shaderPipeline.setLabel("tre::Renderer2D Pipeline");
	_shaderPipeline.vertexShader().setLabel("tre::Renderer2D Vertex Shader");
	_shaderPipeline.fragmentShader().setLabel("tre::Renderer2D Fragment Shader");
	_vertexBuffer.setLabel("tre::Renderer2D Vertex Buffer");
	_indexBuffer.setLabel("tre::Renderer2D Index Buffer");
#endif
	setFieldSize({1, 1});
}

tre::Renderer2D::~Renderer2D() noexcept
{
	_renderer2D = nullptr;
}

glm::vec2 tre::Renderer2D::fieldSize() const noexcept
{
	return _fieldSize;
}

void tre::Renderer2D::setFieldSize(glm::vec2 size) noexcept
{
	_fieldSize = size;
	_shaderPipeline.vertexShader().setUniform(0, glm::ortho(0.0f, _fieldSize.x, _fieldSize.y, 0.0f));
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
	TexturedQuad data;
	tr::fillRectVertices((data | tr::positions).begin(), rect.tl, rect.size);
	for (auto& vertex : data) {
		vertex.uv    = UNTEXTURED_UV;
		vertex.color = color;
	}

	_renderGraph[priority][std::nullopt].emplace_back(std::in_place_type<TexturedQuad>, data);
}

void tre::Renderer2D::addUntexturedRect(int priority, const tr::RectF2& rect, std::array<tr::RGBA8, 4> colors)
{
	TexturedQuad data;
	tr::fillRectVertices((data | tr::positions).begin(), rect.tl, rect.size);
	for (int i = 0; i < 4; ++i) {
		data[i].uv    = UNTEXTURED_UV;
		data[i].color = colors[i];
	}

	_renderGraph[priority][std::nullopt].emplace_back(std::in_place_type<TexturedQuad>, data);
}

void tre::Renderer2D::addTexturedRect(int priority, const tr::RectF2& rect, TextureRef texture, const tr::RectF2& uv,
									  tr::RGBA8 tint)
{
	TexturedQuad data;
	tr::fillRectVertices((data | tr::positions).begin(), rect.tl, rect.size);
	tr::fillRectVertices((data | tr::uvs).begin(), uv.tl, uv.size);
	std::ranges::fill(data | tr::colors, tint);

	_renderGraph[priority][texture].emplace_back(std::in_place_type<TexturedQuad>, data);
}

void tre::Renderer2D::addTexturedRect(int priority, const tr::RectF2& rect, TextureRef texture,
									  std::array<glm::vec2, 4> uvs, std::array<tr::RGBA8, 4> tints)
{
	TexturedQuad data;
	tr::fillRectVertices((data | tr::positions).begin(), rect.tl, rect.size);
	for (int i = 0; i < 4; i++) {
		data[i].uv    = uvs[i];
		data[i].color = tints[i];
	}

	_renderGraph[priority][texture].emplace_back(std::in_place_type<TexturedQuad>, data);
}

void tre::Renderer2D::addUntexturedRotatedRectangle(int priority, glm::vec2 pos, glm::vec2 posAnchor, glm::vec2 size,
													tr::AngleF rotation, tr::RGBA8 color)
{
	TexturedQuad data;
	tr::fillRectVertices((data | tr::positions).begin(), pos - posAnchor, size);
	if (rotation == 0_degf) {
		for (auto& vertex : data) {
			vertex.uv    = UNTEXTURED_UV;
			vertex.color = color;
		}
	}
	else {
		const auto transform{tr::rotateAroundPoint2(glm::mat4{1}, pos, rotation)};
		for (auto& vertex : data) {
			vertex.pos   = transform * vertex.pos;
			vertex.uv    = UNTEXTURED_UV;
			vertex.color = color;
		}
	}

	_renderGraph[priority][std::nullopt].emplace_back(std::in_place_type<TexturedQuad>, data);
}

void tre::Renderer2D::addUntexturedRotatedRectangle(int priority, glm::vec2 pos, glm::vec2 posAnchor, glm::vec2 size,
													tr::AngleF rotation, std::array<tr::RGBA8, 4> colors)
{
	TexturedQuad data;
	tr::fillRectVertices((data | tr::positions).begin(), pos - posAnchor, size);
	if (rotation == 0_degf) {
		for (int i = 0; i < 4; ++i) {
			data[i].uv    = UNTEXTURED_UV;
			data[i].color = colors[i];
		}
	}
	else {
		const auto transform{tr::rotateAroundPoint2(glm::mat4{1}, pos, rotation)};
		for (int i = 0; i < 4; ++i) {
			data[i].pos   = transform * data[i].pos;
			data[i].uv    = UNTEXTURED_UV;
			data[i].color = colors[i];
		}
	}

	_renderGraph[priority][std::nullopt].emplace_back(std::in_place_type<TexturedQuad>, data);
}

void tre::Renderer2D::addTexturedRotatedRectangle(int priority, glm::vec2 pos, glm::vec2 posAnchor, glm::vec2 size,
												  tr::AngleF rotation, TextureRef texture, const tr::RectF2& uv,
												  tr::RGBA8 tint)
{
	TexturedQuad data;
	tr::fillRectVertices((data | tr::positions).begin(), pos - posAnchor, size);
	tr::fillRectVertices((data | tr::uvs).begin(), uv.tl, uv.size);
	if (rotation == 0_degf) {
		std::ranges::fill(data | tr::colors, tint);
	}
	else {
		const auto transform{tr::rotateAroundPoint2(glm::mat4{1}, pos, rotation)};
		for (int i = 0; i < 4; ++i) {
			data[i].pos   = transform * data[i].pos;
			data[i].color = tint;
		}
	}

	_renderGraph[priority][texture].emplace_back(std::in_place_type<TexturedQuad>, data);
}

void tre::Renderer2D::addTexturedRotatedRectangle(int priority, glm::vec2 pos, glm::vec2 posAnchor, glm::vec2 size,
												  tr::AngleF rotation, TextureRef texture, std::array<glm::vec2, 4> uvs,
												  std::array<tr::RGBA8, 4> tints)
{
	TexturedQuad data;
	tr::fillRectVertices((data | tr::positions).begin(), pos - posAnchor, size);
	if (rotation == 0_degf) {
		for (int i = 0; i < 4; ++i) {
			data[i].uv    = uvs[i];
			data[i].color = tints[i];
		}
	}
	else {
		const auto transform{tr::rotateAroundPoint2(glm::mat4{1}, pos, rotation)};
		for (int i = 0; i < 4; ++i) {
			data[i].pos   = transform * data[i].pos;
			data[i].uv    = uvs[i];
			data[i].color = tints[i];
		}
	}

	_renderGraph[priority][texture].emplace_back(std::in_place_type<TexturedQuad>, data);
}

void tre::Renderer2D::addUntexturedTriangle(int priority, std::span<tr::ClrVtx2, 3> triangle)
{
	_renderGraph[priority][std::nullopt].emplace_back(
		std::in_place_type<TexturedTriangle>, TexturedTriangle{{{triangle[0].pos, UNTEXTURED_UV, triangle[0].color},
																{triangle[1].pos, UNTEXTURED_UV, triangle[1].color},
																{triangle[2].pos, UNTEXTURED_UV, triangle[2].color}}});
}

void tre::Renderer2D::addTexturedTriangle(int priority, TexturedTriangle triangle, TextureRef texture)
{
	_renderGraph[priority][std::nullopt].emplace_back(std::in_place_type<TexturedTriangle>, triangle);
}

void tre::Renderer2D::addUntexturedQuad(int priority, std::span<tr::ClrVtx2, 4> quad)
{
	_renderGraph[priority][std::nullopt].emplace_back(std::in_place_type<TexturedQuad>,
													  TexturedQuad{{{quad[0].pos, UNTEXTURED_UV, quad[0].color},
																	{quad[1].pos, UNTEXTURED_UV, quad[1].color},
																	{quad[2].pos, UNTEXTURED_UV, quad[2].color},
																	{quad[3].pos, UNTEXTURED_UV, quad[3].color}}});
}

void tre::Renderer2D::addTexturedQuad(int priority, TexturedQuad quad, TextureRef texture)
{
	_renderGraph[priority][std::nullopt].emplace_back(std::in_place_type<TexturedQuad>, quad);
}

void tre::Renderer2D::addUntexturedPolygon(int priority, const tr::CircleF& circle, int vertexCount,
										   tr::AngleF rotation, tr::RGBA8 color)
{
	TexturedVertexFan data(vertexCount);
	tr::fillPolygonVertices((data | tr::positions).begin(), vertexCount, circle, rotation);
	for (auto& vertex : data) {
		vertex.uv    = UNTEXTURED_UV;
		vertex.color = color;
	}

	_renderGraph[priority][std::nullopt].emplace_back(std::in_place_type<TexturedVertexFan>, std::move(data));
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

	TexturedVertexFan data(vertices.size());
	for (std::size_t i = 0; i < data.size(); ++i) {
		data[i].pos   = vertices[i].pos;
		data[i].uv    = UNTEXTURED_UV;
		data[i].color = vertices[i].color;
	}

	_renderGraph[priority][std::nullopt].emplace_back(std::in_place_type<TexturedVertexFan>, std::move(data));
}

void tre::Renderer2D::addTexturedPolygonFan(int priority, std::vector<Vertex> vertices, TextureRef texture)
{
	if (vertices.size() < 3) {
		return;
	}
	_renderGraph[priority][texture].emplace_back(std::in_place_type<TexturedVertexFan>, std::move(vertices));
}

void tre::Renderer2D::setupContext() noexcept
{
	auto& context{tr::window().glContext()};

	context.useDepthTest(false);
	context.useFaceCulling(false);

	context.useBlending(true);
	context.setBlendingMode(_blendMode);

	if (_scissorBox != NO_SCISSOR_BOX) {
		context.useScissorTest(true);
		context.setScissorBox(_scissorBox);
	}
	else {
		context.useScissorTest(false);
	}

	context.useStencilTest(false);

	context.setShaderPipeline(_shaderPipeline);
	context.setVertexFormat(tr::TintVtx2::vertexFormat());
}

void tre::Renderer2D::writeToVertexIndexVectors(const Primitive& primitive, std::uint16_t& index)
{
	const auto triangle{[&](const TexturedTriangle& triangle) {
		_vertices.insert(_vertices.end(), triangle.begin(), triangle.end());
		_indices.insert(_indices.end(), {index++, index++, index++});
	}};
	const auto rectangle{[&](const TexturedQuad& rectangle) {
		_vertices.insert(_vertices.end(), rectangle.begin(), rectangle.end());
		tr::fillPolygonIndices(back_inserter(_indices), 4, index);
		index += 4;
	}};
	const auto fan{[&](const TexturedVertexFan& fan) {
		_vertices.insert(_vertices.end(), fan.begin(), fan.end());
		tr::fillPolygonIndices(back_inserter(_indices), fan.size(), index);
		index += fan.size();
	}};
	const auto data{[&](const RawData& data) {
		const auto offset{std::views::transform([&](auto idx) { return idx + index; })};

		_vertices.insert(_vertices.end(), data.first.begin(), data.first.end());
		std::ranges::copy(data.second | offset, back_inserter(_indices));
		index += data.first.size();
	}};

	std::visit(tr::Overloaded{triangle, rectangle, fan, data}, primitive);
}

void tre::Renderer2D::drawUpToPriority(int maxPriority, tr::BasicFramebuffer& target)
{
	auto& glContext{tr::window().glContext()};

	if (lastRendererID() != ID) {
		setupContext();
		setLastRendererID(ID);
	}
	glContext.setFramebuffer(target);

	const std::ranges::subrange range{_renderGraph.begin(), _renderGraph.lower_bound(maxPriority)};
	for (auto& priority : range | std::views::values) {
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
	_renderGraph.erase(range.begin(), range.end());
}

void tre::Renderer2D::draw(tr::BasicFramebuffer& target)
{
	drawUpToPriority(std::numeric_limits<int>::max(), target);
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

bool tre::renderer2DActive() noexcept
{
	return _renderer2D != nullptr;
}

tre::Renderer2D& tre::renderer2D() noexcept
{
	assert(renderer2DActive());
	return *_renderer2D;
}