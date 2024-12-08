#pragma once
#include <map>
#include <tr/tr.hpp>

namespace tre {
	/** @addtogroup renderer
	 *  @{
	 */

	/******************************************************************************************************************
	 * General-purpose batched 2D renderer.
	 *
	 * Only one instance of the 2D renderer is allowed to exist at a time.
	 ******************************************************************************************************************/
	class Renderer2D {
	  public:
		/**************************************************************************************************************
		 * The unique renderer ID of this renderer.
		 **************************************************************************************************************/
		static constexpr std::uint32_t ID{-2U};

		/**************************************************************************************************************
		 * Shorthand for the vertex type used by the renderer.
		 **************************************************************************************************************/
		using Vertex = tr::TintVtx2;

		/**************************************************************************************************************
		 * The combination of texture and sampler used when drawing an object.
		 **************************************************************************************************************/
		using TextureRef = std::pair<const tr::Texture2D&, const tr::Sampler&>;

		/**************************************************************************************************************
		 * Shorthand for a textured triangle used by the renderer.
		 **************************************************************************************************************/
		using TexturedTriangle = std::array<Vertex, 3>;

		/**************************************************************************************************************
		 * Shorthand for a textured quad used by the renderer.
		 **************************************************************************************************************/
		using TexturedQuad = std::array<Vertex, 4>;

		/**************************************************************************************************************
		 * Shorthand for a textured vertex fan used by the renderer.
		 **************************************************************************************************************/
		using TexturedVertexFan = std::vector<Vertex>;

		/**************************************************************************************************************
		 * Creates the 2D renderer.
		 *
		 * Only one of these should be created at a time. This class can only be instantiated after an OpenGL context
		 * is opened.
		 **************************************************************************************************************/
		Renderer2D();

		~Renderer2D() noexcept;

		/**************************************************************************************************************
		 * Gets the size of the rendered field.
		 *
		 * @return The size of the rendered field.
		 **************************************************************************************************************/
		glm::vec2 fieldSize() const noexcept;

		/**************************************************************************************************************
		 * Sets the size of the rendered field.
		 *
		 * @param[in] size The new size of the rendered field.
		 **************************************************************************************************************/
		void setFieldSize(glm::vec2 size) noexcept;

		/**************************************************************************************************************
		 * Sets the blending mode used by the renderer.
		 *
		 * @param[in] blendMode The blending mode used by the renderer.
		 **************************************************************************************************************/
		void setBlendingMode(tr::BlendMode blendMode) noexcept;

		/**************************************************************************************************************
		 * Sets the scissor box used by the renderer.
		 *
		 * @param[in] scissorBox The scissor box to use, or std::nullopt to disable the scissor test.
		 **************************************************************************************************************/
		void setScissorBox(std::optional<tr::RectI2> scissorBox) noexcept;

		/**************************************************************************************************************
		 * Adds an untextured rect to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the object (higher is drawn on top).
		 * @param[in] rect The rectangle to draw.
		 * @param[in] color The color of the rectangle.
		 **************************************************************************************************************/
		void addUntexturedRect(int priority, const tr::RectF2& rect, tr::RGBA8 color);

		/**************************************************************************************************************
		 * Adds an untextured rect to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the object (higher is drawn on top).
		 * @param[in] rect The rectangle to draw.
		 * @param[in] colors The colors of the rectangle, sampled counterclockwise starting from the top-left.
		 **************************************************************************************************************/
		void addUntexturedRect(int priority, const tr::RectF2& rect, std::array<tr::RGBA8, 4> colors);

		/**************************************************************************************************************
		 * Adds a textured rect to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the object (higher is drawn on top).
		 * @param[in] rect The rectangle to draw.
		 * @param[in] texture The texture and sampler used for the rectangle.
		 * @param[in] uv The UV values used for the rectangle (normalized).
		 * @param[in] tint The tint of the rectangle.
		 **************************************************************************************************************/
		void addTexturedRect(int priority, const tr::RectF2& rect, TextureRef texture, const tr::RectF2& uv,
							 tr::RGBA8 tint);

		/**************************************************************************************************************
		 * Adds a textured rect to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the object (higher is drawn on top).
		 * @param[in] rect The rectangle to draw.
		 * @param[in] texture The texture and sampler used for the rectangle.
		 * @param[in] uvs The UV values used for the rectangle (normalized), sampled counterclockwise starting from the
		 *                top-left.
		 * @param[in] tints The tint of the rectangle, sampled counterclockwise starting from the top-left.
		 **************************************************************************************************************/
		void addTexturedRect(int priority, const tr::RectF2& rect, TextureRef texture, std::array<glm::vec2, 4> uvs,
							 std::array<tr::RGBA8, 4> tints);

		/**************************************************************************************************************
		 * Adds an untextured, rotated rectangle to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the object (higher is drawn on top).
		 * @param[in] pos The position of the posAnchor.
		 * @param[in] posAnchor The position of the position/rotation anchor within the rectangle.
		 * @param[in] size The size of the rectangle.
		 * @param[in] rotation The rotation of the rectangle.
		 * @param[in] color The color of the rectangle.
		 **************************************************************************************************************/
		void addUntexturedRotatedRectangle(int priority, glm::vec2 pos, glm::vec2 posAnchor, glm::vec2 size,
										   tr::AngleF rotation, tr::RGBA8 color);

		/**************************************************************************************************************
		 * Adds an untextured, rotated rectangle to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the object (higher is drawn on top).
		 * @param[in] pos The position of the posAnchor.
		 * @param[in] posAnchor The position of the position/rotation anchor within the rectangle.
		 * @param[in] size The size of the rectangle.
		 * @param[in] rotation The rotation of the rectangle.
		 * @param[in] colors The colors of the rectangle, sampled counterclockwise starting from the top-left.
		 **************************************************************************************************************/
		void addUntexturedRotatedRectangle(int priority, glm::vec2 pos, glm::vec2 posAnchor, glm::vec2 size,
										   tr::AngleF rotation, std::array<tr::RGBA8, 4> colors);

		/**************************************************************************************************************
		 * Adds an untextured, rotated rectangle to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the object (higher is drawn on top).
		 * @param[in] pos The position of the posAnchor.
		 * @param[in] posAnchor The position of the position/rotation anchor within the rectangle.
		 * @param[in] size The size of the rectangle.
		 * @param[in] rotation The rotation of the rectangle.
		 * @param[in] texture The texture and sampler used for the rectangle.
		 * @param[in] uv The UV values used for the rectangle (normalized).
		 * @param[in] tint The tint of the rectangle.
		 **************************************************************************************************************/
		void addTexturedRotatedRectangle(int priority, glm::vec2 pos, glm::vec2 posAnchor, glm::vec2 size,
										 tr::AngleF rotation, TextureRef texture, const tr::RectF2& uv, tr::RGBA8 tint);

		/**************************************************************************************************************
		 * Adds an untextured, rotated rectangle to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the object (higher is drawn on top).
		 * @param[in] pos The position of the posAnchor.
		 * @param[in] posAnchor The position of the position/rotation anchor within the rectangle.
		 * @param[in] size The size of the rectangle.
		 * @param[in] rotation The rotation of the rectangle.
		 * @param[in] texture The texture and sampler used for the rectangle.
		 * @param[in] uvs The UV values used for the rectangle (normalized), sampled counterclockwise starting from the
		 *                top-left.
		 * @param[in] tints The tint of the rectangle, sampled counterclockwise starting from the top-left.
		 **************************************************************************************************************/
		void addTexturedRotatedRectangle(int priority, glm::vec2 pos, glm::vec2 posAnchor, glm::vec2 size,
										 tr::AngleF rotation, TextureRef texture, std::array<glm::vec2, 4> uvs,
										 std::array<tr::RGBA8, 4> tints);

		/**************************************************************************************************************
		 * Adds an untextured triangle to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the object (higher is drawn on top).
		 * @param[in] triangle The triangle vertices.
		 **************************************************************************************************************/
		void addUntexturedTriangle(int priority, std::span<tr::ClrVtx2, 3> triangle);

		/**************************************************************************************************************
		 * Adds a textured triangle to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the object (higher is drawn on top).
		 * @param[in] triangle The triangle vertices.
		 * @param[in] texture The texture and sampler used for the triangle.
		 **************************************************************************************************************/
		void addTexturedTriangle(int priority, TexturedTriangle triangle, TextureRef texture);

		/**************************************************************************************************************
		 * Adds an untextured quad to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the object (higher is drawn on top).
		 * @param[in] quad The quad vertices.
		 **************************************************************************************************************/
		void addUntexturedQuad(int priority, std::span<tr::ClrVtx2, 4> quad);

		/**************************************************************************************************************
		 * Adds a textured quad to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the object (higher is drawn on top).
		 * @param[in] quad The quad vertices.
		 * @param[in] texture The texture and sampler used for the quad.
		 **************************************************************************************************************/
		void addTexturedQuad(int priority, TexturedQuad quad, TextureRef texture);

		/**************************************************************************************************************
		 * Adds an untextured regular polygon to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the object (higher is drawn on top).
		 * @param[in] circle The polygon circle.
		 * @param[in] vertexCount The number of vertices in the polygon.
		 * @param[in] rotation The rotation of the polygon.
		 * @param[in] color The color of the polygon.
		 **************************************************************************************************************/
		void addUntexturedPolygon(int priority, const tr::CircleF& circle, int vertexCount, tr::AngleF rotation,
								  tr::RGBA8 color);

		/**************************************************************************************************************
		 * Adds an untextured circle to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the object (higher is drawn on top).
		 * @param[in] circle The circle to draw.
		 * @param[in] color The color of the circle.
		 **************************************************************************************************************/
		void addUntexturedCircle(int priority, const tr::CircleF& circle, tr::RGBA8 color);

		/**************************************************************************************************************
		 * Adds an untextured polygon fan to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the object (higher is drawn on top).
		 * @param[in] vertices The vertices of the polygon fan.
		 **************************************************************************************************************/
		void addUntexturedPolygonFan(int priority, std::span<tr::ClrVtx2> vertices);

		/**************************************************************************************************************
		 * Adds an untextured polygon fan to the list of objects to draw in the next draw call.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the object (higher is drawn on top).
		 * @param[in] vertices The vertices of the polygon fan.
		 * @param[in] texture The texture and sampler used for the polygon fan.
		 **************************************************************************************************************/
		void addTexturedPolygonFan(int priority, std::vector<Vertex> vertices, TextureRef texture);

		/**************************************************************************************************************
		 * Draws added objects of a certain priotrity or lower to a target.
		 *
		 * The render graph is cleared of these objects afterwards.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 * @exception tr::GLBufferBadAlloc If an internal allocation fails.
		 *
		 * @param[in] maxPriority The maximum drawn priority.
		 * @param[in] target The drawing target.
		 **************************************************************************************************************/
		void drawUpToPriority(int maxPriority, tr::BasicFramebuffer& target = tr::window().backbuffer());

		/**************************************************************************************************************
		 * Draws all added objects to a target.
		 *
		 * The render graph is cleared afterwards.
		 *
		 * Equivalent to drawUpToPriority(glContext, target, INT_MIN).
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 * @exception tr::GLBufferBadAlloc If an internal allocation fails.
		 *
		 * @param[in] target The drawing target.
		 **************************************************************************************************************/
		void draw(tr::BasicFramebuffer& target = tr::window().backbuffer());

	  private:
		struct TextureRefHash {
			std::size_t operator()(const std::optional<TextureRef>& texture) const noexcept;
		};

		using RawData   = std::pair<std::vector<Vertex>, std::vector<std::uint16_t>>;
		using Primitive = std::variant<TexturedTriangle, TexturedQuad, TexturedVertexFan, RawData>;
		using Priority  = std::unordered_map<std::optional<TextureRef>, std::vector<Primitive>, TextureRefHash>;

		tr::OwningShaderPipeline   _shaderPipeline;
		tr::TextureUnit            _textureUnit;
		tr::VertexBuffer           _vertexBuffer;
		tr::IndexBuffer            _indexBuffer;
		std::vector<Vertex>        _vertices;
		std::vector<std::uint16_t> _indices;
		std::map<int, Priority>    _renderGraph;

		glm::vec2     _fieldSize;
		tr::BlendMode _blendMode;
		tr::RectI2    _scissorBox;

		void setupContext() noexcept;
		void writeToVertexIndexVectors(const Primitive& primitive, std::uint16_t& index);
	};

	/******************************************************************************************************************
	 * Gets whether the 2D renderer was initialized.
	 *
	 * @return True if the 2D renderer was initialized, and false otherwise.
	 ******************************************************************************************************************/
	bool renderer2DActive() noexcept;

	/******************************************************************************************************************
	 * Gets a reference to the 2D renderer.
	 * This function cannot be called if the 2D renderer wasn't initialized.
	 *
	 * @return A reference to the 2D renderer.
	 ******************************************************************************************************************/
	Renderer2D& renderer2D() noexcept;

	/// @}
} // namespace tre