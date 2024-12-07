#pragma once
#include <tr/sampler.hpp>

namespace tre {
	/** @defgroup sampler_presets Sampler Presets
	 *  Common sampler presets.
	 *
	 *  An instance of tr::Window must be created before any functionality from this section can be used.
	 *  @{
	 */

	/******************************************************************************************************************
	 * Gives access to a nearest neighbor sampler.
	 *
	 * @return A constant reference to the sampler.
	 ******************************************************************************************************************/
	const tr::Sampler& nearestNeighborSampler() noexcept;

	/******************************************************************************************************************
	 * Gives access to a bilinear sampler.
	 *
	 * @return A constant reference to the sampler.
	 ******************************************************************************************************************/
	const tr::Sampler& bilinearSampler() noexcept;

	/******************************************************************************************************************
	 * Gives access to a trilinear sampler.
	 *
	 * @return A constant reference to the sampler.
	 ******************************************************************************************************************/
	const tr::Sampler& trilinearSampler() noexcept;

	/// @}
} // namespace tre