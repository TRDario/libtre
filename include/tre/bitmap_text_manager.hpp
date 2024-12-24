#pragma once
#include "atlas.hpp"
#include "renderer_2d.hpp"
#include "text.hpp"
#include <tref/tref.hpp>

namespace tre {
	/** @ingroup text
	 *  @defgroup bitmap_text Bitmap Text
	 *  Bitmap text rendering functionality.
	 *
	 *  @{
	 */

	/******************************************************************************************************************
	 * Texture and mesh manager for bitmapped text.
	 *
	 * The BitmapTextManager class uses something akin to the singleton pattern. It is still your job to instantiate the
	 * renderer once (and only once!), after which it will stay active until its destructor is called, but this instance
	 * will be globally available through bitmapText(). Instancing the renderer again after it has been closed is a
	 * valid action.
	 *
	 * BitmapTextManager is move-constructible, but neither copyable nor assignable. A moved renderer is left in a state
	 * where another renderer can be moved into it, but is otherwise unusable.
	 *
	 * @note An instance of tr::Window must be created before BitmapTextManager can be instantiated.
	 ******************************************************************************************************************/
	class BitmapTextManager {
	  public:
		using Glyph    = tref::Glyph;
		using GlyphMap = tref::GlyphMap;

		/**************************************************************************************************************
		 * Bitmap font information.
		 **************************************************************************************************************/
		struct Font {
			/**********************************************************************************************************
			 * The distance between two lines.
			 **********************************************************************************************************/
			std::int32_t lineSkip;

			/**********************************************************************************************************
			 * The glyphs of the font.
			 **********************************************************************************************************/
			GlyphMap glyphs;

			/**********************************************************************************************************
			 * Determines if a glyph has an associated texture to it when drawing (i.e: it is not whitespace).
			 *
			 * @param codepoint The codepoint to check, doesn't have to be in the font.
			 *
			 * @return False if the glyph is whitespace,and true otherwise. The fallback glyph is checked if @em
			 *         codepoint isn't in the glyph.
			 **********************************************************************************************************/
			bool glyphDrawable(std::uint32_t codepoint) const noexcept;
		};

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
		 * Shorthand for the glyph mesh output type.
		 **************************************************************************************************************/
		using GlyphMesh = Renderer2D::TextureQuad;

		/**************************************************************************************************************
		 * The text mesh.
		 **************************************************************************************************************/
		struct Mesh {
			/**********************************************************************************************************
			 * The vertices of the mesh.
			 **********************************************************************************************************/
			std::vector<tr::TintVtx2> vertices;

			/**********************************************************************************************************
			 * The indices of the mesh.
			 **********************************************************************************************************/
			std::vector<std::uint16_t> indices;
		};

		/**************************************************************************************************************
		 * Constructs the bitmap text manager.
		 **************************************************************************************************************/
		BitmapTextManager() noexcept;

		/**************************************************************************************************************
		 * Move-constructs a bitmap text manager.
		 *
		 * @param[in] r The bitmap text manager to move from. @em r will be left in a moved-from state that shouldn't
		 *              be used.
		 **************************************************************************************************************/
		BitmapTextManager(BitmapTextManager&& r) noexcept;

		/**************************************************************************************************************
		 * Destroys the bitmap text renderer and disables the ability to use the bitmapText() getter.
		 **************************************************************************************************************/
		~BitmapTextManager() noexcept;

		/**************************************************************************************************************
		 * Gets a reference to the manager's texture atlas.
		 *
		 * @return An immutable reference to the manager's texture atlas.
		 **************************************************************************************************************/
		const tr::Texture2D& texture() const noexcept;

		/**************************************************************************************************************
		 * Gets font information.
		 *
		 * @param name
		 * @parblock
		 * The name of the font.
		 *
		 * @pre @em name must be a valid font name.
		 * @endparblock
		 *
		 * @return A constant reference to the font information.
		 **************************************************************************************************************/
		const Font& font(std::string_view name) const noexcept;

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
		 * Creates a glyph mesh.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] codepoint
		 * @parblock
		 * The unicode codepoint to draw (newlines are allowed).
		 *
		 * @pre @em codepoint (or '\0' if @em codepoint is not in the font) must not be whitespace.
		 * @endparblock
		 * @param[in] font The name of the font to use. If the font isn't found, no text is drawn.
		 * @param[in] style The style of the text to use.
		 * @param[in] scale The scale of the text.
		 * @param[in] tint The tint of the glyph.
		 * @param[in] pos The position of the glyph.
		 * @param[in] posAnchor Where the position of the glyph is relative to the top-left of the glyph.
		 * @param[in] rotation The rotation of the glyph around the position.
		 *
		 * @return The glyph's quad.
		 **************************************************************************************************************/
		GlyphMesh createGlyphMesh(std::uint32_t codepoint, std::string_view font, Style style, glm::vec2 scale,
								  tr::RGBA8 tint, glm::vec2 pos, glm::vec2 posAnchor, tr::AngleF rotation);

		/**************************************************************************************************************
		 * Creates a mesh for unformatted, single-style text.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] text The text to draw (newlines are allowed).
		 * @param[in] font
		 * @parblock
		 * The name of the font to use.
		 *
		 * @pre @em font must be a valid font name.
		 * @endparblock
		 * @param[in] style The style of the text to use.
		 * @param[in] scale The scale of the text.
		 * @param[in] tint The tint of the text.
		 * @param[in] textbox The textbox to frame the text around.
		 *
		 * @return A text mesh.
		 **************************************************************************************************************/
		Mesh createUnformattedTextMesh(std::string_view text, std::string_view font, Style style, glm::vec2 scale,
									   tr::RGBA8 tint, const Textbox& textbox);

		/**************************************************************************************************************
		 * Creates a mesh for formatted, multistyle text.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] text The text to draw (newlines are allowed). See @ref renderformat for the specifics of the text
		 *                 format.
		 * @param[in] font
		 * @parblock
		 * The name of the font to use.
		 *
		 * @pre @em font must be a valid font name.
		 * @endparblock
		 * @param[in] scale The scale of the text.
		 * @param[in] colors The available text colors. By default, a white tint not in the span is used.
		 * @param[in] textbox The textbox to frame the text around.
		 *
		 * @return A text mesh.
		 **************************************************************************************************************/
		Mesh createFormattedTextMesh(std::string_view text, std::string_view font, glm::vec2 scale,
									 std::span<tr::RGBA8> colors, const Textbox& textbox);

	  private:
		struct CachedRotationTransform {
			glm::vec2  pos{};
			tr::AngleF rotation{};
			glm::mat4  transform{};
		};

		DynAtlas2D              _atlas;
		tr::StringHashMap<Font> _fonts;
		CachedRotationTransform _cachedRotationTransform;

		GlyphMesh createGlyphMesh(std::uint32_t codepoint, const Font& font, tr::RectF2 fontUV, Style style,
								  glm::vec2 scale, tr::RGBA8 tint, glm::vec2 pos, glm::vec2 posAnchor,
								  tr::AngleF rotation);
	};

	/******************************************************************************************************************
	 * Gets whether the bitmap text manager was initialized.
	 *
	 * @return True if the bitmap text manager was initialized, and false otherwise.
	 ******************************************************************************************************************/
	bool bitmapTextActive() noexcept;

	/******************************************************************************************************************
	 * Gets a reference to the bitmap text manager.
	 * This function cannot be called if the bitmap text manager wasn't initialized.
	 *
	 * @return A reference to the bitmap text manager.
	 ******************************************************************************************************************/
	BitmapTextManager& bitmapText() noexcept;

	/// @}
} // namespace tre
