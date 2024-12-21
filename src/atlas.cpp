#include "../include/tre/atlas.hpp"

using NamedBitmaps  = tr::StringHashMap<tr::Bitmap>;
using NamedBitmapIt = NamedBitmaps::const_iterator;
using FreeRectList  = std::forward_list<tr::RectI2>;
using FreeRectIt    = FreeRectList::iterator;

namespace tre {
	glm::ivec2 doubleSmallerComponent(glm::ivec2 size) noexcept;
	// Gets the area of a 2D size.
	int area(glm::ivec2 size) noexcept;
	// Reasonable first guess at an atlas size.
	glm::ivec2 initialSize(const NamedBitmaps& bitmaps) noexcept;
	// Creates a vector of iterators to bitmaps sorted by area.
	std::vector<NamedBitmapIt> bitmapsByArea(const NamedBitmaps& bitmaps);
	// Finds the predecessor to the smallest suitable free rect or end() if none could be found.
	FreeRectIt findFreeRectPrev(FreeRectList& freeRects, glm::ivec2 size) noexcept;
	// Shrinks a free rect that has partially or fully been allocated to a bitmap.
	void shrinkFreeRect(FreeRectList& freeRects, FreeRectIt prev, glm::ivec2 size);
	// Tries to pack all of the bitmaps into a rectangle.
	std::optional<tr::StringHashMap<tr::RectI2>> tryPacking(glm::ivec2 size, const NamedBitmaps& bitmaps);
	// Attempts to pack until a non-nullopt result is reached.
	std::pair<glm::ivec2, tr::StringHashMap<tr::RectI2>> pack(const NamedBitmaps& bitmaps);
} // namespace tre

glm::ivec2 tre::doubleSmallerComponent(glm::ivec2 size) noexcept
{
	size.y < size.x ? size.y *= 2 : size.x *= 2;
	return size;
}

int tre::area(glm::ivec2 size) noexcept
{
	return size.x * size.y;
}

glm::ivec2 tre::initialSize(const NamedBitmaps& bitmaps) noexcept
{
	glm::ivec2 size{};
	int        bitmapArea{};
	for (auto& [name, bitmap] : bitmaps) {
		auto bitmapSize{bitmap.size()};
		if (bitmapSize.x > size.x) {
			size.x = std::bit_ceil((unsigned int)(bitmapSize.x));
		}
		if (bitmapSize.y > size.y) {
			size.y = std::bit_ceil((unsigned int)(bitmapSize.y));
		}
		bitmapArea += area(bitmap.size());
	}
	while (bitmapArea > area(size)) {
		size = doubleSmallerComponent(size);
	}
	return size;
}

std::vector<NamedBitmapIt> tre::bitmapsByArea(const NamedBitmaps& bitmaps)
{
	constexpr auto GREATER_AREA{[](auto& l, auto& r) { return area(l->second.size()) > area(r->second.size()); }};

	const auto                 iterators{std::views::iota(bitmaps.begin(), bitmaps.end())};
	std::vector<NamedBitmapIt> list{iterators.begin(), iterators.end()};
	std::ranges::sort(list, GREATER_AREA);
	return list;
}

FreeRectIt tre::findFreeRectPrev(FreeRectList& freeRects, glm::ivec2 size) noexcept
{
	FreeRectIt minPrev{freeRects.end()};
	int        minArea;
	for (auto [prev, it] = std::pair(freeRects.before_begin(), freeRects.begin()); it != freeRects.end(); prev = it++) {
		if (it->size.x < size.x || it->size.y < size.y) {
			continue;
		}
		int curArea{area(it->size)};
		if (minPrev == freeRects.end() || curArea < minArea) {
			minPrev = prev;
			minArea = curArea;
		}
	}
	return minPrev;
}

void tre::shrinkFreeRect(FreeRectList& freeRects, FreeRectIt prev, glm::ivec2 size)
{
	const FreeRectIt rect{std::next(prev)};

	if (rect->size == size) {
		freeRects.erase_after(prev);
		return;
	}
	if (rect->size.x > size.x) {
		if (rect->size.y > size.y) {
			const glm::vec2 newFreeRectTL{rect->tl.x, rect->tl.y + size.y};
			const glm::vec2 newFreeRectSize{size.x, rect->size.y - size.y};
			freeRects.emplace_front(newFreeRectTL, newFreeRectSize);
		}
		rect->tl.x += size.x;
		rect->size.x -= size.x;
		return;
	}
	if (rect->size.y > size.y) {
		if (rect->size.x > size.x) {
			const glm::vec2 newFreeRectSize{rect->size.x, size.y};
			freeRects.emplace_front(rect->tl, newFreeRectSize);
		}
		rect->tl.y += size.y;
		rect->size.y -= size.y;
	}
}

std::optional<tr::StringHashMap<tr::RectI2>> tre::tryPacking(glm::ivec2 size, const NamedBitmaps& bitmaps)
{
	constexpr auto deref{std::views::transform(&NamedBitmapIt::operator*)};

	tr::StringHashMap<tr::RectI2> rects;
	std::forward_list<tr::RectI2> freeRects{{{}, size}};
	for (auto& [name, bitmap] : bitmapsByArea(bitmaps) | deref) {
		auto prev{findFreeRectPrev(freeRects, bitmap.size())};
		if (prev == freeRects.end()) {
			return std::nullopt;
		}
		rects.emplace(std::move(name), tr::RectI2{std::next(prev)->tl, bitmap.size()});
		shrinkFreeRect(freeRects, prev, bitmap.size());
	}
	return rects;
}

std::pair<glm::ivec2, tr::StringHashMap<tr::RectI2>> tre::pack(const NamedBitmaps& bitmaps)
{
	glm::ivec2 size{initialSize(bitmaps)};
	auto       rects{tryPacking(size, bitmaps)};
	while (!rects.has_value()) {
		size  = doubleSmallerComponent(size);
		rects = tryPacking(size, bitmaps);
	}
	return {size, *std::move(rects)};
}

tre::AtlasBitmap tre::buildAtlasBitmap(const NamedBitmaps& bitmaps, tr::BitmapFormat format)
{
	auto [size, rects]{pack(bitmaps)};
	tr::Bitmap atlas{size, format};
	for (auto& [name, bitmap] : bitmaps) {
		atlas.blit(rects.at(name).tl, bitmap);
	}
	return {std::move(atlas), std::move(rects)};
}

tre::Atlas2D::Atlas2D(AtlasBitmap atlasBitmap)
	: _tex{atlasBitmap.bitmap, tr::NO_MIPMAPS, tr::TextureFormat::RGBA8}
{
	const auto size{glm::vec2(atlasBitmap.bitmap.size())};
	for (auto& [name, rect] : atlasBitmap.entries) {
		_entries.emplace(std::move(name), tr::RectF2{glm::vec2(rect.tl) / size, glm::vec2(rect.size) / size});
	}
}

tre::Atlas2D::Atlas2D(const tr::StringHashMap<tr::Bitmap>& bitmaps)
	: Atlas2D{buildAtlasBitmap(bitmaps)}
{
}

bool tre::Atlas2D::contains(std::string_view name) const noexcept
{
	return _entries.contains(name);
}

const tr::RectF2& tre::Atlas2D::operator[](std::string_view name) const noexcept
{
	assert(contains(name));
	return _entries.find(name)->second;
}

const tr::Texture2D& tre::Atlas2D::texture() const noexcept
{
	return _tex;
}

void tre::Atlas2D::setLabel(std::string_view label) noexcept
{
	_tex.setLabel(label);
}

tre::DynAtlas2D::DynAtlas2D() noexcept {}

tre::DynAtlas2D::DynAtlas2D(glm::ivec2 capacity)
	: _tex{{capacity, tr::NO_MIPMAPS, tr::TextureFormat::RGBA8}}, _freeRects{{{}, capacity}}
{
}

const tr::Texture2D& tre::DynAtlas2D::texture() const noexcept
{
	assert(_tex.has_value());
	return *_tex;
}

bool tre::DynAtlas2D::contains(std::string_view name) const noexcept
{
	return _entries.contains(name);
}

std::size_t tre::DynAtlas2D::size() const noexcept
{
	return _entries.size();
}

tr::RectF2 tre::DynAtlas2D::operator[](std::string_view name) const noexcept
{
	assert(contains(name));
	tr::RectF2 rect{_entries.find(name)->second};
	rect.tl /= _tex->size();
	rect.size /= _tex->size();
	return rect;
}

void tre::DynAtlas2D::rawReserve(glm::ivec2 capacity)
{
	if (!_tex.has_value()) {
		_tex.emplace(capacity, tr::NO_MIPMAPS, tr::TextureFormat::RGBA8);
	}
	else {
		glm::ivec2 oldCapacity{_tex->size()};
		if (capacity.x <= oldCapacity.x && capacity.y <= oldCapacity.y) {
			return;
		}
		tr::Texture2D          newTex{capacity, tr::NO_MIPMAPS, tr::TextureFormat::RGBA8};
		static tr::Framebuffer dynArrayCopyFBO;
#ifndef NDEBUG
		static bool addedLabel{false};
		if (!addedLabel) {
			dynArrayCopyFBO.setLabel("(tr) Dynamic Atlas Copy Framebuffer");
			addedLabel = true;
		}
#endif
		dynArrayCopyFBO.attach(*_tex, tr::Framebuffer::Slot::COLOR0);
		dynArrayCopyFBO.copyRegion({{}, oldCapacity}, newTex, {});
		_tex = std::move(newTex);
	}
	if (!_label.empty()) {
		_tex->setLabel(_label);
	}
}

void tre::DynAtlas2D::reserve(glm::ivec2 capacity)
{
	if (!_tex.has_value()) {
		rawReserve(capacity);
		_freeRects.emplace_front(glm::ivec2{0, 0}, capacity);
	}
	else {
		glm::ivec2 oldCapacity{_tex->size()};
		rawReserve(capacity);
		_freeRects.emplace_front(glm::ivec2{oldCapacity.x, 0}, glm::ivec2{capacity.x - oldCapacity.x, oldCapacity.y});
		_freeRects.emplace_front(glm::ivec2{0, oldCapacity.y}, glm::ivec2{capacity.x, capacity.y - oldCapacity.y});
	}
}

std::forward_list<tr::RectI2>::iterator tre::DynAtlas2D::findFreeRectPrev(glm::ivec2 size)
{
	if (!_tex.has_value()) {
		glm::uvec2 capacity{std::bit_ceil((unsigned int)(size.x)), std::bit_ceil((unsigned int)(size.y))};
		rawReserve(capacity);
		_freeRects.emplace_front(glm::ivec2{0, 0}, capacity);
		return _freeRects.before_begin();
	}
	else {
		auto it{tre::findFreeRectPrev(_freeRects, size)};
		if (it == _freeRects.end()) {
			glm::ivec2 oldCapacity{_tex->size()};
			glm::ivec2 newCapacity{doubleSmallerComponent(_tex->size())};
			auto&      newRect1{_freeRects.emplace_front(glm::ivec2{oldCapacity.x, 0},
														 glm::ivec2{newCapacity.x - oldCapacity.x, oldCapacity.y})};
			auto&      newRect2{_freeRects.emplace_front(glm::ivec2{0, oldCapacity.y},
														 glm::ivec2{newCapacity.x, newCapacity.y - oldCapacity.y})};
			it = tre::findFreeRectPrev(_freeRects, size);
			while (it == _freeRects.end()) {
				newCapacity = doubleSmallerComponent(newCapacity);
				newRect1    = {_freeRects.emplace_front(glm::ivec2{oldCapacity.x, 0},
														glm::ivec2{newCapacity.x - oldCapacity.x, oldCapacity.y})};
				newRect2    = {_freeRects.emplace_front(glm::ivec2{0, oldCapacity.y},
														glm::ivec2{newCapacity.x, newCapacity.y - oldCapacity.y})};
				it          = tre::findFreeRectPrev(_freeRects, size);
			}
			rawReserve(newCapacity);
		}
		return it;
	}
}

void tre::DynAtlas2D::add(const std::string& name, const tr::SubBitmap& bitmap)
{
	const auto prev{findFreeRectPrev(bitmap.size())};
	const auto it{std::next(prev)};

	_entries.emplace(std::piecewise_construct, std::forward_as_tuple(name),
					 std::forward_as_tuple(it->tl, bitmap.size()));
	_tex->setRegion(it->tl, bitmap);
	tre::shrinkFreeRect(_freeRects, prev, bitmap.size());
}

void tre::DynAtlas2D::add(std::string&& name, const tr::SubBitmap& bitmap)
{
	const auto prev{findFreeRectPrev(bitmap.size())};
	const auto it{std::next(prev)};

	_entries.emplace(std::piecewise_construct, std::forward_as_tuple(std::move(name)),
					 std::forward_as_tuple(it->tl, bitmap.size()));
	_tex->setRegion(it->tl, bitmap);
	tre::shrinkFreeRect(_freeRects, prev, bitmap.size());
}

void tre::DynAtlas2D::remove(std::string_view name) noexcept
{
	auto it{_entries.find(name)};
	if (it != _entries.end()) {
		_entries.erase(it);
		_freeRects.emplace_front(it->second.tl, it->second.size);
	}
}

void tre::DynAtlas2D::clear() noexcept
{
	_entries.clear();
	_freeRects.clear();
	if (_tex.has_value()) {
		_freeRects.emplace_front(glm::ivec2{}, _tex->size());
	}
}

void tre::DynAtlas2D::setLabel(const std::string& label)
{
	_label = label;
	if (_tex.has_value()) {
		_tex->setLabel(_label);
	}
}

void tre::DynAtlas2D::setLabel(std::string&& label) noexcept
{
	_label = std::move(label);
	if (_tex.has_value()) {
		_tex->setLabel(_label);
	}
}
