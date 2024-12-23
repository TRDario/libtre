#pragma once
#include "atlas.hpp"
#include "renderer_2d.hpp"
#include "text.hpp"

namespace tre {
	/** @ingroup text
	 *  @defgroup dynamic_text Dynamic Text
	 *  Dynamic text management functionality.
	 *
	 *  @{
	 */

	/******************************************************************************************************************
	 * Texture and mesh manager for frequently-changing text.
	 *
	 * The DynamicTextManager class uses something akin to the singleton pattern. It is still your job to instantiate
	 * the renderer once (and only once!), after which it will stay active until its destructor is called, but this
	 * instance will be globally available through staticText(). Instancing the renderer again after it has been closed
	 * is a valid action.
	 *
	 * DynamicTextManager is move-constructible, but neither copyable nor assignable. A moved renderer is left in a
	 * state where another renderer can be moved into it, but is otherwise unusable.
	 *
	 * @note An instance of tr::Window must be created before DynamicTextManager can be instantiated.
	 ******************************************************************************************************************/
	class DynamicTextManager {
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
		 * Constructs the dynamic text manager.
		 **************************************************************************************************************/
		DynamicTextManager() noexcept;

		/**************************************************************************************************************
		 * Move-constructs a dynamic text manager.
		 *
		 * @param[in] r The dynamic text manager to move from. @em r will be left in a moved-from state that shouldn't
		 *              be used.
		 **************************************************************************************************************/
		DynamicTextManager(DynamicTextManager&& r) noexcept;

		/**************************************************************************************************************
		 * Destroys the dynamic text renderer and disables the ability to use the dynamicText() getter.
		 **************************************************************************************************************/
		~DynamicTextManager() noexcept;

		/**************************************************************************************************************
		 * Gets a reference to the manager's texture atlas.
		 *
		 * @return An immutable reference to the manager's texture atlas.
		 **************************************************************************************************************/
		const tr::Texture2D& texture() const noexcept;

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
		 * @param[in] text
		 * @parblock
		 * The text string.
		 *
		 * @pre @em text cannot be an empty string.
		 * @endparblock
		 * @param[in] font The font to use for the text.
		 * @param[in] fontSize The font size to use for the text.
		 * @param[in] style The font style to use for the text.
		 * @param[in] textColor The color to use for the text.
		 * @param[in] outline The outline parameters to use for the text, or NO_OUTLINE.
		 * @param[in] textbox The textbox to frame the text around.
		 **************************************************************************************************************/
		Renderer2D::TextureQuad createUnformatted(const char* text, tr::TTFont& font, int fontSize,
												  tr::TTFont::Style style, tr::RGBA8 textColor, TextOutline outline,
												  const Textbox& textbox);

		/**************************************************************************************************************
		 * Adds unformatted, single-style text to the 2D renderer.
		 *
		 * @exception tr::BitmapBadAlloc If an internal allocation fails.
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] text
		 * @parblock
		 * The text string.
		 *
		 * @pre @em text cannot be an empty string.
		 * @endparblock
		 * @param[in] font The font to use for the text.
		 * @param[in] fontSize The font size to use for the text.
		 * @param[in] style The font style to use for the text.
		 * @param[in] textColor The color to use for the text.
		 * @param[in] outline The outline parameters to use for the text, or NO_OUTLINE.
		 * @param[in] textbox The textbox to frame the text around.
		 **************************************************************************************************************/
		Renderer2D::TextureQuad createUnformatted(const std::string& text, tr::TTFont& font, int fontSize,
												  tr::TTFont::Style style, tr::RGBA8 textColor, TextOutline outline,
												  const Textbox& textbox);

		/**************************************************************************************************************
		 * Adds formatted, multistyle text to the 2D renderer.
		 *
		 * @exception tr::BitmapBadAlloc If an internal allocation fails.
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] text
		 * @parblock
		 * The text string. See @ref renderformat for the specifics of the text format.
		 *
		 * @pre @em text cannot be an empty string.
		 * @endparblock
		 * @param[in] font The font to use for the text.
		 * @param[in] fontSize The font size to use for the text.
		 * @param[in] textColor The color to use for the text.
		 * @param[in] outline The outline parameters to use for the text, or NO_OUTLINE.
		 * @param[in] textbox The textbox to frame the text around.
		 **************************************************************************************************************/
		Renderer2D::TextureQuad createFormatted(std::string_view text, tr::TTFont& font, int fontSize,
												tr::RGBA8 textColor, TextOutline outline, const Textbox& textbox);

		/**************************************************************************************************************
		 * Adds formatted, multistyle text to the 2D renderer.
		 *
		 * @exception tr::BitmapBadAlloc If an internal allocation fails.
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] text
		 * @parblock
		 * The text string. See @ref renderformat for the specifics of the text format.
		 *
		 * @pre @em text cannot be an empty string.
		 * @endparblock
		 * @param[in] font The font to use for the text.
		 * @param[in] fontSize The font size to use for the text.
		 * @param[in] textColors
		 * @parblock
		 * The available text colors.
		 *
		 * @pre @em textColors cannot be empty.
		 * @endparblock
		 * @param[in] outline The outline parameters to use for the text, or NO_OUTLINE.
		 * @param[in] textbox The textbox to frame the text around.
		 **************************************************************************************************************/
		Renderer2D::TextureQuad createFormatted(std::string_view text, tr::TTFont& font, int fontSize,
												std::span<tr::RGBA8> textColors, TextOutline outline,
												const Textbox& textbox);

		/**************************************************************************************************************
		 * Prepares the manager for a new frame.
		 *
		 * @warning This function should only be called after all previously outputted text was rendered!
		 **************************************************************************************************************/
		void newFrame() noexcept;

	  private:
		DynAtlas2D _atlas;
		glm::uvec2 _dpi;

		Renderer2D::TextureQuad createMesh(const std::string& name, const Textbox& textbox);
	};

	/******************************************************************************************************************
	 * Gets whether the dynamic text manager was initialized.
	 *
	 * @return True if the dynamic text manager was initialized, and false otherwise.
	 ******************************************************************************************************************/
	bool dynamicTextActive() noexcept;

	/******************************************************************************************************************
	 * Gets a reference to the dynamic text manager.
	 *
	 * @pre The dynamic text manager must be instantiated.
	 *
	 * @return A reference to the dynamic text manager.
	 ******************************************************************************************************************/
	DynamicTextManager& dynamicText() noexcept;

	/// @}
} // namespace tre