#pragma once
#include "image.hpp"
#include "input.hpp"

ErrorCode writeToOutput(std::string_view path, const FontInfo& fontInfo, const InputImage& inputImage);