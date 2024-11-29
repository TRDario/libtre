#include "../include/tre/localization_manager.hpp"

namespace tre {
	bool skipLine(const std::string& line) noexcept;

	bool validateDelimiter(std::size_t delimiter, std::size_t lineSize, std::vector<std::string>& errors,
						   int lineNumber);

	bool validateKey(std::string_view key, const LocalizationManager::Map& map, std::vector<std::string>& errors,
					 int lineNumber);

	std::string processValue(std::string_view rawValue, std::vector<std::string>& errors, int lineNumber);
} // namespace tre

bool tre::skipLine(const std::string& line) noexcept
{
	constexpr auto WHITESPACE{[](auto chr) { return chr == ' ' || chr == '\t'; }};

	return line.empty() || line.starts_with("//") || std::ranges::all_of(line, WHITESPACE);
}

bool tre::validateDelimiter(std::size_t delimiter, std::size_t lineSize, std::vector<std::string>& errors,
							int lineNumber)
{
	if (delimiter == std::string::npos) {
		errors.emplace_back(std::format("line {}: Expected a delimiting colon.", lineNumber));
		return false;
	}
	else if (delimiter == 0) {
		errors.emplace_back(std::format("line {}: Expected a key string before the delimiting colon.", lineNumber));
		return false;
	}
	else if (delimiter == lineSize - 1) {
		errors.emplace_back(std::format("line {}: Expected a value string after the delimiting colon.", lineNumber));
		return false;
	}
	else {
		return true;
	}
}

bool tre::validateKey(std::string_view key, const LocalizationManager::Map& map, std::vector<std::string>& errors,
					  int lineNumber)
{
	if (key.size() > 30) {
		errors.emplace_back(std::format("line {}: Key string '{}' is too long.", lineNumber, key));
		return false;
	}
	else if (map.contains(key)) {
		errors.emplace_back(std::format("line {}: Duplicate key '{}'.", lineNumber, key));
		return false;
	}
	else {
		return true;
	}
}

std::string tre::processValue(std::string_view rawValue, std::vector<std::string>& errors, int lineNumber)
{
	std::string value;
	value.reserve(rawValue.size());
	for (auto it = rawValue.begin(); it != rawValue.end(); ++it) {
		if (*it == '\\') {
			if (++it == rawValue.end()) {
				errors.emplace_back(std::format("line {}: Unterminated escape sequence in value string.", lineNumber));
				break;
			}

			if (*it == 'n') {
				value.push_back('\n');
			}
			else if (*it == '\\') {
				value.push_back('\\');
			}
			else {
				errors.emplace_back(
					std::format("line {}: Unknown escape sequence \\{} in value string.", lineNumber, *it));
			}
		}
		else {
			value.push_back(*it);
		}
	}
	value.shrink_to_fit();
	return value;
}

tre::LocFileParseWithErrors::LocFileParseWithErrors(std::string path, std::vector<std::string> errors,
													LocalizationManager manager) noexcept
	: FileError{path}, _errors{std::move(errors)}, _manager{std::move(manager)}
{
}

const std::vector<std::string>& tre::LocFileParseWithErrors::errors() const noexcept
{
	return _errors;
}

tre::LocalizationManager&& tre::LocFileParseWithErrors::manager() && noexcept
{
	return std::move(_manager);
}

const char* tre::LocFileParseWithErrors::what() const noexcept
{
	static std::string str;
	str = std::format("Loaded localization file with {} parsing errors: '{}'", errors().size(), path());
	return str.c_str();
}

tre::LocalizationManager::LocalizationManager() noexcept {}

tre::LocalizationManager::LocalizationManager(Map map) noexcept
	: _map{std::move(map)}
{
}

tre::LocalizationManager::LocalizationManager(const std::filesystem::path& file)
{
	auto                     is{tr::openFileR(file)};
	std::vector<std::string> errors;

	std::string line;
	for (int lineNumber = 1; !is.eof(); ++lineNumber) {
		std::getline(is, line);
		if (skipLine(line)) {
			continue;
		}
		std::size_t delimiter{line.find(':')};
		if (!validateDelimiter(delimiter, line.size(), errors, lineNumber)) {
			continue;
		}
		std::string_view key{line.begin(), line.begin() + delimiter};
		if (!validateKey(key, _map, errors, lineNumber)) {
			continue;
		}
		std::string_view value{line.begin() + delimiter + 1, line.end()};
		_map.emplace(key, processValue(value, errors, lineNumber));
	}

	if (!errors.empty()) {
		throw LocFileParseWithErrors{file.string(), std::move(errors), LocalizationManager{std::move(_map)}};
	}
}

std::string_view tre::LocalizationManager::operator[](std::string_view key) const noexcept
{
	auto it{_map.find(key)};
	return it != _map.end() ? it->second : key;
}

const tre::LocalizationManager::Map& tre::LocalizationManager::map() const noexcept
{
	return _map;
}
