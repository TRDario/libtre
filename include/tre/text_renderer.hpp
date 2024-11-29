/**********************************************************************************************************************
 * @file text_renderer.hpp
 * @brief Provides text rendering support.
 **********************************************************************************************************************/

#include "atlas.hpp"
#include "renderer_2d.hpp"

namespace tre {
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
	 * Renderer for persistent text.
	 *
	 * Implemented as an abstraction layer that forwards its output to the 2D renderer.
	 ******************************************************************************************************************/
	class StaticTextRenderer {
	  public:
		/******************************************************************************************************************
		 * Static text textbox rectangle.
		 ******************************************************************************************************************/
		struct Textbox {
			/**************************************************************************************************************
			 * The position of the textbox.
			 **************************************************************************************************************/
			glm::vec2 pos;

			/**************************************************************************************************************
			 * Where the position of the texbox is relative to the top-left of the textbox.
			 **************************************************************************************************************/
			glm::vec2 posAnchor;

			/**************************************************************************************************************
			 * The height of the textbox. (The width is locked at the maxWidth of the used entry)
			 **************************************************************************************************************/
			float height;

			/**************************************************************************************************************
			 * The rotation of the textbox around the position.
			 **************************************************************************************************************/
			tr::AngleF rotation;

			/**************************************************************************************************************
			 * The vertical alignment of the text within the textbox (The horizontal alignment is locked for any given
			 * entry from creation).
			 **************************************************************************************************************/
			VerticalAlign textAlignment;

			/**************************************************************************************************************
			 * The tint of the text in the textbox.
			 **************************************************************************************************************/
			tr::RGBA8 tint;
		};

		/**************************************************************************************************************
		 * Constructs the static text renderer.
		 **************************************************************************************************************/
		StaticTextRenderer() noexcept;

		/**************************************************************************************************************
		 * Sets the DPI of the renderer.
		 *
		 * This function cannot be called if any text has been added, but not forwarded to the 2D renderer.
		 *
		 * @param dpi The new text DPI, by default it is 72.
		 **************************************************************************************************************/
		void setDPI(unsigned int dpi) noexcept;

		/**************************************************************************************************************
		 * Sets the DPI of the renderer.
		 *
		 * This function clears all existing entries unless the DPI matches the previous DPI.
		 *
		 * @param dpi The new text DPI, by default it is {72, 72}.
		 **************************************************************************************************************/
		void setDPI(glm::uvec2 dpi) noexcept;

		/**************************************************************************************************************
		 * Adds an unformatted, single-style text entry.
		 *
		 * @exception tr::BitmapBadAlloc If an internal allocation failed.
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param name The name of the text entry. An entry of the same name must not exist already, otherwise a failed
		 *             assertion may be triggered.
		 * @param text The text string.
		 * @param font The font to use for the text.
		 * @param fontSize The font size to use for the text.
		 * @param style The font style to use for the text.
		 * @param textColor The color to use for the text.
		 * @param outline The outline parameters to use for the text, or NO_OUTLINE.
		 * @param maxWidth The maximum width of the entry.
		 * @param alignment The horizontal alignment of the entry.
		 **************************************************************************************************************/
		void newUnformattedEntry(std::string name, const char* text, tr::TTFont& font, int fontSize,
								 tr::TTFont::Style style, tr::RGBA8 textColor, TextOutline outline, int maxWidth,
								 HorizontalAlign alignment);

		/**************************************************************************************************************
		 * Adds an unformatted, single-style text entry.
		 *
		 * @exception tr::BitmapBadAlloc If an internal allocation failed.
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param name The name of the text entry. An entry of the same name must not exist already, otherwise a failed
		 *             assertion may be triggered.
		 * @param text The text string.
		 * @param font The font to use for the text.
		 * @param fontSize The font size to use for the text.
		 * @param style The font style to use for the text.
		 * @param textColor The color to use for the text.
		 * @param outline The outline parameters to use for the text, or NO_OUTLINE.
		 * @param maxWidth The maximum width of the entry.
		 * @param alignment The horizontal alignment of the entry.
		 **************************************************************************************************************/
		void newUnformattedEntry(std::string name, const std::string& text, tr::TTFont& font, int fontSize,
								 tr::TTFont::Style style, tr::RGBA8 textColor, TextOutline outline, int maxWidth,
								 HorizontalAlign alignment);

		/**************************************************************************************************************
		 * Adds a formatted, multistyle text entry.
		 *
		 * @exception tr::BitmapBadAlloc If an internal allocation failed.
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param name The name of the text entry. An entry of the same name must not exist already, otherwise a failed
		 *             assertion may be triggered.
		 * @param text The text string. See @ref renderformat for the specifics of the text format.
		 * @param font The font to use for the text.
		 * @param fontSize The font size to use for the text.
		 * @param textColor The color to use for the text.
		 * @param outline The outline parameters to use for the text, or NO_OUTLINE.
		 * @param maxWidth The maximum width of the entry.
		 * @param alignment The horizontal alignment of the entry.
		 **************************************************************************************************************/
		void newFormattedEntry(std::string name, std::string_view text, tr::TTFont& font, int fontSize,
							   tr::RGBA8 textColor, TextOutline outline, int maxWidth, HorizontalAlign alignment);

		/**************************************************************************************************************
		 * Adds a formatted, multistyle text entry.
		 *
		 * @exception tr::BitmapBadAlloc If an internal allocation failed.
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param name The name of the text entry. An entry of the same name must not exist already, otherwise a failed
		 *             assertion may be triggered.
		 * @param text The text string. See @ref renderformat for the specifics of the text format.
		 * @param font The font to use for the text.
		 * @param fontSize The font size to use for the text.
		 * @param textColors The available text colors. By default, the color at index 0 is used.
		 *                   Must not be empty, otherwise a failed assertion may be triggered.
		 * @param outline The outline parameters to use for the text, or NO_OUTLINE.
		 * @param maxWidth The maximum width of the entry.
		 * @param alignment The horizontal alignment of the entry.
		 **************************************************************************************************************/
		void newFormattedEntry(std::string name, std::string_view text, tr::TTFont& font, int fontSize,
							   std::span<tr::RGBA8> textColors, TextOutline outline, int maxWidth,
							   HorizontalAlign alignment);

		/**************************************************************************************************************
		 * Removes a text entry from the renderer.
		 *
		 * @param name The name of the entry to remove.
		 **************************************************************************************************************/
		void removeEntry(std::string_view name) noexcept;

		/**************************************************************************************************************
		 * Adds an instance of a text entry.
		 *
		 * @param priority The drawing priority of the text (higher is drawn on top).
		 * @param entry The name of the text entry to use. If no such entry exists, no instances will be added.
		 * @param textbox The textbox to frame the text around.
		 **************************************************************************************************************/
		void addInstance(int priority, std::string_view entry, const Textbox& textbox);

		/**************************************************************************************************************
		 * Forwards all of the added text to the 2D renderer.
		 *
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param renderer The renderer to forward to.
		 **************************************************************************************************************/
		void forward(Renderer2D& renderer);

	  private:
		/// @cond IMPLEMENTATION
		struct FixedEntryTextboxInfo {
			int width;
			HorizontalAlign textAlignment;
		};

		struct Instance {
			Textbox textbox;
			tr::RectF2 rect;
			FixedEntryTextboxInfo fixedInfo;
		};
		/// @endcond

		DynAtlas2D _atlas;
		tr::StringHashMap<FixedEntryTextboxInfo> _fixedEntryTextboxInfo;
		std::unordered_map<int, std::vector<Instance>> _instances;
		glm::uvec2 _dpi;
	};

	/******************************************************************************************************************
	 * Renderer for one-off or frequently changing text.
	 *
	 * Implemented as an abstraction layer that forwards its output to the 2D renderer.
	 ******************************************************************************************************************/
	class DynamicTextRenderer {
	  public:
		/******************************************************************************************************************
		 * Dynamic text textbox rectangle.
		 ******************************************************************************************************************/
		struct Textbox {
			/**************************************************************************************************************
			 * The position of the textbox.
			 **************************************************************************************************************/
			glm::vec2 pos;

			/**************************************************************************************************************
			 * Where the position of the texbox is relative to the top-left of the textbox.
			 **************************************************************************************************************/
			glm::vec2 posAnchor;

			/**************************************************************************************************************
			 * The size of the textbox.
			 **************************************************************************************************************/
			glm::vec2 size;

			/**************************************************************************************************************
			 * The rotation of the textbox around the position.
			 **************************************************************************************************************/
			tr::AngleF rotation;

			/**************************************************************************************************************
			 * The alignment of the text within the textbox.
			 **************************************************************************************************************/
			Align textAlignment;
		};

		/**************************************************************************************************************
		 * Constructs the dynamic text renderer.
		 **************************************************************************************************************/
		DynamicTextRenderer() noexcept;

		/**************************************************************************************************************
		 * Sets the DPI of the renderer.
		 *
		 * This function cannot be called if any text has been added, but not forwarded to the 2D renderer.
		 *
		 * @param dpi The new text DPI, by default it is 72.
		 **************************************************************************************************************/
		void setDPI(unsigned int dpi) noexcept;

		/**************************************************************************************************************
		 * Sets the DPI of the renderer.
		 *
		 * This function cannot be called if any text has been added, but not forwarded to the 2D renderer.
		 *
		 * @param dpi The new text DPI, by default it is {72, 72}.
		 **************************************************************************************************************/
		void setDPI(glm::uvec2 dpi) noexcept;

		/**************************************************************************************************************
		 * Adds unformatted, single-style text.
		 *
		 * @exception tr::BitmapBadAlloc If an internal allocation failed.
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param priority The drawing priority of the text (higher is drawn on top).
		 * @param text The text string.
		 * @param font The font to use for the text.
		 * @param fontSize The font size to use for the text.
		 * @param style The font style to use for the text.
		 * @param textColor The color to use for the text.
		 * @param outline The outline parameters to use for the text, or NO_OUTLINE.
		 * @param textbox The textbox to frame the text around.
		 **************************************************************************************************************/
		void addUnformatted(int priority, const char* text, tr::TTFont& font, int fontSize, tr::TTFont::Style style,
							tr::RGBA8 textColor, TextOutline outline, const Textbox& textbox);

		/**************************************************************************************************************
		 * Adds unformatted, single-style text.
		 *
		 * @exception tr::BitmapBadAlloc If an internal allocation failed.
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param priority The drawing priority of the text (higher is drawn on top).
		 * @param text The text string.
		 * @param font The font to use for the text.
		 * @param fontSize The font size to use for the text.
		 * @param style The font style to use for the text.
		 * @param textColor The color to use for the text.
		 * @param outline The outline parameters to use for the text, or NO_OUTLINE.
		 * @param textbox The textbox to frame the text around.
		 **************************************************************************************************************/
		void addUnformatted(int priority, const std::string& text, tr::TTFont& font, int fontSize,
							tr::TTFont::Style style, tr::RGBA8 textColor, TextOutline outline, const Textbox& textbox);

		/**************************************************************************************************************
		 * Adds formatted, multistyle text.
		 *
		 * @exception tr::BitmapBadAlloc If an internal allocation failed.
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param priority The drawing priority of the text (higher is drawn on top).
		 * @param text The text string. See @ref renderformat for the specifics of the text format.
		 * @param font The font to use for the text.
		 * @param fontSize The font size to use for the text.
		 * @param textColor The color to use for the text.
		 * @param outline The outline parameters to use for the text, or NO_OUTLINE.
		 * @param textbox The textbox to frame the text around.
		 **************************************************************************************************************/
		void addFormatted(int priority, std::string_view text, tr::TTFont& font, int fontSize, tr::RGBA8 textColor,
						  TextOutline outline, const Textbox& textbox);

		/**************************************************************************************************************
		 * Adds formatted, multistyle text.
		 *
		 * @exception tr::BitmapBadAlloc If an internal allocation failed.
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param priority The drawing priority of the text (higher is drawn on top).
		 * @param text The text string. See @ref renderformat for the specifics of the text format.
		 * @param font The font to use for the text.
		 * @param fontSize The font size to use for the text.
		 * @param textColors The available text colors. By default, the color at index 0 is used.
		 *                   Must not be empty, otherwise a failed assertion may be triggered.
		 * @param outline The outline parameters to use for the text, or NO_OUTLINE.
		 * @param textbox The textbox to frame the text around.
		 **************************************************************************************************************/
		void addFormatted(int priority, std::string_view text, tr::TTFont& font, int fontSize,
						  std::span<tr::RGBA8> textColors, TextOutline outline, const Textbox& textbox);

		/**************************************************************************************************************
		 * Forwards all of the added text to the 2D renderer.
		 *
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param renderer The renderer to forward to.
		 **************************************************************************************************************/
		void forward(Renderer2D& renderer);

	  private:
		DynAtlas2D _atlas;
		std::unordered_map<int, std::vector<Textbox>> _textboxes;
		glm::uvec2 _dpi;
	};
} // namespace tre