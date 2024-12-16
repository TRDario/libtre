#include "../include/tre/renderer_2d.hpp"

#include "../include/tre/renderer_base.hpp"
#include "../resources/renderer_2d.frag.spv.hpp"
#include "../resources/renderer_2d.vert.spv.hpp"

#include <boost/container_hash/hash.hpp>
#include <cstddef>
#include <ranges>

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

void tre::Renderer2D::addRectOutline(int priority, const tr::RectF2& rect, float thickness, tr::RGBA8 color)
{
	PolygonOutline data(8);
	tr::fillRectOutlineVertices((data | tr::positions).begin(), rect.tl, rect.size, thickness);
	for (int i = 0; i < 4; ++i) {
		data[i].uv    = UNTEXTURED_UV;
		data[i].color = color;
	}

	_renderGraph[priority][std::nullopt].emplace_back(std::in_place_type<PolygonOutline>, std::move(data));
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

void tre::Renderer2D::addUntexturedRotatedRectangle(int priority, glm::vec2 pos, glm::vec2 posAnchor, glm::vec2 size,
													tr::AngleF rotation, tr::RGBA8 color)
{
	TexturedQuad data;
	if (rotation == 0_degf) {
		tr::fillRectVertices((data | tr::positions).begin(), pos - posAnchor, size);
	}
	else {
		const auto transform{tr::rotateAroundPoint2(glm::mat4{1}, pos, rotation)};
		tr::fillRectVertices((data | tr::positions).begin(), pos - posAnchor, size, transform);
	}
	for (auto& vertex : data) {
		vertex.uv    = UNTEXTURED_UV;
		vertex.color = color;
	}

	_renderGraph[priority][std::nullopt].emplace_back(std::in_place_type<TexturedQuad>, data);
}

void tre::Renderer2D::addRotatedRectangleOutline(int priority, glm::vec2 pos, glm::vec2 posAnchor, glm::vec2 size,
												 tr::AngleF rotation, float thickness, tr::RGBA8 color)
{
	PolygonOutline data(8);
	if (rotation == 0_degf) {
		tr::fillRectOutlineVertices((data | tr::positions).begin(), pos - posAnchor, size, thickness);
	}
	else {
		const auto transform{tr::rotateAroundPoint2(glm::mat4{1}, pos, rotation)};
		tr::fillRectOutlineVertices((data | tr::positions).begin(), pos - posAnchor, size, thickness, transform);
	}
	for (auto& vertex : data) {
		vertex.uv    = UNTEXTURED_UV;
		vertex.color = color;
	}

	_renderGraph[priority][std::nullopt].emplace_back(std::in_place_type<PolygonOutline>, std::move(data));
}

void tre::Renderer2D::addTexturedRotatedRectangle(int priority, glm::vec2 pos, glm::vec2 posAnchor, glm::vec2 size,
												  tr::AngleF rotation, TextureRef texture, const tr::RectF2& uv,
												  tr::RGBA8 tint)
{
	TexturedQuad data;
	if (rotation == 0_degf) {
		tr::fillRectVertices((data | tr::positions).begin(), pos - posAnchor, size);
	}
	else {
		const auto transform{tr::rotateAroundPoint2(glm::mat4{1}, pos, rotation)};
		tr::fillRectVertices((data | tr::positions).begin(), pos - posAnchor, size, transform);
	}
	tr::fillRectVertices((data | tr::uvs).begin(), uv.tl, uv.size);
	std::ranges::fill(data | tr::colors, tint);

	_renderGraph[priority][texture].emplace_back(std::in_place_type<TexturedQuad>, data);
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
	_renderGraph[priority][texture].emplace_back(std::in_place_type<TexturedQuad>, quad);
}

void tre::Renderer2D::addUntexturedRegularPolygon(int priority, const tr::CircleF& circle, int vertexCount,
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

void tre::Renderer2D::addRegularPolygonOutline(int priority, const tr::CircleF& circle, int vertexCount,
											   tr::AngleF rotation, float thickness, tr::RGBA8 color)
{
	PolygonOutline data(vertexCount * 2);
	tr::fillPolygonOutlineVertices((data | tr::positions).begin(), vertexCount, circle, rotation, thickness);
	for (auto& vertex : data) {
		vertex.uv    = UNTEXTURED_UV;
		vertex.color = color;
	}

	_renderGraph[priority][std::nullopt].emplace_back(std::in_place_type<PolygonOutline>, std::move(data));
}

void tre::Renderer2D::addUntexturedCircle(int priority, const tr::CircleF& circle, tr::RGBA8 color)
{
	addUntexturedRegularPolygon(priority, circle, tr::smoothPolygonVerticesCount(circle.r), tr::rads(0), color);
}

void tre::Renderer2D::addCircleOutline(int priority, const tr::CircleF& circle, float thickness, tr::RGBA8 color)
{
	addRegularPolygonOutline(priority, circle, tr::smoothPolygonVerticesCount(circle.r), tr::rads(0), thickness, color);
}

void tre::Renderer2D::addUntexturedPolygon(int priority, std::span<glm::vec2> vertices, tr::RGBA8 color)
{
	if (vertices.size() < 3) {
		return;
	}

	TexturedVertexFan data(vertices.size());
	for (std::size_t i = 0; i < data.size(); ++i) {
		data[i].pos   = vertices[i];
		data[i].uv    = UNTEXTURED_UV;
		data[i].color = color;
	}

	_renderGraph[priority][std::nullopt].emplace_back(std::in_place_type<TexturedVertexFan>, std::move(data));
}

void tre::Renderer2D::addUntexturedPolygon(int priority, std::span<tr::ClrVtx2> vertices)
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

void tre::Renderer2D::addTexturedPolygon(int priority, std::vector<Vertex> vertices, TextureRef texture)
{
	if (vertices.size() < 3) {
		return;
	}
	_renderGraph[priority][texture].emplace_back(std::in_place_type<TexturedVertexFan>, std::move(vertices));
}

void tre::Renderer2D::setupContext() noexcept
{
	auto& graphics{tr::window().graphics()};

	graphics.useDepthTest(false);
	graphics.useFaceCulling(false);

	graphics.useBlending(true);
	graphics.setBlendingMode(_blendMode);

	if (_scissorBox != NO_SCISSOR_BOX) {
		graphics.useScissorTest(true);
		graphics.setScissorBox(_scissorBox);
	}
	else {
		graphics.useScissorTest(false);
	}

	graphics.useStencilTest(false);

	graphics.setShaderPipeline(_shaderPipeline);
	graphics.setVertexFormat(tr::TintVtx2::vertexFormat());
}

void tre::Renderer2D::writeToVertexIndexVectors(const Primitive& primitive, std::uint16_t& index)
{
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
	const auto outline{[&](const PolygonOutline& outline) {
		_vertices.insert(_vertices.end(), outline.begin(), outline.end());
		tr::fillPolygonOutlineIndices(back_inserter(_indices), outline.size() / 2, index);
		index += outline.size();
	}};
	const auto data{[&](const RawData& data) {
		const auto offset{std::views::transform([&](auto idx) { return idx + index; })};

		_vertices.insert(_vertices.end(), data.first.begin(), data.first.end());
		std::ranges::copy(data.second | offset, back_inserter(_indices));
		index += data.first.size();
	}};

	std::visit(tr::Overloaded{rectangle, fan, outline, data}, primitive);
}

void tre::Renderer2D::drawUpToPriority(int maxPriority, tr::BasicFramebuffer& target)
{
	auto& graphics{tr::window().graphics()};

	if (lastRendererID() != ID) {
		setupContext();
		setLastRendererID(ID);
	}
	graphics.setFramebuffer(target);

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
			graphics.setVertexBuffer(_vertexBuffer, 0, sizeof(tr::TintVtx2));
			graphics.setIndexBuffer(_indexBuffer);
			graphics.drawIndexed(tr::Primitive::TRIS, 0, _indexBuffer.size());
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