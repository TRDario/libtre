#include "../include/tre/sampler.hpp"

const tr::Sampler& tre::nearestNeighborSampler() noexcept
{
	static bool inited{false};
	static tr::Sampler sampler;
	if (!inited) {
		sampler.setMinFilter(tr::MinFilter::NEAREST);
		sampler.setMagFilter(tr::MagFilter::NEAREST);
#ifndef NDEBUG
		sampler.setLabel("tre Nearest Neighbor Sampler");
#endif
		inited = true;
	}
	return sampler;
}

const tr::Sampler& tre::bilinearSampler() noexcept
{
	static bool inited{false};
	static tr::Sampler sampler;
	if (!inited) {
		sampler.setMinFilter(tr::MinFilter::LINEAR);
		sampler.setMagFilter(tr::MagFilter::LINEAR);
#ifndef NDEBUG
		sampler.setLabel("tre Bilinear Sampler");
#endif
		inited = true;
	}
	return sampler;
}

const tr::Sampler& tre::trilinearSampler() noexcept
{
	static bool inited{false};
	static tr::Sampler sampler;
	if (!inited) {
		sampler.setMinFilter(tr::MinFilter::LMIPS_LINEAR);
		sampler.setMagFilter(tr::MagFilter::LINEAR);
#ifndef NDEBUG
		sampler.setLabel("tre Trilinear Sampler");
#endif
		inited = true;
	}
	return sampler;
}