#include "trefc.hpp"
#include <filesystem>
#include <stb_image.h>

constexpr auto IMAGE_LOADING_FAILURE_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"error:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" failed to load image from '{}' ({})\n"};

Bitmap::~Bitmap() noexcept
{
	stbi_image_free((void*)(data));
}

Expected<Bitmap, ErrorCode> loadBitmap(std::string_view path)
{
	if (!std::filesystem::exists(path)) {
		print(std::cerr, FILE_NOT_FOUND_MESSAGE, path);
		return FILE_NOT_FOUND;
	}

	int  width, height, channels;
	auto bitmapData{stbi_load(path.data(), &width, &height, &channels, 4)};
	if (bitmapData == nullptr) {
		print(std::cerr, IMAGE_LOADING_FAILURE_MESSAGE, path, stbi_failure_reason());
		return IMAGE_FAILURE;
	}
	else {
		return Bitmap{bitmapData, (unsigned int)(width), (unsigned int)(height)};
	}
}