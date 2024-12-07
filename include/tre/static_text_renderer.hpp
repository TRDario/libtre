#pragma once
#include "atlas.hpp"
#include "text.hpp"

namespace tre {
	/** @addtogroup text
	 *  @{
	 */

	/******************************************************************************************************************
	 * Renderer for persistent text.
	 *
	 * Implemented as an abstraction layer that forwards its output to the 2D renderer.
	 *
	 * Only one instance of the static text renderer is allowed to exist at a time.
	 ******************************************************************************************************************/
	class StaticTextRenderer {
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
		 * Constructs the static text renderer.
		 **************************************************************************************************************/
		StaticTextRenderer() noexcept;

		~StaticTextRenderer() noexcept;

		/**************************************************************************************************************
		 * Sets the DPI of the renderer.
		 *
		 * @param[in] dpi The new text DPI, by default it is 72.
		 **************************************************************************************************************/
		void setDPI(unsigned int dpi) noexcept;

		/**************************************************************************************************************
		 * Sets the DPI of the renderer.
		 *
		 * This function clears all existing entries unless the DPI matches the previous DPI.
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
		 * @param[in] name The name of the text entry. An entry of the same name must not exist already.
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
		 * @param[in] name The name of the text entry. An entry of the same name must not exist already.
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
		 * @param[in] name The name of the text entry. An entry of the same name must not exist already.
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
		 * @param[in] name The name of the text entry. An entry of the same name must not exist already.
		 * @param[in] text The text string. See @ref renderformat for the specifics of the text format.
		 * @param[in] font The font to use for the text.
		 * @param[in] fontSize The font size to use for the text.
		 * @param[in] textColors The available text colors. By default, the color at index 0 is used. Must not be empty.
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
		 * Adds an instance of a text entry to the 2D renderer.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the text (higher is drawn on top).
		 * @param[in] entry The name of the text entry to use. If no such entry exists, no instances will be added.
		 * @param[in] textbox The textbox to frame the text around.
		 **************************************************************************************************************/
		void addInstance(int priority, std::string_view entry, const Textbox& textbox);

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
	 * Gets whether the static text renderer was initialized.
	 *
	 * @return True if the static text renderer was initialized, and false otherwise.
	 ******************************************************************************************************************/
	bool staticTextRendererActive() noexcept;

	/******************************************************************************************************************
	 * Gets a reference to the static text renderer.
	 * This function cannot be called if the static text renderer wasn't initialized.
	 *
	 * @return A reference to the static text renderer.
	 ******************************************************************************************************************/
	StaticTextRenderer& staticTextRenderer() noexcept;

	/// @}
} // namespace tre