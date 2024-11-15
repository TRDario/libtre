/**********************************************************************************************************************
 * @file debug_text.hpp
 * @brief Provides a debug text renderer.
 **********************************************************************************************************************/

#pragma once
#include "renderer_2d.hpp"

namespace tre {
	/******************************************************************************************************************
	 * Debug text renderer.
	 ******************************************************************************************************************/
	class DebugTextRenderer {
	  public:
		void outputTo(Renderer2D& renderer, int priority);

	  private:
		tr::Texture2D _texture;
		tr::Sampler _sampler;
		glm::ivec2 _lPen;
		glm::ivec2 _rPen;
	};
} // namespace tre