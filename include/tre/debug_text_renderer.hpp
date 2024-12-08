#pragma once
#include <tr/tr.hpp>

namespace tre {
	/** @addtogroup text
	 *  @{
	 */

	/******************************************************************************************************************
	 * Debug text renderer.
	 *
	 * The renderer is conceptualized around two independent "pens", a left-aligned and right-aligned one, writing text
	 * sequentially. Writing uses special text formatting, the specifics of which you can see at @ref debformat.
	 *
	 * Only one instance of the debug text renderer is allowed to exist at a time.
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
		 * @exception tr::GLBufferBadAlloc If an internal allocation fails.
		 **************************************************************************************************************/
		DebugTextRenderer();

		~DebugTextRenderer() noexcept;

		/**************************************************************************************************************
		 * Sets the text's drawing scale.
		 *
		 * @param[in] scale The scale of the text. Glyphs are 8x8 on x1.0 scale.
		 **************************************************************************************************************/
		void setScale(float scale) noexcept;

		/**************************************************************************************************************
		 * Sets the text's column limit.
		 *
		 * Setting this in the middle of writing will leave any existing text as-is.
		 *
		 * @param[in] columns The maximum allowed number of columns tolerated before a break is needed.
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
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] text The text to write.
		 * @param[in] textColor The default text color.
		 * @param[in] backgroundColor The default background color.
		 * @param[in] extraColors Additional colors used by the text.
		 * @param[in] alignment Whether to draw the text left- or right-aligned.
		 **************************************************************************************************************/
		void write(std::string_view text, tr::RGBA8 textColor = WHITE, tr::RGBA8 backgroundColor = NONE,
				   std::span<tr::RGBA8> extraColors = {}, Align alignment = Align::LEFT);

		/**************************************************************************************************************
		 * Draws all written text to the screren.
		 *
		 * The text will remain drawable until a call to clear().
		 *
		 * @exception tr::GLBufferBadAlloc If an internal allocation fails.
		 **************************************************************************************************************/
		void draw();

	  private:
		struct ShaderGlyph {
			glm::u8vec2 pos;
			bool        alignRight;
			char        chr;
			tr::RGBA8   textColor;
			tr::RGBA8   backgroundColor;
		};
		struct DebugTextContext {
			std::uint8_t& line;
			Align         alignment;
			tr::RGBA8     textColor;
			tr::RGBA8     backgroundColor;
			std::uint8_t  lineLength;
			std::size_t   textStart;
			std::size_t   lineStart;
			std::size_t   wordStart;
		};

		tr::OwningShaderPipeline _shaderPipeline;
		tr::ShaderBuffer         _shaderGlyphBuffer;
		tr::Texture2D            _font;
		tr::TextureUnit          _textureUnit;
		tr::VertexFormat         _vertexFormat;
		tr::VertexBuffer         _vertexBuffer;

		std::uint8_t             _columnLimit;
		std::uint8_t             _leftLine;
		std::uint8_t             _rightLine;
		std::vector<ShaderGlyph> _shaderGlyphs;

		void rightAlignLine(std::size_t begin, std::size_t end) noexcept;
		void trimTrailingWhitespace(DebugTextContext& context) noexcept;
		void moveCurrentWordToNextLine(DebugTextContext& context) noexcept;
		void breakAtLastWhitespace(DebugTextContext& context) noexcept;
		void breakOverlongWord(DebugTextContext& context) noexcept;
		void handleColumnLimit(DebugTextContext& context) noexcept;
		void writeCharacter(char chr, DebugTextContext& context);
		void handleNewline(DebugTextContext& context);
		void handleControlSequence(std::string_view::iterator& it, std::string_view::iterator end,
								   DebugTextContext& context, tr::RGBA8 textColor, tr::RGBA8 backgroundColor,
								   std::span<tr::RGBA8> extraColors);
		void setupContext() noexcept;
	};

	/******************************************************************************************************************
	 * Gets whether the debug text renderer was initialized.
	 *
	 * @return True if the debug text renderer was initialized, and false otherwise.
	 ******************************************************************************************************************/
	bool debugTextRendererActive() noexcept;

	/******************************************************************************************************************
	 * Gets a reference to the debug text renderer.
	 * This function cannot be called if the debug text renderer wasn't initialized.
	 *
	 * @return A reference to the debug text renderer.
	 ******************************************************************************************************************/
	DebugTextRenderer& debugTextRenderer() noexcept;

	/// @}
} // namespace tre