#pragma once
#include <format>
#include <iostream>

constexpr auto FILE_NOT_FOUND_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"error:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" file '{}' not found\n"};

constexpr auto FILE_OPENING_FAILURE_MESSAGE{
#ifdef TREFC_ANSI_COLORS
	"\x1b[1;91m"
#endif
	"error:"
#ifdef TREFC_ANSI_COLORS
	"\x1b[0m"
#endif
	" failed to open file '{}'\n"};

enum ErrorCode : int {
	UNHANDLED_EXCEPTION = -1,
	SUCCESS,
	PRINTED_HELP = 0,
	INVALID_ARGUMENT_COUNT,
	FILE_NOT_FOUND,
	FILE_OPENING_FAILURE,
	PARSING_FAILURE,
	IMAGE_FAILURE,
	WRITING_FAILURE
};

template <class T, class Error> using Expected = std::variant<T, Error>;

template <class T, class Error, class U> const U& get(const Expected<T, Error>& value) noexcept
{
	return *get_if(&value);
}

template <class... Ts> void print(std::ostream& os, std::format_string<Ts...> fmt, Ts&&... args)
{
	std::format_to(std::ostreambuf_iterator<char>{os}, fmt, std::forward<Ts&&>(args)...);
}