#pragma once
#include "atlas.hpp"
#include "text.hpp"

namespace tre {
	/******************************************************************************************************************
	 * Renderer for one-off or frequently changing text.
	 *
	 * Implemented as an abstraction layer that forwards its output to the 2D renderer.
	 *
	 * Only one instance of the dynamic text renderer is allowed to exist at a time.
	 *
	 * @ingroup renderer text
	 ******************************************************************************************************************/
	class DynamicTextRenderer {
	  public:
		/**************************************************************************************************************
		 * Dynamic text textbox rectangle.
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
		 * Constructs the dynamic text renderer.
		 **************************************************************************************************************/
		DynamicTextRenderer() noexcept;

		~DynamicTextRenderer() noexcept;

		/**************************************************************************************************************
		 * Sets the DPI of the renderer.
		 *
		 * @param[in] dpi The new text DPI, by default it is 72.
		 **************************************************************************************************************/
		void setDPI(unsigned int dpi) noexcept;

		/**************************************************************************************************************
		 * Sets the DPI of the renderer.
		 *
		 * @param[in] dpi The new text DPI, by default it is {72, 72}.
		 **************************************************************************************************************/
		void setDPI(glm::uvec2 dpi) noexcept;

		/**************************************************************************************************************
		 * Adds unformatted, single-style text to the 2D renderer.
		 *
		 * @exception tr::BitmapBadAlloc If an internal allocation fails.
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the text (higher is drawn on top).
		 * @param[in] text The text string.
		 * @param[in] font The font to use for the text.
		 * @param[in] fontSize The font size to use for the text.
		 * @param[in] style The font style to use for the text.
		 * @param[in] textColor The color to use for the text.
		 * @param[in] outline The outline parameters to use for the text, or NO_OUTLINE.
		 * @param[in] textbox The textbox to frame the text around.
		 **************************************************************************************************************/
		void addUnformatted(int priority, const char* text, tr::TTFont& font, int fontSize, tr::TTFont::Style style,
							tr::RGBA8 textColor, TextOutline outline, const Textbox& textbox);

		/**************************************************************************************************************
		 * Adds unformatted, single-style text to the 2D renderer.
		 *
		 * @exception tr::BitmapBadAlloc If an internal allocation fails.
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the text (higher is drawn on top).
		 * @param[in] text The text string.
		 * @param[in] font The font to use for the text.
		 * @param[in] fontSize The font size to use for the text.
		 * @param[in] style The font style to use for the text.
		 * @param[in] textColor The color to use for the text.
		 * @param[in] outline The outline parameters to use for the text, or NO_OUTLINE.
		 * @param[in] textbox The textbox to frame the text around.
		 **************************************************************************************************************/
		void addUnformatted(int priority, const std::string& text, tr::TTFont& font, int fontSize,
							tr::TTFont::Style style, tr::RGBA8 textColor, TextOutline outline, const Textbox& textbox);

		/**************************************************************************************************************
		 * Adds formatted, multistyle text to the 2D renderer.
		 *
		 * @exception tr::BitmapBadAlloc If an internal allocation fails.
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the text (higher is drawn on top).
		 * @param[in] text The text string. See @ref renderformat for the specifics of the text format.
		 * @param[in] font The font to use for the text.
		 * @param[in] fontSize The font size to use for the text.
		 * @param[in] textColor The color to use for the text.
		 * @param[in] outline The outline parameters to use for the text, or NO_OUTLINE.
		 * @param[in] textbox The textbox to frame the text around.
		 **************************************************************************************************************/
		void addFormatted(int priority, std::string_view text, tr::TTFont& font, int fontSize, tr::RGBA8 textColor,
						  TextOutline outline, const Textbox& textbox);

		/**************************************************************************************************************
		 * Adds formatted, multistyle text to the 2D renderer.
		 *
		 * @exception tr::BitmapBadAlloc If an internal allocation fails.
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] priority The drawing priority of the text (higher is drawn on top).
		 * @param[in] text The text string. See @ref renderformat for the specifics of the text format.
		 * @param[in] font The font to use for the text.
		 * @param[in] fontSize The font size to use for the text.
		 * @param[in] textColors The available text colors. By default, the color at index 0 is used. Must not be empty.
		 * @param[in] outline The outline parameters to use for the text, or NO_OUTLINE.
		 * @param[in] textbox The textbox to frame the text around.
		 **************************************************************************************************************/
		void addFormatted(int priority, std::string_view text, tr::TTFont& font, int fontSize,
						  std::span<tr::RGBA8> textColors, TextOutline outline, const Textbox& textbox);

		/**************************************************************************************************************
		 * Prepares the renderer for a new frame.
		 *
		 * @warning This function should only be called after all previously outputted text was rendered!
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 **************************************************************************************************************/
		void newFrame();

	  private:
		DynAtlas2D _atlas;
		glm::uvec2 _dpi;

		void addToRenderer(int priority, const std::string& name, const Textbox& textbox);
	};

	/******************************************************************************************************************
	 * Gets whether the dynamic text renderer was initialized.
	 *
	 * @return True if the dynamic text renderer was initialized, and false otherwise.
	 *
	 * @ingroup renderer text
	 ******************************************************************************************************************/
	bool dynamicTextRendererActive() noexcept;

	/******************************************************************************************************************
	 * Gets a reference to the dynamic text renderer.
	 * This function cannot be called if the dynamic text renderer wasn't initialized.
	 *
	 * @return A reference to the dynamic text renderer.
	 *
	 * @ingroup renderer text
	 ******************************************************************************************************************/
	DynamicTextRenderer& dynamicTextRenderer() noexcept;
} // namespace tre