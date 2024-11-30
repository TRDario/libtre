#pragma once
#include "common.hpp"
#include <memory>

struct InputImageDeleter {
	void operator()(unsigned char* ptr) const noexcept;
};

struct InputImage {
	std::unique_ptr<unsigned char, InputImageDeleter> data;
	int                                               width;
	int                                               height;
	int                                               channels;
};

Expected<InputImage, ErrorCode> loadImage(std::string_view path);