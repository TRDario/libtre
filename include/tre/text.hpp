#pragma once
#include <tr/tr.hpp>

namespace tre {
	/** @defgroup text Text
	 *  Text-related functionality.
	 *  @{
	 */

	/**************************************************************************************************************
	 * Horizontal text alignment types.
	 **************************************************************************************************************/
	enum class HorizontalAlign : std::uint8_t {
		/**********************************************************************************************************
		 * The text is is placed left-aligned from the left edge of the textbox.
		 **********************************************************************************************************/
		LEFT,

		/**********************************************************************************************************
		 * The text is is placed center-aligned from the center of the textbox.
		 **********************************************************************************************************/
		CENTER,

		/**********************************************************************************************************
		 * The text is is placed right-aligned from the right edge of the textbox.
		 **********************************************************************************************************/
		RIGHT
	};

	/**************************************************************************************************************
	 * Vertical text alignment types.
	 **************************************************************************************************************/
	enum class VerticalAlign : std::uint8_t {
		/**********************************************************************************************************
		 * The text is is placed top-aligned from the top edge of the textbox.
		 **********************************************************************************************************/
		TOP,

		/**********************************************************************************************************
		 * The text is is placed center-aligned from the center of the textbox.
		 **********************************************************************************************************/
		CENTER = 3,

		/**********************************************************************************************************
		 * The text is is placed bottom-aligned from the bottom edge of the textbox.
		 **********************************************************************************************************/
		BOTTOM = 6
	};

	/**************************************************************************************************************
	 * Text alignment types.
	 **************************************************************************************************************/
	enum class Align : std::uint8_t {
		/**********************************************************************************************************
		 * The text is aligned from the top vertically and from the left horizontally.
		 **********************************************************************************************************/
		TOP_LEFT,

		/**********************************************************************************************************
		 * The text is aligned from the top vertically and from the center horizontally.
		 **********************************************************************************************************/
		TOP_CENTER,

		/**********************************************************************************************************
		 * The text is aligned from the top vertically and from the right horizontally.
		 **********************************************************************************************************/
		TOP_RIGHT,

		/**********************************************************************************************************
		 * The text is aligned from the center vertically and from the left horizontally.
		 **********************************************************************************************************/
		CENTER_LEFT,

		/**********************************************************************************************************
		 * The text is aligned from the center vertically and from the center horizontally.
		 **********************************************************************************************************/
		CENTER,

		/**********************************************************************************************************
		 * The text is aligned from the center vertically and from the right horizontally.
		 **********************************************************************************************************/
		CENTER_RIGHT,

		/**********************************************************************************************************
		 * The text is aligned from the bottom vertically and from the left horizontally.
		 **********************************************************************************************************/
		BOTTOM_LEFT,

		/**********************************************************************************************************
		 * The text is aligned from the bottom vertically and from the center horizontally.
		 **********************************************************************************************************/
		BOTTOM_CENTER,

		/**********************************************************************************************************
		 * The text is aligned from the bottom vertically and from the right horizontally.
		 **********************************************************************************************************/
		BOTTOM_RIGHT
	};

	/******************************************************************************************************************
	 * Text outline settings.
	 ******************************************************************************************************************/
	struct TextOutline {
		/**************************************************************************************************************
		 * The thickness of the outline.
		 **************************************************************************************************************/
		int thickness;

		/**************************************************************************************************************
		 * The color of the outline.
		 **************************************************************************************************************/
		tr::RGBA8 color;
	};

	/******************************************************************************************************************
	 * Special text outline value that represents a lack of an outline.
	 ******************************************************************************************************************/
	inline constexpr TextOutline NO_OUTLINE{0, {0, 0, 0, 0}};

	/******************************************************************************************************************
	 * Renders text to a bitmap according to the format described in @ref renderformat.
	 *
	 * @par Exception Safety
	 *
	 * Strong exception guarantee.
	 *
	 * @exception std::bad_alloc If an internal allocation occurs.
	 * @exception tr::BitmapBadAlloc If allocating a bitmap fails.
	 *
	 * @param[in] text The text to render. Must not be an empty string or one solely consisting of control sequences.
	 * @param[in] font The font to use in rendering.
	 * @param[in] size The size of the font.
	 * @param[in] dpi The dpi to use for the text.
	 * @param[in] maxWidth The width limit of the resulting bitmap.
	 * @param[in] alignment The alignment of the text within the bitmap.
	 * @param[in] textColors The colors to use when rendering the text. The span cannot be empty.
	 *                       By default, the color at index 0 is used.
	 * @param[in] outline The outline parameters to use for the text, or NO_OUTLINE.
	 *
	 * @return A bitmap containing the text.
	 ******************************************************************************************************************/
	tr::Bitmap renderMultistyleText(std::string_view text, tr::TTFont& font, int size, glm::uvec2 dpi, int maxWidth,
									HorizontalAlign alignment, std::span<tr::RGBA8> textColors, TextOutline outline);

	/// @}
} // namespace tre