#include "../include/tre/renderer_base.hpp"

namespace tre {
	std::uint32_t _lastRenderer {NO_RENDERER};
}

std::uint32_t tre::generateRendererID() noexcept
{
	static std::uint32_t newID {0};
	return newID++;
}

std::uint32_t tre::lastRendererID() noexcept
{
	return _lastRenderer;
}

void tre::setLastRendererID(std::uint32_t id) noexcept
{
	_lastRenderer = id;
}
