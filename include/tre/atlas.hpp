/**********************************************************************************************************************
 * @file atlas.hpp
 * @brief Provides texture atlases.
 **********************************************************************************************************************/

#pragma once
#include <forward_list>
#include <tr/tr.hpp>

namespace tre {
	/******************************************************************************************************************
	 * Atlas bitmap with named entries.
	 ******************************************************************************************************************/
	struct AtlasBitmap {
		/**************************************************************************************************************
		 * The atlas bitmap.
		 **************************************************************************************************************/
		tr::Bitmap bitmap;

		/**************************************************************************************************************
		 * The atlas entries.
		 **************************************************************************************************************/
		tr::StringHashMap<tr::RectI2> entries;
	};

	/******************************************************************************************************************
	 * Builds an atlas bitmap.
	 *
	 * @exception tr::BitmapBadAlloc If allocating the bitmap failed.
	 * @exception std::bad_alloc If allocating the entry map failed.
	 *
	 * @param[in] bitmap A list of named bitmaps.
	 * @param[in] format The format of the atlas bitmap.
	 *
	 * @return An atlas bitmap with entries named after the bitmaps that make up the atlas.
	 ******************************************************************************************************************/
	AtlasBitmap buildAtlasBitmap(const tr::StringHashMap<tr::Bitmap>& bitmaps,
								 tr::BitmapFormat format = tr::BitmapFormat::RGBA_8888);

	/******************************************************************************************************************
	 * Static 2D texture atlas.
	 ******************************************************************************************************************/
	class Atlas2D {
	  public:
		/**************************************************************************************************************
		 * Uploads a pre-made atlas bitmap.
		 *
		 * @exception tr::TextureBadAlloc If allocating the texture failed.
		 * @exception std::bad_alloc If allocating the texture entries failed.
		 *
		 * @param[in] atlasBitmap The atlas to upload to a texture.
		 **************************************************************************************************************/
		Atlas2D(AtlasBitmap atlasBitmap);

		/**************************************************************************************************************
		 * Creates an atlas from a list of named bitmaps.
		 *
		 * Equivalent to `Atlas2D(buildAtlasBitmap(bitmaps))`.
		 *
		 * @exception tr::BitmapBadAlloc If allocating the bitmap failed.
		 * @exception tr::TextureBadAlloc If allocating the texture failed.
		 * @exception std::bad_alloc If allocating a entry map failed.
		 *
		 * @param[in] bitmap A list of named bitmaps to upload.
		 **************************************************************************************************************/
		Atlas2D(const tr::StringHashMap<tr::Bitmap>& bitmaps);

		/**************************************************************************************************************
		 * Gets the atlas texture.
		 *
		 * @return An immutable reference to the atlas texture.
		 **************************************************************************************************************/
		const tr::Texture2D& texture() const noexcept;

		/**************************************************************************************************************
		 * Gets whether the atlas contains a texture.
		 *
		 * @param[in] name The name of the texture.
		 *
		 * @return True if an entry with that name exists, and false otherwise.
		 **************************************************************************************************************/
		bool contains(std::string_view name) const noexcept;

		/**************************************************************************************************************
		 * Returns the rect associated with an entry.
		 *
		 * @param[in] name The name of the entry. A failed assertion may be triggered if no entry with this name is
		 *                 contained in the atlas.
		 *
		 * @return The entry rect with normalized size and coordinates.
		 **************************************************************************************************************/
		const tr::RectF2& operator[](std::string_view name) const noexcept;

		/**************************************************************************************************************
		 * Sets the debug label of the atlas texture.
		 *
		 * @param[in] label The new label of the atlas texture.
		 **************************************************************************************************************/
		void setLabel(std::string_view label) noexcept;

	  private:
		tr::Texture2D _tex;
		tr::StringHashMap<tr::RectF2> _entries;
	};

	/******************************************************************************************************************
	 * Dynamically-allocated 2D texture atlas.
	 ******************************************************************************************************************/
	class DynAtlas2D {
	  public:
		/**************************************************************************************************************
		 * Creates an empty atlas.
		 **************************************************************************************************************/
		DynAtlas2D() noexcept;

		/**************************************************************************************************************
		 * Creates an empty atlas with an initial capacity.
		 *
		 * @exception tr::TextureBadAlloc If allocating the atlas texture failed.
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param capacity The initial capacity of the atlas.
		 **************************************************************************************************************/
		DynAtlas2D(glm::ivec2 capacity);

		/**************************************************************************************************************
		 * Gets the atlas texture.
		 *
		 * Calling this function for an empty atlas may trigger a failed assertion.
		 *
		 * @return An immutable reference to the atlas texture.
		 **************************************************************************************************************/
		const tr::Texture2D& texture() const noexcept;

		/**************************************************************************************************************
		 * Gets whether the atlas contains an entry.
		 *
		 * @param[in] name The name of the entry.
		 *
		 * @return True if an entry with that name exists, and false otherwise.
		 **************************************************************************************************************/
		bool contains(std::string_view name) const noexcept;

		/**************************************************************************************************************
		 * Returns the rect associated with an entry.
		 *
		 * @param[in] name The name of the entry. A failed assertion may be triggered if no entry with this name is
		 *                 contained in the atlas.
		 *
		 * @return The entry rect with normalized size and coordinates.
		 **************************************************************************************************************/
		tr::RectF2 operator[](std::string_view name) const noexcept;

		/**************************************************************************************************************
		 * Reserves a certain amount of space in the bitmap.
		 *
		 * If the requested capacity is larger than the current capacity, this function does nothing.
		 *
		 * @warning Calling this function invalidates any previous atlas texture bindings, the atlas texture must be
		 *          rebound to any texture units it was bound to.
		 *
		 * @exception tr::TextureBadAlloc If a texture reallocation happened and failed.
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param[in] capacity The new capacity of the atlas.
		 **************************************************************************************************************/
		void reserve(glm::ivec2 capacity);

		/**************************************************************************************************************
		 * Adds an entry to the atlas.
		 *
		 * @warning Calling this function invalidates any previous atlas texture bindings, the atlas texture must be
		 *          rebound to any texture units it was bound to.
		 *
		 * @exception tr::TextureBadAlloc If a texture reallocation happened and failed.
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param[in] name The name of the new entry.
		 * @param[in] bitmap The entry's bitmap data.
		 **************************************************************************************************************/
		void add(std::string name, tr::SubBitmap bitmap);

		/**************************************************************************************************************
		 * Removes an entry from the atlas.
		 *
		 * @exception std::bad_alloc If an internal allocation failed.
		 *
		 * @param[in] name The name of the entry to remove.
		 **************************************************************************************************************/
		void remove(std::string_view name);

		/**************************************************************************************************************
		 * Removes all entries from the atlas.
		 *
		 * @exception std::bad_alloc If an internal allocation failed.
		 **************************************************************************************************************/
		void clear();

		/**************************************************************************************************************
		 * Sets the debug label of the atlas texture.
		 *
		 * @param[in] label The new label of the atlas texture.
		 **************************************************************************************************************/
		void setLabel(std::string label) noexcept;

	  private:
		std::optional<tr::Texture2D> _tex;
		tr::StringHashMap<tr::RectI2> _entries;
		std::forward_list<tr::RectI2> _freeRects;
		std::string _label;

		// Does not append new free rects unlike the exposed function.
		void rawReserve(glm::ivec2 capacity);
		// Finds the predecessor to the first free rect, reallocating the atlas texture if needed until a suitable rect
		// is available.
		std::forward_list<tr::RectI2>::iterator findFreeRectPrev(glm::ivec2 size);
	};
} // namespace tre
