#include "../include/tre/localization_manager.hpp"


tre::LocFileParseWithErrors::LocFileParseWithErrors(std::string path, std::vector<std::string> errors, LocalizationManager manager) noexcept
	: FileError { path }
	, _errors { std::move(errors) }
	, _manager { std::move(manager) }
{}

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

tre::LocalizationManager::LocalizationManager() noexcept
{}

tre::LocalizationManager::LocalizationManager(Map map) noexcept
	: _map { std::move(map) }
{}

tre::LocalizationManager::LocalizationManager(const std::filesystem::path& file)
{
	auto is { tr::openFileR(file) };

	std::vector<std::string> errors;
    std::string line;
	for (int lineNumber = 1; !is.eof(); ++lineNumber) {
		std::getline(is, line);
		if (line.empty() || line.starts_with("//") || std::ranges::all_of(line, [] (auto chr) { return chr == ' ' || chr == '\t'; })) {
			continue;
		}

		auto delimColon { line.find(':') };
		if (delimColon == line.npos) {
			errors.emplace_back(std::format("line {}: Expected a delimiting colon.", lineNumber));
			continue;
		}
		else if (delimColon == 0) {
			errors.emplace_back(std::format("line {}: Expected a key string before the delimiting colon.", lineNumber));
			continue;
		}
		else if (delimColon == line.size() - 1) {
			errors.emplace_back(std::format("line {}: Expected a value string after the delimiting colon.", lineNumber));
			continue;
		}

		std::string_view rawKeyView { line.begin(), line.begin() + delimColon };
		if (rawKeyView.size() > 30) {
			errors.emplace_back(std::format("line {}: Key string '{}' is too long.", lineNumber, rawKeyView));
			continue;
		}
		else if (_map.contains(rawKeyView)) {
			errors.emplace_back(std::format("line {}: Duplicate key '{}'.", lineNumber, rawKeyView));
			continue;
		}

		std::string_view rawValueView { line.begin() + delimColon + 1, line.end() };
		std::string value;
		value.reserve(rawValueView.size());
		for (auto it = rawValueView.begin(); it != rawValueView.end(); ++it) {
			if (*it == '\\') {
				++it;
				if (it == rawValueView.end()) {
					errors.emplace_back(std::format("line {}: Unterminated escape sequence in value string.", lineNumber, rawKeyView));
					break;
				}

				if (*it == 'n') {
					value.push_back('\n');
				}
				else if (*it == '\\') {
					value.push_back(*it);
				}
				else {
					errors.emplace_back(std::format("line {}: Unknown escape sequence \\{} in value string.", lineNumber, *it));
				}
			}
			else {
				value.push_back(*it);
			}
		}
		value.shrink_to_fit();

		_map.emplace(rawKeyView, std::move(value));
	}

	if (!errors.empty()) {
		throw LocFileParseWithErrors { file.string(), std::move(errors), LocalizationManager { std::move(_map) } };
	}
}

std::string_view tre::LocalizationManager::operator[](std::string_view key) const noexcept
{
	auto it { _map.find(key) };
	if (it == _map.end()) {
		return key;
	}
	return it->second;
}

const tre::LocalizationManager::Map& tre::LocalizationManager::map() const noexcept
{
	return _map;
}