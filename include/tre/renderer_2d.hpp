#pragma once
#include "render_view.hpp"
#include <map>

namespace tre {
	/** @defgroup renderer_2d 2D Renderer
	 *  Layer-based batched 2D renderer.
	 *
	 *  @{
	 */

	/******************************************************************************************************************
	 * Layer-based batched 2D renderer.
	 *
	 * Renderer2D uses @em layers as a way of grouping primitives of the same drawing priority, as well as the same
	 * rendering configuration (texture, sampler, transfomration matrix, blending mode). Smart usage of layers will
	 * group layers with similar rendering configurations cloe by to minimize rendering state changes.
	 *
	 * The Renderer2D class uses something akin to the singleton pattern. It is still your job to instantiate the
	 * renderer once (and only once!), after which it will stay active until its destructor is called, but this instance
	 * will be globally available through renderer2D(). Instancing the renderer again after it has been closed is a
	 * valid action.
	 *
	 * Renderer2D is move-constructible, but neither copyable nor assignable. A moved renderer is left in a state where
	 * another renderer can be moved into it, but is otherwise unusable.
	 *
	 * @note An instance of tr::Window must be created before Renderer2D can be instantiated.
	 ******************************************************************************************************************/
	class Renderer2D {
	  public:
		/**************************************************************************************************************
		 * Creates the 2D renderer and enables the ability to use the tre::renderer2D() getter.
		 *
		 * @note Only one instance of Renderer2D can exist at any one time.
		 **************************************************************************************************************/
		Renderer2D();

		/**************************************************************************************************************
		 * Move-constructs a 2D renderer.
		 *
		 * @param[in] r The renderer to move from. @em r will be left in a moved-from state that shouldn't be used.
		 **************************************************************************************************************/
		Renderer2D(Renderer2D&& r) noexcept;

		/**************************************************************************************************************
		 * Destroys the 2D renderer and disables the ability to use the tre::renderer2D() getter.
		 **************************************************************************************************************/
		~Renderer2D() noexcept;

		/**************************************************************************************************************
		 * Adds a new color-only layer to the renderer.
		 *
		 * @note The layer can be turned into a full layer afterwards by setting its texture and sampler.
		 *
		 * @par Exception Safety
		 *
		 * Strong exception guarantee.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority
		 * @parblock
		 * The priority of the layer (layers with a higher priority are drawn on top).
		 *
		 * This value will be used as the name of the layer afterwards.
		 *
		 * @pre A layer with this priority cannot exist already.
		 * @endparblock
		 * @param[in] transform The transformation matrix used for primitives on this layer.
		 * @param[in] blendMode The blending mode used for primitives on this layer.
		 **************************************************************************************************************/
		void addColorOnlyLayer(int priority, const glm::mat4& transform,
							   const tr::BlendMode& blendMode = tr::ALPHA_BLENDING);

		/**************************************************************************************************************
		 * Adds a new full layer to the renderer.
		 *
		 * @par Exception Safety
		 *
		 * Strong exception guarantee.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority
		 * @parblock
		 * The priority of the layer (layers with a higher priority are drawn on top).
		 *
		 * This value will be used as the name of the layer afterwards.
		 *
		 * @pre A layer with this priority cannot exist already.
		 * @endparblock
		 * @param[in] texture
		 * @parblock
		 * The texture used for textured primitives on this layer.
		 *
		 * @warning The texture reference must stay valid for as long as this layer uses it.
		 * @endparblock
		 * @param[in] sampler
		 * @parblock
		 * The sampler used for textured primitives on this layer.
		 *
		 * @warning The sampler reference must stay valid for as long as this layer uses it.
		 * @endparblock
		 * @param[in] transform The transformation matrix used for primitives on this layer.
		 * @param[in] blendMode The blending mode used for primitives on this layer.
		 **************************************************************************************************************/
		void addLayer(int priority, const tr::Texture2D& texture, const tr::Sampler& sampler,
					  const glm::mat4& transform, const tr::BlendMode& blendMode = tr::ALPHA_BLENDING);

		/**************************************************************************************************************
		 * Sets the texture used by textured primitives on a layer.
		 *
		 * @param[in] layer
		 * @parblock
		 * The layer to set the texture for.
		 *
		 * @pre The renderer must have a layer with priority @em layer.
		 * @endparblock
		 * @param[in] texture
		 * @parblock
		 * The texture to use for textured primitives on this layer.
		 *
		 * @warning The texture reference must stay valid for as long as this layer uses it.
		 * @endparblock
		 **************************************************************************************************************/
		void setLayerTexture(int layer, const tr::Texture2D& texture) noexcept;

		/**************************************************************************************************************
		 * Sets the sampler used by textured primitives on a layer.
		 *
		 * @param[in] layer
		 * @parblock
		 * The layer to set the sampler for.
		 *
		 * @pre The renderer must have a layer with priority @em layer.
		 * @endparblock
		 * @param[in] sampler
		 * @parblock
		 * The sampler to use for textured primitives on this layer.
		 *
		 * @warning The sampler reference must stay valid for as long as this layer uses it.
		 * @endparblock
		 **************************************************************************************************************/
		void setLayerSampler(int layer, const tr::Sampler& sampler) noexcept;

		/**************************************************************************************************************
		 * Sets the transformation matrix used by primitives on a layer.
		 *
		 * @param[in] layer
		 * @parblock
		 * The layer to set the transformation matrix for.
		 *
		 * @pre The renderer must have a layer with priority @em layer.
		 * @endparblock
		 * @param[in] transform The transformation matrix to use for textured primitives on this layer.
		 **************************************************************************************************************/
		void setLayerTransform(int layer, const glm::mat4& transform) noexcept;

		/**************************************************************************************************************
		 * Sets the blending mode used by primitives on a layer.
		 *
		 * @param[in] layer
		 * @parblock
		 * The layer to set the blending mode for.
		 *
		 * @pre The renderer must have a layer with priority @em layer.
		 * @endparblock
		 * @param[in] blendMode The blending mode to use for textured primitives on this layer.
		 **************************************************************************************************************/
		void setLayerBlendMode(int layer, const tr::BlendMode& blendMode) noexcept;

		/**************************************************************************************************************
		 * Removes a layer from the renderer.
		 *
		 * @param[in] layer The layer to remove.
		 **************************************************************************************************************/
		void removeLayer(int layer) noexcept;

		/**************************************************************************************************************
		 * Shorthand for an untextured quad primitive.
		 **************************************************************************************************************/
		using ColorQuad = std::array<tr::ClrVtx2, 4>;

		/**************************************************************************************************************
		 * Adds an untextured quad to be rendered.
		 *
		 * @par Exception Safety
		 *
		 * Strong exception guarantee.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] layer
		 * @parblock
		 * The layer to draw the quad on.
		 *
		 * @pre The renderer must have a layer with priority @em layer.
		 * @endparblock
		 * @param[in] quad The quad to draw according to layer parameters.
		 **************************************************************************************************************/
		void addColorQuad(int layer, const ColorQuad& quad);

		/**************************************************************************************************************
		 * Shorthand for a textured quad primitive.
		 **************************************************************************************************************/
		using TextureQuad = std::array<tr::TintVtx2, 4>;

		/**************************************************************************************************************
		 * Adds a textured quad to be rendered.
		 *
		 * @par Exception Safety
		 *
		 * Strong exception guarantee.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] layer
		 * @parblock
		 * The layer to draw the quad on.
		 *
		 * @pre The renderer must have a layer with priority @em layer.
		 *
		 * @pre @em layer must be a full layer, i.e. have a texture and sampler defined for it.
		 * @endparblock
		 * @param[in] quad The quad to draw according to layer parameters.
		 **************************************************************************************************************/
		void addTextureQuad(int layer, const TextureQuad& quad);

		/**************************************************************************************************************
		 * Shorthand for an untextured vertex fan primitive.
		 **************************************************************************************************************/
		using ColorFan = std::vector<tr::ClrVtx2>;

		/**************************************************************************************************************
		 * Adds an untextured vertex fan to be rendered.
		 *
		 * @par Exception Safety
		 *
		 * Strong exception guarantee.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] layer
		 * @parblock
		 * The layer to draw the vertex fan on.
		 *
		 * @pre The renderer must have a layer with priority @em layer.
		 * @endparblock
		 * @param[in] fan
		 * @parblock
		 * The vertex fan to draw according to layer parameters.
		 *
		 * @pre @em fan must contain 3 or more vertices.
		 * @endparblock
		 **************************************************************************************************************/
		void addColorFan(int layer, const ColorFan& fan);

		/**************************************************************************************************************
		 * Shorthand for a textured vertex fan primitive.
		 **************************************************************************************************************/
		using TextureFan = std::vector<tr::TintVtx2>;

		/**************************************************************************************************************
		 * Adds a textured vertex fan to be rendered.
		 *
		 * @par Exception Safety
		 *
		 * Strong exception guarantee.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] layer
		 * @parblock
		 * The layer to draw the vertex fan on.
		 *
		 * @pre The renderer must have a layer with priority @em layer.
		 *
		 * @pre @em layer must be a full layer, i.e. have a texture and sampler defined for it.
		 * @endparblock
		 * @param[in] fan
		 * @parblock
		 * The vertex fan to draw according to layer parameters.
		 *
		 * @pre @em fan must contain 3 or more vertices.
		 * @endparblock
		 **************************************************************************************************************/
		void addTextureFan(int layer, const TextureFan& fan);

		/**************************************************************************************************************
		 * Adds a textured vertex fan to be rendered.
		 *
		 * @par Exception Safety
		 *
		 * Strong exception guarantee.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] layer
		 * @parblock
		 * The layer to draw the vertex fan on.
		 *
		 * @pre The renderer must have a layer with priority @em layer.
		 *
		 * @pre @em layer must be a full layer, i.e. have a texture and sampler defined for it.
		 * @endparblock
		 * @param[in] fan
		 * @parblock
		 * The vertex fan to draw according to layer parameters. The contents of @em fan will be moved.
		 *
		 * @pre @em fan must contain 3 or more vertices.
		 * @endparblock
		 **************************************************************************************************************/
		void addTextureFan(int layer, TextureFan&& fan);

		/**************************************************************************************************************
		 * Adds an untextured mesh to be rendered.
		 *
		 * @par Exception Safety
		 *
		 * Strong exception guarantee.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] layer
		 * @parblock
		 * The layer to draw the mesh on.
		 *
		 * @pre The renderer must have a layer with priority @em layer.
		 * @endparblock
		 * @param[in] vertices
		 * @parblock
		 * The vertices of the mesh.
		 *
		 * @pre @em vertices must contain 3 or more vertices.
		 * @endparblock
		 * @param[in] indices
		 * @parblock
		 * The indices of the mesh.
		 *
		 * @pre The contents of @em indices must span from [0, vertices.size()).
		 * @endparblock
		 **************************************************************************************************************/
		void addColorMesh(int layer, const std::vector<tr::ClrVtx2>& vertices,
						  const std::vector<std::uint16_t>& indices);

		/**************************************************************************************************************
		 * Adds an untextured mesh to be rendered.
		 *
		 * @par Exception Safety
		 *
		 * Strong exception guarantee.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] layer
		 * @parblock
		 * The layer to draw the mesh on.
		 *
		 * @pre The renderer must have a layer with priority @em layer.
		 * @endparblock
		 * @param[in] vertices
		 * @parblock
		 * The vertices of the mesh.
		 *
		 * @pre @em vertices must contain 3 or more vertices.
		 * @endparblock
		 * @param[in] indices
		 * @parblock
		 * The indices of the mesh. The contents of @em indices will be moved.
		 *
		 * @pre The contents of @em indices must span from [0, vertices.size()).
		 * @endparblock
		 **************************************************************************************************************/
		void addColorMesh(int layer, const std::vector<tr::ClrVtx2>& vertices, std::vector<std::uint16_t>&& indices);

		/**************************************************************************************************************
		 * Adds a textured mesh to be rendered.
		 *
		 * @par Exception Safety
		 *
		 * Strong exception guarantee.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] layer
		 * @parblock
		 * The layer to draw the mesh on.
		 *
		 * @pre The renderer must have a layer with priority @em layer.
		 *
		 * @pre @em layer must be a full layer, i.e. have a texture and sampler defined for it.
		 * @endparblock
		 * @param[in] vertices
		 * @parblock
		 * The vertices of the mesh.
		 *
		 * @pre @em vertices must contain 3 or more vertices.
		 * @endparblock
		 * @param[in] indices
		 * @parblock
		 * The indices of the mesh.
		 *
		 * @pre The contents of @em indices must span from [0, vertices.size()).
		 * @endparblock
		 **************************************************************************************************************/
		void addTextureMesh(int layer, const std::vector<tr::TintVtx2>& vertices,
							const std::vector<std::uint16_t>& indices);

		/**************************************************************************************************************
		 * Adds a textured mesh to be rendered.
		 *
		 * @par Exception Safety
		 *
		 * Strong exception guarantee.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] layer
		 * @parblock
		 * The layer to draw the mesh on.
		 *
		 * @pre The renderer must have a layer with priority @em layer.
		 *
		 * @pre @em layer must be a full layer, i.e. have a texture and sampler defined for it.
		 * @endparblock
		 * @param[in] vertices
		 * @parblock
		 * The vertices of the mesh. The contents of @em vertices will be moved.
		 *
		 * @pre @em vertices must contain 3 or more vertices.
		 * @endparblock
		 * @param[in] indices
		 * @parblock
		 * The indices of the mesh. The contents of @em indices will be moved.
		 *
		 * @pre The contents of @em indices must span from [0, vertices.size()).
		 * @endparblock
		 **************************************************************************************************************/
		void addTextureMesh(int layer, std::vector<tr::TintVtx2>&& vertices, std::vector<std::uint16_t>&& indices);

		/**************************************************************************************************************
		 * Draws all layers of priority <= maxLayer to a render view.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 * @exception tr::GLBufferBadAlloc If an internal allocation fails.
		 *
		 * @param[in] maxLayer The maximum drawn layer priority.
		 * @param[in] view The target render view.
		 **************************************************************************************************************/
		void drawUpToLayer(int maxLayer, const RenderView& view = tr::window().backbuffer());

		/**************************************************************************************************************
		 * Draws all added primitives to a render view.
		 *
		 * Equivalent to drawUpToLayer(target, INT_MAX).
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 * @exception tr::GLBufferBadAlloc If an internal allocation fails.
		 *
		 * @param[in] view The target render view.
		 **************************************************************************************************************/
		void draw(const RenderView& view = tr::window().backbuffer());

	  private:
		using TextureMesh = std::pair<std::vector<tr::TintVtx2>, std::vector<std::uint16_t>>;
		using Primitive   = std::variant<TextureQuad, TextureFan, TextureMesh>;
		struct Layer {
			const tr::Texture2D*   texture;
			const tr::Sampler*     sampler;
			glm::mat4              transform;
			tr::BlendMode          blendMode;
			std::vector<Primitive> primitives;
		};

		tr::OwningShaderPipeline   _shaderPipeline;
		tr::TextureUnit            _textureUnit;
		tr::VertexBuffer           _vertexBuffer;
		tr::IndexBuffer            _indexBuffer;
		std::vector<tr::TintVtx2>  _vertices;
		std::vector<std::uint16_t> _indices;
		std::map<int, Layer>       _layers;

		void                     setupContext() noexcept;
		void                     writeToBuffers(const Primitive& primitive, std::uint16_t& index);
		std::vector<std::size_t> uploadToGraphicsBuffers(decltype(_layers)::iterator end);
	};

	/******************************************************************************************************************
	 * Gets whether the 2D renderer was initialized.
	 *
	 * @return True if the 2D renderer was initialized, and false otherwise.
	 ******************************************************************************************************************/
	bool renderer2DActive() noexcept;

	/******************************************************************************************************************
	 * Gets a reference to the 2D renderer.
	 *
	 * @pre The 2D renderer must be instantiated.
	 *
	 * @return A reference to the 2D renderer.
	 ******************************************************************************************************************/
	Renderer2D& renderer2D() noexcept;

	/// @}
} // namespace tre