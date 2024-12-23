#pragma once
#include "atlas.hpp"
#include "renderer_2d.hpp"
#include "text.hpp"

namespace tre {
	/** @ingroup text
	 *  @defgroup static_text Static Text
	 *  Static text management functionality.
	 *
	 *  @{
	 */

	/******************************************************************************************************************
	 * Texture and mesh manager for rarely-changing text.
	 *
	 * The StaticTextManager class uses something akin to the singleton pattern. It is still your job to instantiate the
	 * renderer once (and only once!), after which it will stay active until its destructor is called, but this instance
	 * will be globally available through staticText(). Instancing the renderer again after it has been closed is a
	 * valid action.
	 *
	 * StaticTextManager is move-constructible, but neither copyable nor assignable. A moved renderer is left in a state
	 * where another renderer can be moved into it, but is otherwise unusable.
	 *
	 * @note An instance of tr::Window must be created before StaticTextManager can be instantiated.
	 ******************************************************************************************************************/
	class StaticTextManager {
	  public:
		/**************************************************************************************************************
		 * Static text textbox rectangle.
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
			 * The height of the textbox. (The width is locked at the maxWidth of the used entry)
			 **********************************************************************************************************/
			float height;

			/**********************************************************************************************************
			 * The rotation of the textbox around the position.
			 **********************************************************************************************************/
			tr::AngleF rotation;

			/**********************************************************************************************************
			 * The vertical alignment of the text within the textbox (The horizontal alignment is locked for any given
			 * entry from creation).
			 **********************************************************************************************************/
			VerticalAlign textAlignment;

			/**********************************************************************************************************
			 * The tint of the text in the textbox.
			 **********************************************************************************************************/
			tr::RGBA8 tint;
		};

		/**************************************************************************************************************
		 * Constructs the static text manager.
		 **************************************************************************************************************/
		StaticTextManager() noexcept;

		/**************************************************************************************************************
		 * Move-constructs a static text manager.
		 *
		 * @param[in] r The static text manager to move from. @em r will be left in a moved-from state that shouldn't
		 *              be used.
		 **************************************************************************************************************/
		StaticTextManager(StaticTextManager&& r) noexcept;

		/**************************************************************************************************************
		 * Destroys the debug text renderer and disables the ability to use the staticText() getter.
		 **************************************************************************************************************/
		~StaticTextManager() noexcept;

		/**************************************************************************************************************
		 * Gets a reference to the manager's texture atlas.
		 *
		 * @return An immutable reference to the manager's texture atlas.
		 **************************************************************************************************************/
		const tr::Texture2D& texture() const noexcept;

		/**************************************************************************************************************
		 * Sets the DPI of the renderer.
		 *
		 * @note This function clears all existing entries unless the DPI matches the previous DPI.
		 *
		 * @param[in] dpi The new text DPI, by default it is 72.
		 **************************************************************************************************************/
		void setDPI(unsigned int dpi) noexcept;

		/**************************************************************************************************************
		 * Sets the DPI of the renderer.
		 *
		 * @note This function clears all existing entries unless the DPI matches the previous DPI.
		 *
		 * @param[in] dpi The new text DPI, by default it is {72, 72}.
		 **************************************************************************************************************/
		void setDPI(glm::uvec2 dpi) noexcept;

		/**************************************************************************************************************
		 * Adds an unformatted, single-style text entry.
		 *
		 * @exception tr::BitmapBadAlloc If an internal allocation fails.
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] name
		 * @parblock
		 * The name of the text entry.
		 *
		 * @pre An entry named @em name cannot already exist in the manager.
		 * @endparblock
		 * @param[in] text The text string.
		 * @param[in] font The font to use for the text.
		 * @param[in] fontSize The font size to use for the text.
		 * @param[in] style The font style to use for the text.
		 * @param[in] textColor The color to use for the text.
		 * @param[in] outline The outline parameters to use for the text, or NO_OUTLINE.
		 * @param[in] maxWidth The maximum width of the entry.
		 * @param[in] alignment The horizontal alignment of the entry.
		 **************************************************************************************************************/
		void newUnformattedEntry(std::string name, const char* text, tr::TTFont& font, int fontSize,
								 tr::TTFont::Style style, tr::RGBA8 textColor, TextOutline outline, int maxWidth,
								 HorizontalAlign alignment);

		/**************************************************************************************************************
		 * Adds an unformatted, single-style text entry.
		 *
		 * @exception tr::BitmapBadAlloc If an internal allocation fails.
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] name
		 * @parblock
		 * The name of the text entry.
		 *
		 * @pre An entry named @em name cannot already exist in the manager.
		 * @endparblock
		 * @param[in] text The text string.
		 * @param[in] font The font to use for the text.
		 * @param[in] fontSize The font size to use for the text.
		 * @param[in] style The font style to use for the text.
		 * @param[in] textColor The color to use for the text.
		 * @param[in] outline The outline parameters to use for the text, or NO_OUTLINE.
		 * @param[in] maxWidth The maximum width of the entry.
		 * @param[in] alignment The horizontal alignment of the entry.
		 **************************************************************************************************************/
		void newUnformattedEntry(std::string name, const std::string& text, tr::TTFont& font, int fontSize,
								 tr::TTFont::Style style, tr::RGBA8 textColor, TextOutline outline, int maxWidth,
								 HorizontalAlign alignment);

		/**************************************************************************************************************
		 * Adds a formatted, multistyle text entry.
		 *
		 * @exception tr::BitmapBadAlloc If an internal allocation fails.
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] name
		 * @parblock
		 * The name of the text entry.
		 *
		 * @pre An entry named @em name cannot already exist in the manager.
		 * @endparblock
		 * @param[in] text The text string. See @ref renderformat for the specifics of the text format.
		 * @param[in] font The font to use for the text.
		 * @param[in] fontSize The font size to use for the text.
		 * @param[in] textColor The color to use for the text.
		 * @param[in] outline The outline parameters to use for the text, or NO_OUTLINE.
		 * @param[in] maxWidth The maximum width of the entry.
		 * @param[in] alignment The horizontal alignment of the entry.
		 **************************************************************************************************************/
		void newFormattedEntry(std::string name, std::string_view text, tr::TTFont& font, int fontSize,
							   tr::RGBA8 textColor, TextOutline outline, int maxWidth, HorizontalAlign alignment);

		/**************************************************************************************************************
		 * Adds a formatted, multistyle text entry.
		 *
		 * @exception tr::BitmapBadAlloc If an internal allocation fails.
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] name
		 * @parblock
		 * The name of the text entry.
		 *
		 * @pre An entry named @em name cannot already exist in the manager.
		 * @endparblock
		 * @param[in] text The text string. See @ref renderformat for the specifics of the text format.
		 * @param[in] font The font to use for the text.
		 * @param[in] fontSize The font size to use for the text.
		 * @param[in] textColors
		 * @parblock
		 * The available text colors.
		 *
		 * @pre @em textColors cannot be empty.
		 * @endparblock
		 * @param[in] outline The outline parameters to use for the text, or NO_OUTLINE.
		 * @param[in] maxWidth The maximum width of the entry.
		 * @param[in] alignment The horizontal alignment of the entry.
		 **************************************************************************************************************/
		void newFormattedEntry(std::string name, std::string_view text, tr::TTFont& font, int fontSize,
							   std::span<tr::RGBA8> textColors, TextOutline outline, int maxWidth,
							   HorizontalAlign alignment);

		/**************************************************************************************************************
		 * Removes a text entry from the renderer.
		 *
		 * @param[in] name The name of the entry to remove.
		 **************************************************************************************************************/
		void removeEntry(std::string_view name) noexcept;

		/**************************************************************************************************************
		 * Creates a text entry mesh.
		 *
		 * @param[in] entry
		 * @parblock
		 * The name of the text entry to use.
		 *
		 * @pre @em entry must be a valid text entry name.
		 * @endparblock
		 * @param[in] textbox The textbox to frame the text around.
		 *
		 * @return A textured quad. When drawing, use the texture gotten from texture().
		 **************************************************************************************************************/
		Renderer2D::TextureQuad createMesh(std::string_view entry, const Textbox& textbox) noexcept;

	  private:
		struct FixedEntryTextboxInfo {
			int             width;
			HorizontalAlign textAlignment;
		};

		DynAtlas2D                               _atlas;
		tr::StringHashMap<FixedEntryTextboxInfo> _fixedEntryTextboxInfo;
		glm::uvec2                               _dpi;
	};

	/******************************************************************************************************************
	 * Gets whether the static text manager was initialized.
	 *
	 * @return True if the static text manager was initialized, and false otherwise.
	 ******************************************************************************************************************/
	bool staticTextActive() noexcept;

	/******************************************************************************************************************
	 * Gets a reference to the static text manager.
	 *
	 * @pre The static text manager must be instantiated.
	 *
	 * @return A reference to the static text manager.
	 ******************************************************************************************************************/
	StaticTextManager& staticText() noexcept;

	/// @}
} // namespace tre