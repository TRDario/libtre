#pragma once
#include <map>
#include <tr/geometry.hpp>
#include <tr/tr.hpp>

namespace tre {
	/******************************************************************************************************************
	 * Basic, optimized 2D renderer.
	 ******************************************************************************************************************/
	class Renderer2D {
	public:
		/**************************************************************************************************************
		 * The unique renderer ID of this renderer.
		 **************************************************************************************************************/
		static constexpr std::uint32_t ID {-2U};

		/**************************************************************************************************************
		 * Shorthand for the vertex type used by the renderer.
		 **************************************************************************************************************/
		using Vertex     = tr::TintVtx2;

		/**************************************************************************************************************
		 * The combination of texture and sampler used when drawing an object.
		 **************************************************************************************************************/
		using TextureRef = std::pair<const tr::Texture2D&, const tr::Sampler&>;

		/**************************************************************************************************************
		 * Creates the 2D renderer.
		 *
		 * Only one of these should be created at a time. This class can only be instantiated after an OpenGL context
		 * is opened.
		 **************************************************************************************************************/
		Renderer2D();

		/**************************************************************************************************************
		 * Sets the size of the rendered field.
		 *
		 * @param size The new size of the rendered field.
		 **************************************************************************************************************/
		void setFieldSize(glm::vec2 size) noexcept;

		/**************************************************************************************************************
		 * Sets the blending mode used by the renderer.
		 *
		 * @param blendMode The blending mode used by the renderer.
		 **************************************************************************************************************/
		void setBlendingMode(tr::BlendMode blendMode) noexcept;

		/**************************************************************************************************************
		 * Sets the scissor box used by the renderer.
		 *
		 * @param scissorBox The scissor box to use, or std::nullopt to disable the scissor test.
		 **************************************************************************************************************/
		void setScissorBox(std::optional<tr::RectI2> scissorBox) noexcept;

		/**************************************************************************************************************
		 * Adds an untextured rect to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an itr::RectI2.
		 * @param color The color of the rectangle.
		 **************************************************************************************************************/
		void addUntexturedRect(std::int64_t priority, const tr::RectF2& rect, tr::RGBA8 color);

		/**************************************************************************************************************
		 * Adds an untextured rect to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param priority The drawing priority of the object (higher is drawn on top).
		 * @param rect The rectangle to draw.
		 * @param colors The colors of the rectangle, sampled counterclockwise starting from the top-left.
		 **************************************************************************************************************/
		void addUntexturedRect(std::int64_t priority, const tr::RectF2& rect, std::array<tr::RGBA8, 4> colors);

		/**************************************************************************************************************
		 * Adds a textured rect to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param priority The drawing priority of the object (higher is drawn on top).
		 * @param rect The rectangle to draw.
		 * @param texture The texture and sampler used for the rectangle.
		 * @param uv The UV values used for the rectangle (normalized).
		 * @param tint The tint of the rectangle.
		 **************************************************************************************************************/
		void addTexturedRect(
			std::int64_t      priority,
			const tr::RectF2& rect,
			TextureRef        texture,
			const tr::RectF2& uv,
			tr::RGBA8         tint
		);

		/**************************************************************************************************************
		 * Adds a textured rect to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param priority The drawing priority of the object (higher is drawn on top).
		 * @param rect The rectangle to draw.
		 * @param texture The texture and sampler used for the rectangle.
		 * @param uvs The UV values used for the rectangle (normalized), sampled counterclockwise starting from the top-left.
		 * @param tints The tint of the rectangle, sampled counterclockwise starting from the top-left.
		 **************************************************************************************************************/
		void addTexturedRect(
			std::int64_t             priority,
			const tr::RectF2&        rect,
			TextureRef               texture,
			std::array<glm::vec2, 4> uvs,
			std::array<tr::RGBA8, 4> tints
		);

		/**************************************************************************************************************
		 * Adds an untextured, rotated rectangle to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param priority The drawing priority of the object (higher is drawn on top).
		 * @param pos The position of the posAnchor.
		 * @param posAchor The position of the position/rotation anchor within the rectangle.
		 * @param size The size of the rectangle.
		 * @param rotation The rotation of the rectangle.
		 * @param color The color of the rectangle.
		 **************************************************************************************************************/
		void addUntexturedRotatedRectangle(
			std::int64_t priority,
			glm::vec2    pos,
			glm::vec2    posAnchor,
			glm::vec2    size,
			tr::AngleF   rotation,
			tr::RGBA8    color
		);

		/**************************************************************************************************************
		 * Adds an untextured, rotated rectangle to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param priority The drawing priority of the object (higher is drawn on top).
		 * @param pos The position of the posAnchor.
		 * @param posAchor The position of the position/rotation anchor within the rectangle.
		 * @param size The size of the rectangle.
		 * @param rotation The rotation of the rectangle.
		 * @param colors The colors of the rectangle, sampled counterclockwise starting from the top-left.
		 **************************************************************************************************************/
		void addUntexturedRotatedRectangle(
			std::int64_t             priority,
			glm::vec2                pos,
			glm::vec2                posAnchor,
			glm::vec2                size,
			tr::AngleF               rotation,
			std::array<tr::RGBA8, 4> colors
		);

		/**************************************************************************************************************
		 * Adds an untextured, rotated rectangle to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param priority The drawing priority of the object (higher is drawn on top).
		 * @param pos The position of the posAnchor.
		 * @param posAchor The position of the position/rotation anchor within the rectangle.
		 * @param size The size of the rectangle.
		 * @param rotation The rotation of the rectangle.
		 * @param texture The texture and sampler used for the rectangle.
		 * @param uv The UV values used for the rectangle (normalized).
		 * @param tint The tint of the rectangle.
		 **************************************************************************************************************/
		void addTexturedRotatedRectangle(
			std::int64_t      priority,
			glm::vec2         pos,
			glm::vec2         posAnchor,
			glm::vec2         size,
			tr::AngleF        rotation,
			TextureRef        texture,
			const tr::RectF2& uv,
			tr::RGBA8         tint
		);

		/**************************************************************************************************************
		 * Adds an untextured, rotated rectangle to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param priority The drawing priority of the object (higher is drawn on top).
		 * @param pos The position of the posAnchor.
		 * @param posAchor The position of the position/rotation anchor within the rectangle.
		 * @param size The size of the rectangle.
		 * @param rotation The rotation of the rectangle.
		 * @param texture The texture and sampler used for the rectangle.
		 * @param uvs The UV values used for the rectangle (normalized), sampled counterclockwise starting from the top-left.
		 * @param tints The tint of the rectangle, sampled counterclockwise starting from the top-left.
		 **************************************************************************************************************/
		void addTexturedRotatedRectangle(
			std::int64_t             priority,
			glm::vec2                pos,
			glm::vec2                posAnchor,
			glm::vec2                size,
			tr::AngleF               rotation,
			TextureRef               texture,
			std::array<glm::vec2, 4> uvs,
			std::array<tr::RGBA8, 4> tints
		);

		/**************************************************************************************************************
		 * Adds an untextured regular polygon to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param priority The drawing priority of the object (higher is drawn on top).
		 * @param circle The polygon circle.
		 * @param vertexCount The number of vertices in the polygon.
		 * @param rotation The rotation of the polygon.
		 * @param color The color of the rectangle.
		 **************************************************************************************************************/
		void addUntexturedPolygon(
			std::int64_t       priority,
			const tr::CircleF& circle,
			int                vertexCount,
			tr::AngleF         rotation,
			tr::RGBA8          color
		);

		/**************************************************************************************************************
		 * Adds an untextured circle to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param priority The drawing priority of the object (higher is drawn on top).
		 * @param circle The circle to draw.
		 * @param color The color of the rectangle.
		 **************************************************************************************************************/
		void addUntexturedCircle(std::int64_t priority, const tr::CircleF& circle, tr::RGBA8 color);

		/**************************************************************************************************************
		 * Adds an untextured polygon fan to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param priority The drawing priority of the object (higher is drawn on top).
		 * @param vertices The vertices of the polygon fan.
		 **************************************************************************************************************/
		void addUntexturedPolygonFan(std::int64_t priority, std::span<tr::ClrVtx2> vertices);

		/**************************************************************************************************************
		 * Adds an untextured polygon fan to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param priority The drawing priority of the object (higher is drawn on top).
		 * @param vertices The vertices of the polygon fan.
		 * @param texture The texture and sampler used for the polygon fan.
		 **************************************************************************************************************/
		void addTexturedPolygonFan(std::int64_t priority, std::vector<Vertex> vertices, TextureRef texture);

		/**************************************************************************************************************
		 * Draws all added objects to a target.
		 *
		 * The render graph is cleared afterwards.
		 *
		 * @exception std::bad_alloc If an internal allocation failed.
		 * @exception tr::GLBufferBadAlloc If an internal allocation failed.
		 *
		 * @param glContext The OpenGL context to manipulate.
		 * @param target The drawing target.
		 **************************************************************************************************************/
		void draw(tr::GLContext& glContext, tr::BasicFramebuffer& target);

	private:
		struct TextureRefHash {
			std::size_t operator()(const std::optional<TextureRef>& texture) const noexcept;
		};

		using Triangle      = std::array<Vertex, 3>;
		using Rectangle     = std::array<Vertex, 4>;
		using VertexFan     = std::vector<Vertex>;
		using RawData       = std::pair<std::vector<Vertex>, std::vector<std::uint16_t>>;
		using PrimitiveList = std::vector<std::variant<Triangle, Rectangle, VertexFan, RawData>>;
		using Priority      = std::unordered_map<std::optional<TextureRef>, PrimitiveList, TextureRefHash>;
		using RenderGraph   = std::map<std::int64_t, Priority, std::greater<std::int64_t>>;

		tr::Shader                 _vertexShader;
		tr::Shader                 _fragmentShader;
		tr::ShaderPipeline         _shaderPipeline;
		tr::TextureUnit            _textureUnit;
		tr::VertexBuffer           _vertexBuffer;
		tr::IndexBuffer            _indexBuffer;
		std::vector<Vertex>        _vertices;
		std::vector<std::uint16_t> _indices;
		RenderGraph                _renderGraph;

		glm::vec2                  _fieldSize;
		tr::BlendMode              _blendMode;
		tr::RectI2                 _scissorBox;

	private:
		void setupContext(tr::GLContext& glContext) noexcept;
		void writeToVertexIndexVectors(
			const std::variant<Triangle, Rectangle, VertexFan, RawData>& primitive,
			std::uint16_t&                                               index
		);
	};
} // namespace tre
