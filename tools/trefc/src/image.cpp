#include "../include/image.hpp"
#include "../include/dependencies/stb_image.h"
#include <filesystem>

constexpr auto IMAGE_LOADING_FAILURE_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"error:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" failed to load image from '{}' ({})\n"};

constexpr auto INVALID_IMAGE_FORMAT_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"error:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" image at '{}' could not be converted to a supported format (3 or 4 RGB(A) channels)\n"};

void InputImageDeleter::operator()(unsigned char* ptr) const noexcept
{
	stbi_image_free(ptr);
}

Expected<InputImage, ErrorCode> loadImage(std::string_view path)
{
	if (!std::filesystem::exists(path)) {
		print(std::cerr, FILE_NOT_FOUND_MESSAGE, path);
		return FILE_NOT_FOUND;
	}

	InputImage inputImage;
	inputImage.data.reset(stbi_load(path.data(), &inputImage.width, &inputImage.height, &inputImage.channels, 0));
	if (inputImage.data == nullptr) {
		print(std::cerr, IMAGE_LOADING_FAILURE_MESSAGE, path, stbi_failure_reason());
		return IMAGE_FAILURE;
	}
	else if (inputImage.channels < 3) {
		print(std::cerr, INVALID_IMAGE_FORMAT_MESSAGE, path);
		return IMAGE_FAILURE;
	}
	else {
		return std::move(inputImage);
	}
}