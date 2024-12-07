#pragma once
#include <cstdint>

namespace tre {
	/** @defgroup renderer Renderers
	 * Renderers and related functionality.
	 *
	 * libtre prefers to work with self-contained renderers, as opposed to directly manipulating OpenGL state, which can
	 * cause inefficiencies if repeated, redundant state changes are made.
	 *
	 * When making a tre-compatible renderer, setLastRendererID and lastRendererID should be used to facilitate more
	 * efficient state changes.
	 *
	 *  An instance of tr::Window must be created before any functionality from this section can be used.
	 *  @{
	 */

	/******************************************************************************************************************
	 * Special ID indicating that rendering without a specific renderer was being done.
	 ******************************************************************************************************************/
	inline constexpr std::uint32_t NO_RENDERER{-1U};

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
	 * @param[in] id The ID of the renderer to be set to last used.
	 ******************************************************************************************************************/
	void setLastRendererID(std::uint32_t id) noexcept;

	/// @}
} // namespace tre
