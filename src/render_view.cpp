#include "../include/tre/render_view.hpp"

namespace tre {
	// The last set render view is kept track of to prevent unnecessary state changes.
	std::optional<tre::RenderView> _lastView;
} // namespace tre

tre::RenderView::RenderView(tr::BasicFramebuffer& framebuffer) noexcept
	: RenderView{framebuffer, {{}, framebuffer.size()}, 0, 1}
{
}

tre::RenderView::RenderView(tr::BasicFramebuffer& framebuffer, const tr::RectI2& viewport) noexcept
	: RenderView{framebuffer, viewport, 0, 1}
{
}

tre::RenderView::RenderView(tr::BasicFramebuffer& framebuffer, const tr::RectI2& viewport, double depthMin,
							double depthMax) noexcept
	: _framebuffer{framebuffer}, _viewport{viewport}, _depthMin{depthMin}, _depthMax{depthMax}
{
}

bool tre::operator==(const RenderView& l, const RenderView& r) noexcept
{
	return &l._framebuffer.get() == &r._framebuffer.get() && l._viewport == r._viewport && l._depthMin == r._depthMin &&
		   l._depthMax == r._depthMax;
}

void tre::RenderView::use() const noexcept
{
	if (_lastView != *this) {
		tr::window().graphics().setFramebuffer(_framebuffer.get());
		tr::window().graphics().setViewport(_viewport);
		tr::window().graphics().setDepthRange(_depthMin, _depthMax);
		tr::window().graphics().useScissorTest(true);
		tr::window().graphics().setScissorBox(_viewport);
		_lastView = *this;
	}
}