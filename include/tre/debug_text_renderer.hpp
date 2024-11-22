/**********************************************************************************************************************
 * @file debug_text_renderer.hpp
 * @brief Provides a debug text renderer.
 **********************************************************************************************************************/

#pragma once
#include <tr/tr.hpp>

namespace tre {
	/******************************************************************************************************************
	 * Debug text renderer.
	 *
	 * The renderer is conceptualized around two independent "pens", a left-aligned and right-aligned one, writing text
	 * sequentially. Writing uses special text formatting, the specifics of which you can see at @ref debformat.
	 ******************************************************************************************************************/
	class DebugTextRenderer {
	  public:
		/**************************************************************************************************************
		 * The unique renderer ID of this renderer.
		 **************************************************************************************************************/
		static constexpr std::uint32_t ID{-3U};

		/**************************************************************************************************************
		 * Text alignment types.
		 **************************************************************************************************************/
		enum class Align : bool {
			/**********************************************************************************************************
			 * The text is is placed left-aligned from the left edge of the screen.
			 **********************************************************************************************************/
			LEFT,

			/**********************************************************************************************************
			 * The text is is placed right-aligned from the right edge of the screen.
			 **********************************************************************************************************/
			RIGHT
		};

		/**************************************************************************************************************
		 * Shorthand for the commonly used white color.
		 **************************************************************************************************************/
		static constexpr tr::RGBA8 WHITE{255, 255, 255, 255};

		/**************************************************************************************************************
		 * Shorthand for the commonly used transparent color.
		 **************************************************************************************************************/
		static constexpr tr::RGBA8 NONE{0, 0, 0, 0};

		/**************************************************************************************************************
		 * Constructs the debug text renderer.
		 *
		 * @exception tr::GLBufferBadAlloc If an internal allocation failed.
		 **************************************************************************************************************/
		DebugTextRenderer();

		/**************************************************************************************************************
		 * Sets the text's drawing scale.
		 *
		 * @param scale The scale of the text. Glyphs are 8x8 on x1.0 scale.
		 **************************************************************************************************************/
		void setScale(float scale) noexcept;

		/**************************************************************************************************************
		 * Sets the text's column limit.
		 *
		 * Setting this in the middle of writing will leave any existing text as-is.
		 *
		 * @param scale The maximum allowed number of columns tolerated before a break is needed.
		 **************************************************************************************************************/
		void setColumnLimit(std::uint8_t columns) noexcept;

		/**************************************************************************************************************
		 * Clears any written text.
		 **************************************************************************************************************/
		void clear() noexcept;

		/**************************************************************************************************************
		 * Writes a line of formatted text.
		 *
		 * For specifics of the text format, see @ref debformat.
		 *
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param text The text to write.
		 * @param textColor The default text color.
		 * @param backgroundColor The default background color.
		 * @param extraColors Additional colors used by the text.
		 * @param alignment Whether to draw the text left- or right-aligned.
		 **************************************************************************************************************/
		void write(std::string_view text, tr::RGBA8 textColor = WHITE, tr::RGBA8 backgroundColor = NONE,
				   std::span<tr::RGBA8> extraColors = {}, Align alignment = Align::LEFT);

		/**************************************************************************************************************
		 * Draws all written text to a target.
		 *
		 * The text will remain drawable until a call to clear().
		 *
		 * @exception tr::GLBufferBadAlloc If an internal allocation failed.
		 *
		 * @param glContext The OpenGL context to manipulate.
		 * @param target The drawing target.
		 **************************************************************************************************************/
		void draw(tr::GLContext& glContext, tr::BasicFramebuffer& target);

	  private:
		/// @cond IMPLEMENTATION
		struct ShaderGlyph {
			glm::u8vec2 pos;
			bool alignRight;
			char chr;
			tr::RGBA8 textColor;
			tr::RGBA8 backgroundColor;
		};
		/// @endcond

		tr::OwningShaderPipeline _shaderPipeline;
		tr::ShaderBuffer _shaderGlyphBuffer;
		tr::Texture2D _font;
		tr::Sampler _sampler;
		tr::TextureUnit _textureUnit;
		tr::VertexBuffer _vertexBuffer;
		tr::VertexFormat _vertexFormat;

		std::uint8_t _columnLimit;
		std::uint8_t _leftLine;
		std::uint8_t _rightLine;
		std::vector<ShaderGlyph> _shaderGlyphs;

		void writeRegularChar(char chr, std::uint8_t& line, std::uint8_t& lineLength,
							  std::optional<decltype(_shaderGlyphs)::iterator>& lineStart,
							  std::optional<decltype(_shaderGlyphs)::iterator>& wordStart, tr::RGBA8 textColor,
							  tr::RGBA8 backgroundColor, Align alignment);
		void setupContext(tr::GLContext& glContext) noexcept;
	};
} // namespace tre