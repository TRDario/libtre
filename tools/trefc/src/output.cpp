#include "../include/output.hpp"
#include "../../../include/tre/dependencies/qoi.h"
#include <fstream>

constexpr auto IMAGE_ENCODING_FAILURE_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"error:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" failed to encode image\n"};

constexpr auto WRITING_FAILURE_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"error:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" a writing operation failed on '{}'\n"};

using OutputImageDeleter = decltype([](void* ptr) { free(ptr); });

struct OutputImage {
	std::unique_ptr<void, OutputImageDeleter> data;
	int                                       size;
};

void writeBinary(std::ostream& os, const auto& value) noexcept
{
	os.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

ErrorCode writeToOutput(std::string_view path, const FontInfo& fontInfo, const InputImage& inputImage)
{
	std::ofstream file{path.data(), std::ios::binary};
	if (!file.is_open()) {
		print(std::cerr, FILE_OPENING_FAILURE_MESSAGE, path);
		return FILE_OPENING_FAILURE;
	}

	qoi_desc    desc{.width      = (unsigned int)(inputImage.width),
					 .height     = (unsigned int)(inputImage.height),
					 .channels   = (unsigned char)(inputImage.channels),
					 .colorspace = QOI_SRGB};
	OutputImage outputImage;
	outputImage.data.reset(qoi_encode(inputImage.data.get(), &desc, &outputImage.size));
	if (outputImage.data == nullptr) {
		print(std::cerr, IMAGE_ENCODING_FAILURE_MESSAGE);
		return IMAGE_FAILURE;
	}

	writeBinary(file, fontInfo.lineSkip);
	for (auto& [codepoint, glyph] : fontInfo.glyphs) {
		writeBinary(file, codepoint);
		writeBinary(file, glyph);
	}
	file.write(static_cast<const char*>(outputImage.data.get()), outputImage.size);

	if (!file.good()) {
		print(std::cerr, WRITING_FAILURE_MESSAGE, path);
		return WRITING_FAILURE;
	}

	return SUCCESS;
}