/**
 * @file atlas.hpp
 * @brief Provides texture atlases.
 * @details libtre prefers to work with self-contained renderers, as opposed to directly manipulating OpenGL state.
 *          This can cause inefficiencies if repeated, redundant state changes are made. This file thus provides
 *          helper functions to minimize switching state when not needed.
 */

#pragma once
#include <tr/tr.hpp>

namespace tre {
	/******************************************************************************************************************
     * Special ID indicating that rendering without a specific renderer was being done.
     ******************************************************************************************************************/
    inline constexpr std::uint32_t NO_RENDERER { -1U };

	/******************************************************************************************************************
     * Generates a new valid renderer ID.
	 *
	 * @return A unique, unused renderer ID.
     ******************************************************************************************************************/
    std::uint32_t generateRendererID() noexcept;


	/******************************************************************************************************************
     * Gets the ID of the last used renderer.
	 *
	 * @return The ID of the last used renderer.
     ******************************************************************************************************************/
    std::uint32_t lastRendererID() noexcept;

	/******************************************************************************************************************
     * Sets the ID of the last used renderer.
	 *
	 * @param id The ID of the renderer to be set to last used.
     ******************************************************************************************************************/
    void setLastRendererID(std::uint32_t id) noexcept;
}