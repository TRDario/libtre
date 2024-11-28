/**********************************************************************************************************************
 * @file sampler.hpp
 * @brief Provides common samplers.
 **********************************************************************************************************************/

#pragma once
#include <tr/sampler.hpp>

namespace tre {
	/******************************************************************************************************************
	 * Gives access to a nearest neighbor sampler.
	 *
	 * This function cannot be called before an OpenGL context is created.
	 *
	 * @return A constant reference to the sampler.
	 ******************************************************************************************************************/
	const tr::Sampler& nearestNeighborSampler() noexcept;

	/******************************************************************************************************************
	 * Gives access to a bilinear sampler.
	 *
	 * This function cannot be called before an OpenGL context is created.
	 *
	 * @return A constant reference to the sampler.
	 ******************************************************************************************************************/
	const tr::Sampler& bilinearSampler() noexcept;

	/******************************************************************************************************************
	 * Gives access to a trilinear sampler.
	 *
	 * This function cannot be called before an OpenGL context is created.
	 *
	 * @return A constant reference to the sampler.
	 ******************************************************************************************************************/
	const tr::Sampler& trilinearSampler() noexcept;
} // namespace tre