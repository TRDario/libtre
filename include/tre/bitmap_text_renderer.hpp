#pragma once
#include "atlas.hpp"
#include "text.hpp"
#include <tref/tref.hpp>

namespace tre {
	/** @addtogroup text
	 *  @{
	 */

	/******************************************************************************************************************
	 * Renderer for text using glyphs from a bitmap.
	 *
	 * Implemented as an abstraction layer that forwards its output to the 2D renderer.
	 *
	 * Only one instance of the bitmap text renderer is allowed to exist at a time.
	 ******************************************************************************************************************/
	class BitmapTextRenderer {
	  public:
		using Glyph = tref::Glyph;

		/**************************************************************************************************************
		 * Bitmap text textbox rectangle.
		 **************************************************************************************************************/
		struct Textbox {
			/**********************************************************************************************************
			 * The position of the textbox.
			 **********************************************************************************************************/
			glm::vec2 pos;

			/**********************************************************************************************************
			 * Where the position of the texbox is relative to the top-left of the textbox.
			 **********************************************************************************************************/
			glm::vec2 posAnchor;

			/**********************************************************************************************************
			 * The size of the textbox.
			 **********************************************************************************************************/
			glm::vec2 size;

			/**********************************************************************************************************
			 * The rotation of the textbox around the position.
			 **********************************************************************************************************/
			tr::AngleF rotation;

			/**********************************************************************************************************
			 * The alignment of the text within the textbox.
			 **********************************************************************************************************/
			Align textAlignment;
		};

		/**************************************************************************************************************
		 * Supported bitmap text styles.
		 **************************************************************************************************************/
		enum class Style {
			/**********************************************************************************************************
			 * Regular text.
			 **********************************************************************************************************/
			NORMAL,

			/**********************************************************************************************************
			 * Italic text.
			 **********************************************************************************************************/
			ITALIC
		};

		/**************************************************************************************************************
		 * Shorthand for the glyph map used by bitmap text fonts.
		 **************************************************************************************************************/
		using GlyphMap = std::unordered_map<std::uint32_t, Glyph>;

		/**************************************************************************************************************
		 * Constructs the bitmap text renderer.
		 **************************************************************************************************************/
		BitmapTextRenderer() noexcept;

		~BitmapTextRenderer() noexcept;

		/**************************************************************************************************************
		 * Adds a font to the renderer.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 * @exception tr::BitmapBadAlloc if an internal allocation fails.
		 *
		 * @param[in] name The name of the font. A font with this name cannot already exist.
		 * @param[in] texture The texture used by the font.
		 * @param[in] lineSkip The distance between two consecutive lines.
		 * @param[in] glyphs A map between unicode codepoints and glyph information.
		 **************************************************************************************************************/
		void addFont(std::string name, tr::SubBitmap texture, std::int32_t lineSkip, GlyphMap glyphs);

		/**************************************************************************************************************
		 * Loads a font file and adds the font to the renderer.
		 *
		 * Font files are stored in the .tref format, see @ref tref for information about compiling font files.
		 *
		 * @exception tr::FileNotFound If the file was not found.
		 * @exception tr::FileOpenError If opening the file fails.
		 * @exception tref::DecodingError If decoding the file fails.
		 * @exception std::bad_alloc If an internal allocation fails.
		 * @exception tr::BitmapBadAlloc if an internal allocation fails.
		 *
		 * @param[in] name The name of the font.
		 * @param[in] path The path to the font file.
		 **************************************************************************************************************/
		void loadFont(std::string name, const std::filesystem::path& path);

		/**************************************************************************************************************
		 * Removes a font from the renderer.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] name The name of the font to remove.
		 **************************************************************************************************************/
		void removeFont(std::string_view name);

		/**************************************************************************************************************
		 * Removes all fonts from the renderer.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 **************************************************************************************************************/
		void clearFonts();

		/**************************************************************************************************************
		 * Adds a glyph to the 2D renderer.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the text (higher is drawn on top).
		 * @param[in] codepoint The unicode codepoint to draw (newlines are allowed).
		 * @param[in] font The name of the font to use. If the font isn't found, no text is drawn.
		 * @param[in] style The style of the text to use.
		 * @param[in] scale The scale of the text.
		 * @param[in] tint The tint of the glyph.
		 * @param[in] pos The position of the glyph.
		 * @param[in] posAnchor Where the position of the glyph is relative to the top-left of the glyph.
		 * @param[in] rotation The rotation of the glyph around the position.
		 **************************************************************************************************************/
		void addGlyph(int priority, std::uint32_t codepoint, std::string_view font, Style style, glm::vec2 scale,
					  tr::RGBA8 tint, glm::vec2 pos, glm::vec2 posAnchor, tr::AngleF rotation);

		/**************************************************************************************************************
		 * Adds unformatted, single-style text to the 2D renderer.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the text (higher is drawn on top).
		 * @param[in] text The text to draw (newlines are allowed).
		 * @param[in] font The name of the font to use. If the font isn't found, no text is drawn.
		 * @param[in] style The style of the text to use.
		 * @param[in] scale The scale of the text.
		 * @param[in] tint The tint of the text.
		 * @param[in] textbox The textbox to frame the text around.
		 **************************************************************************************************************/
		void addUnformatted(int priority, std::string_view text, std::string_view font, Style style, glm::vec2 scale,
							tr::RGBA8 tint, const Textbox& textbox);

		/**************************************************************************************************************
		 * Adds formatted, multistyle text to the 2D renderer.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the text (higher is drawn on top).
		 * @param[in] text The text to draw (newlines are allowed). See @ref renderformat for the specifics of the text
		 *                 format.
		 * @param[in] font The name of the font to use. If the font isn't found, no text is drawn.
		 * @param[in] scale The scale of the text.
		 * @param[in] colors The available text colors. By default, a white tint not in the span is used.
		 * @param[in] textbox The textbox to frame the text around.
		 **************************************************************************************************************/
		void addFormatted(int priority, std::string_view text, std::string_view font, glm::vec2 scale,
						  std::span<tr::RGBA8> colors, const Textbox& textbox);

	  private:
		struct Font {
			std::int32_t lineSkip;
			GlyphMap     glyphs;
		};
		struct CachedRotationTransform {
			glm::vec2  pos{};
			tr::AngleF rotation{};
			glm::mat4  transform{};
		};

		DynAtlas2D              _atlas;
		tr::StringHashMap<Font> _fonts;
		CachedRotationTransform _cachedRotationTransform;

		void addGlyph(int priority, std::uint32_t codepoint, const Font& font, tr::RectF2 fontUV, Style style,
					  glm::vec2 scale, tr::RGBA8 tint, glm::vec2 pos, glm::vec2 posAnchor, tr::AngleF rotation);
	};

	/******************************************************************************************************************
	 * Gets whether the bitmap text renderer was initialized.
	 *
	 * @return True if the bitmap text renderer was initialized, and false otherwise.
	 ******************************************************************************************************************/
	bool bitmapTextRendererActive() noexcept;

	/******************************************************************************************************************
	 * Gets a reference to the bitmap text renderer.
	 * This function cannot be called if the bitmap text renderer wasn't initialized.
	 *
	 * @return A reference to the bitmap text renderer.
	 ******************************************************************************************************************/
	BitmapTextRenderer& bitmapTextRenderer() noexcept;

	/// @}
} // namespace tre