#pragma once
#include <tr/tr.hpp>

namespace tre {
	/******************************************************************************************************************
	 * A view that can be rendered to.
	 *
	 * @warning Manually setting the tr graphics context framebuffer state should not be mixed with render views due to
	 *          internal optimizations for preventing unnecessary state changes only seeing changes made by other
	 *          render view objects.
	 ******************************************************************************************************************/
	class RenderView {
	  public:
		/**************************************************************************************************************
		 * Creates a render view over an entire framebuffer.
		 *
		 * @param framebuffer The framebuffer to create the view over.
		 **************************************************************************************************************/
		RenderView(tr::BasicFramebuffer& framebuffer) noexcept;

		/**************************************************************************************************************
		 * Creates a render view over a region of a framebuffer.
		 *
		 * @param framebuffer The framebuffer to create the view over.
		 * @param viewPort The viewport of the view inside the framebuffer.
		 **************************************************************************************************************/
		RenderView(tr::BasicFramebuffer& framebuffer, const tr::RectI2& viewport) noexcept;

		/**************************************************************************************************************
		 * Creates a render view over a region of a framebuffer.
		 *
		 * @param framebuffer The framebuffer to create the view over.
		 * @param viewPort The viewport of the view inside the framebuffer.
		 * @param doubleMin, doubleMax The minimum and maximum Z buffer depth values, respectively.
		 **************************************************************************************************************/
		RenderView(tr::BasicFramebuffer& framebuffer, const tr::RectI2& viewport, double depthMin,
				   double depthMax) noexcept;

		/**************************************************************************************************************
		 * Equality comparison operator.
		 *
		 * @param l, r The render views to compare.
		 *
		 * @return true if the views are identical, and false otherwise.
		 *************************************************************************************************************/
		friend bool operator==(const RenderView& l, const RenderView& r) noexcept;

		/*************************************************************************************************************
		 * Sets up the graphics context to use the render view.
		 *
		 * This method is primarily intended for use in custom renderers.
		 *
		 * @note Consecutive calls to use() by the same render view are optimized to minimise internal state switching.
		 **************************************************************************************************************/
		void use() const noexcept;

	  private:
		std::reference_wrapper<tr::BasicFramebuffer> _framebuffer;
		tr::RectI2                                   _viewport;
		double                                       _depthMin;
		double                                       _depthMax;
	};
} // namespace tre