#pragma once
#include <tr/tr.hpp>

namespace tre {
	/** @ingroup text
	 *  @defgroup debug_text Debug Text
	 *  Debug text rendering functionality.
	 *
	 *  @{
	 */

	/******************************************************************************************************************
	 * Debug text renderer.
	 *
	 * The renderer is conceptualized around two independent "pens", a left-aligned and right-aligned one, writing text
	 * sequentially. Writing uses special text formatting, the specifics of which can be seen at @ref debformat.
	 *
	 * The DebugTextRenderer class uses something akin to the singleton pattern. It is still your job to instantiate the
	 * renderer once (and only once!), after which it will stay active until its destructor is called, but this instance
	 * will be globally available through debugText(). Instancing the renderer again after it has been closed is a
	 * valid action.
	 *
	 * DebugTextRenderer is move-constructible, but neither copyable nor assignable. A moved renderer is left in a state
	 * where another renderer can be moved into it, but is otherwise unusable.
	 *
	 * @note An instance of tr::Window must be created before Renderer2D can be instantiated.
	 ******************************************************************************************************************/
	class DebugTextRenderer {
	  public:
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
		 * Shorthand for the commonly used red color.
		 **************************************************************************************************************/
		static constexpr tr::RGBA8 RED{255, 0, 0, 255};

		/**************************************************************************************************************
		 * Shorthand for the commonly used black color.
		 **************************************************************************************************************/
		static constexpr tr::RGBA8 BLACK{0, 0, 0, 255};

		/**************************************************************************************************************
		 * Shorthand for the commonly used transparent color.
		 **************************************************************************************************************/
		static constexpr tr::RGBA8 NONE{0, 0, 0, 0};

		/**************************************************************************************************************
		 * Constructs the debug text renderer.
		 *
		 * @par Exception Safety
		 *
		 * Strong exception guarantee.
		 *
		 * @exception tr::GLBufferBadAlloc If an internal allocation fails.
		 **************************************************************************************************************/
		DebugTextRenderer();

		/**************************************************************************************************************
		 * Move-constructs a debug text renderer.
		 *
		 * @param[in] r The debug text renderer to move from. @em r will be left in a moved-from state that shouldn't
		 *              be used.
		 **************************************************************************************************************/
		DebugTextRenderer(DebugTextRenderer&& r) noexcept;

		/**************************************************************************************************************
		 * Destroys the debug text renderer and disables the ability to use the tre::debugText() getter.
		 **************************************************************************************************************/
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
		 * @note Setting this in the middle of writing will leave any existing text as-is.
		 *
		 * @param[in] columns The maximum allowed number of columns tolerated before a break is needed.
		 **************************************************************************************************************/
		void setColumnLimit(std::uint8_t columns) noexcept;

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
		void write(std::string_view text, tr::RGBA8 textColor = WHITE, tr::RGBA8 backgroundColor = BLACK,
				   std::span<tr::RGBA8> extraColors = {}, Align alignment = Align::LEFT);

		/**************************************************************************************************************
		 * Writes benchmark data.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] benchmark The benchmark to display data from.
		 * @param[in] name The name of the benchmark, leave empty for no title.
		 * @param[in] altColorLimit The delimiting length after which text will be shown in the alt color.
		 * @param[in] textColor The primary text color.
		 * @param[in] altTextColor The secondary text color, used for durations above the altColorLimit.
		 * @param[in] backgroundColor The background color.
		 * @param[in] alignment Whether to draw the text left- or right-aligned.
		 **************************************************************************************************************/
		void write(const tr::Benchmark& benchmark, std::string_view name, tr::Duration altColorLimit,
				   tr::RGBA8 textColor = WHITE, tr::RGBA8 altTextColor = RED, tr::RGBA8 backgroundColor = BLACK,
				   Align alignment = Align::RIGHT);

		/**************************************************************************************************************
		 * Draws all written text to the screen and clears it.
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
	bool debugTextActive() noexcept;

	/******************************************************************************************************************
	 * Gets a reference to the debug text renderer.
	 *
	 * @pre The debug text renderer must be instantiated.
	 *
	 * @return A reference to the debug text renderer.
	 ******************************************************************************************************************/
	DebugTextRenderer& debugText() noexcept;

	/// @}
} // namespace tre