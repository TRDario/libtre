#include "../include/tre/renderer_2d.hpp"
#include "../resources/renderer_2d.frag.spv.hpp"
#include "../resources/renderer_2d.vert.spv.hpp"

namespace tre {
	inline constexpr glm::vec2 UNTEXTURED_UV{-100, -100};
	tre::Renderer2D*           _renderer2D{nullptr};
} // namespace tre

tre::Renderer2D::Renderer2D()
	: _shaderPipeline{tr::loadEmbeddedShader(RENDERER_2D_VERT_SPV, tr::ShaderType::VERTEX),
					  tr::loadEmbeddedShader(RENDERER_2D_FRAG_SPV, tr::ShaderType::FRAGMENT)}
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
}

tre::Renderer2D::Renderer2D(Renderer2D&& r) noexcept
	: _shaderPipeline{std::move(r._shaderPipeline)}
	, _textureUnit{std::move(r._textureUnit)}
	, _vertexBuffer{std::move(r._vertexBuffer)}
	, _indexBuffer{std::move(r._indexBuffer)}
	, _vertices{std::move(r._vertices)}
	, _indices{std::move(r._indices)}
	, _layers{std::move(r._layers)}
{
	if (_renderer2D == &r) {
		_renderer2D = this;
	}
}

tre::Renderer2D::~Renderer2D() noexcept
{
	if (_renderer2D == this) {
		_renderer2D = nullptr;
	}
}

void tre::Renderer2D::addColorOnlyLayer(int priority, const glm::mat4& transform, const tr::BlendMode& blendMode)
{
	assert(!_layers.contains(priority));
	_layers.emplace(std::piecewise_construct, std::forward_as_tuple(priority),
					std::forward_as_tuple(nullptr, nullptr, transform, blendMode));
}

void tre::Renderer2D::addLayer(int priority, const tr::Texture2D& texture, const tr::Sampler& sampler,
							   const glm::mat4& transform, const tr::BlendMode& blendMode)
{
	assert(!_layers.contains(priority));
	_layers.emplace(std::piecewise_construct, std::forward_as_tuple(priority),
					std::forward_as_tuple(&texture, &sampler, transform, blendMode));
}

void tre::Renderer2D::setLayerTexture(int layer, const tr::Texture2D& texture) noexcept
{
	assert(_layers.contains(layer));
	_layers.at(layer).texture = &texture;
}

void tre::Renderer2D::setLayerSampler(int layer, const tr::Sampler& sampler) noexcept
{
	assert(_layers.contains(layer));
	_layers.at(layer).sampler = &sampler;
}

void tre::Renderer2D::setLayerTransform(int layer, const glm::mat4& transform) noexcept
{
	assert(_layers.contains(layer));
	_layers.at(layer).transform = transform;
}

void tre::Renderer2D::setLayerBlendMode(int layer, const tr::BlendMode& blendMode) noexcept
{
	assert(_layers.contains(layer));
	_layers.at(layer).blendMode = blendMode;
}

void tre::Renderer2D::removeLayer(int layer) noexcept
{
	_layers.erase(layer);
}

void tre::Renderer2D::addColorQuad(int layer, const ColorQuad& quad)
{
	assert(_layers.contains(layer));
	TextureQuad textureQuad;
	for (int i = 0; i < 4; ++i) {
		textureQuad[i].pos   = quad[i].pos;
		textureQuad[i].uv    = UNTEXTURED_UV;
		textureQuad[i].color = quad[i].color;
	}
	_layers[layer].primitives.emplace_back(std::in_place_type<TextureQuad>, textureQuad);
}

void tre::Renderer2D::addTextureQuad(int layer, const TextureQuad& quad)
{
	assert(_layers.contains(layer));
	assert(_layers.at(layer).texture != nullptr && _layers.at(layer).sampler != nullptr);
	_layers[layer].primitives.emplace_back(std::in_place_type<TextureQuad>, quad);
}

void tre::Renderer2D::addColorFan(int layer, const ColorFan& fan)
{
	assert(_layers.contains(layer));
	assert(fan.size() >= 3);
	TextureFan textureFan;
	for (std::size_t i = 0; i < fan.size(); ++i) {
		textureFan[i].pos   = fan[i].pos;
		textureFan[i].uv    = UNTEXTURED_UV;
		textureFan[i].color = fan[i].color;
	}
	_layers[layer].primitives.emplace_back(std::in_place_type<TextureFan>, std::move(textureFan));
}

void tre::Renderer2D::addTextureFan(int layer, const TextureFan& fan)
{
	addTextureFan(layer, TextureFan{fan});
}

void tre::Renderer2D::addTextureFan(int layer, TextureFan&& fan)
{
	assert(_layers.contains(layer));
	assert(_layers.at(layer).texture != nullptr && _layers.at(layer).sampler != nullptr);
	assert(fan.size() >= 3);
	_layers[layer].primitives.emplace_back(std::in_place_type<TextureFan>, std::move(fan));
}

void tre::Renderer2D::addColorMesh(int layer, const std::vector<tr::ClrVtx2>& vertices,
								   const std::vector<std::uint16_t>& indices)
{
	addColorMesh(layer, vertices, std::vector<std::uint16_t>{indices});
}

void tre::Renderer2D::addColorMesh(int layer, const std::vector<tr::ClrVtx2>& vertices,
								   std::vector<std::uint16_t>&& indices)
{
	assert(_layers.contains(layer));
	assert(std::ranges::max(indices) == vertices.size() - 1);
	std::vector<tr::TintVtx2> textureVertices;
	for (std::size_t i = 0; i < vertices.size(); ++i) {
		textureVertices[i].pos   = vertices[i].pos;
		textureVertices[i].uv    = UNTEXTURED_UV;
		textureVertices[i].color = vertices[i].color;
	}
	_layers[layer].primitives.emplace_back(std::in_place_type<TextureMesh>, std::move(textureVertices),
										   std::move(indices));
}

void tre::Renderer2D::addTextureMesh(int layer, const std::vector<tr::TintVtx2>& vertices,
									 const std::vector<std::uint16_t>& indices)
{
	addTextureMesh(layer, std::vector<tr::TintVtx2>{vertices}, std::vector<std::uint16_t>{indices});
}

void tre::Renderer2D::addTextureMesh(int layer, std::vector<tr::TintVtx2>&& vertices,
									 std::vector<std::uint16_t>&& indices)
{
	assert(_layers.contains(layer));
	assert(_layers.at(layer).texture != nullptr && _layers.at(layer).sampler != nullptr);
	assert(std::ranges::max(indices) == vertices.size() - 1);
	_layers[layer].primitives.emplace_back(std::in_place_type<TextureMesh>, std::move(vertices), std::move(indices));
}

void tre::Renderer2D::setupContext() noexcept
{
	tr::window().graphics().useFaceCulling(false);
	tr::window().graphics().useDepthTest(false);
	tr::window().graphics().useStencilTest(false);
	tr::window().graphics().useBlending(true);
	tr::window().graphics().setShaderPipeline(_shaderPipeline);
	tr::window().graphics().setVertexFormat(tr::TintVtx2::vertexFormat());
}

void tre::Renderer2D::writeToBuffers(const Primitive& primitive, std::uint16_t& index)
{
	const auto textureQuad{[&](const TextureQuad& rect) {
		_vertices.insert(_vertices.end(), rect.begin(), rect.end());
		tr::fillPolygonIndices(back_inserter(_indices), 4, index);
		index += 4;
	}};
	const auto textureFan{[&](const TextureFan& fan) {
		_vertices.insert(_vertices.end(), fan.begin(), fan.end());
		tr::fillPolygonIndices(back_inserter(_indices), fan.size(), index);
		index += fan.size();
	}};
	const auto textureMesh{[&](const TextureMesh& mesh) {
		_vertices.insert(_vertices.end(), mesh.first.begin(), mesh.first.end());
		_indices.insert(_indices.end(), mesh.second.begin(), mesh.second.end());
		index += mesh.first.size();
	}};

	std::visit(tr::Overloaded{textureQuad, textureFan, textureMesh}, primitive);
}

std::vector<std::size_t> tre::Renderer2D::uploadToGraphicsBuffers(decltype(_layers)::iterator end)
{
	_vertices.clear();
	_indices.clear();
	std::uint16_t            index{0};
	std::vector<std::size_t> offsets;

	for (auto& layer : std::ranges::subrange{_layers.begin(), end} | std::views::values) {
		offsets.emplace_back(_indices.size());
		for (auto& primitive : layer.primitives) {
			writeToBuffers(primitive, index);
		}
		layer.primitives.clear();
	}
	offsets.emplace_back(_indices.size()); // Offset to end, simplifies a loop in the main function.

	_vertexBuffer.set(_vertices);
	_indexBuffer.set(_indices);
	tr::window().graphics().setVertexBuffer(_vertexBuffer, 0, sizeof(tr::TintVtx2));
	tr::window().graphics().setIndexBuffer(_indexBuffer);
	return offsets;
}

void tre::Renderer2D::drawUpToLayer(int maxPriority, const RenderView& target)
{
	if (std::ranges::all_of(_layers, [](auto& layer) { return layer.second.primitives.empty(); })) {
		return;
	}

	setupContext();
	target.use();

	const std::ranges::subrange    range{_layers.begin(), _layers.lower_bound(maxPriority)};
	const std::vector<std::size_t> indexOffsets{uploadToGraphicsBuffers(range.end())};
	auto                           it{indexOffsets.begin()};
	for (auto& layer : range | std::views::values) {
		static const tr::Texture2D* texture{};
		if (texture != layer.texture && layer.texture != nullptr) {
			texture = layer.texture;
			_textureUnit.setTexture(*texture);
		}
		static const tr::Sampler* sampler{};
		if (sampler != layer.sampler && layer.sampler != nullptr) {
			sampler = layer.sampler;
			_textureUnit.setSampler(*sampler);
		}
		static glm::mat4 transform{};
		if (transform != layer.transform) {
			transform = layer.transform;
			_shaderPipeline.vertexShader().setUniform(0, transform);
		}
		static tr::BlendMode blendMode{};
		if (blendMode != layer.blendMode) {
			tr::window().graphics().setBlendingMode(blendMode);
		}

		tr::window().graphics().drawIndexed(tr::Primitive::TRIS, *it, *std::next(it) - *it);
		++it;
	}
}

void tre::Renderer2D::draw(const RenderView& view)
{
	drawUpToLayer(std::numeric_limits<int>::max(), view);
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