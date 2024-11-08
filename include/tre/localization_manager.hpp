/**********************************************************************************************************************
 * @file localization_manager.hpp
 * @brief Provides a localization manager class.
 **********************************************************************************************************************/

#pragma once
#include <tr/tr.hpp>

namespace tre {
	/******************************************************************************************************************
	 * Localization string manager.
	 ******************************************************************************************************************/
	class LocalizationManager {
	public:
		/**************************************************************************************************************
		 * Shorthand for the base map type used by the manager.
		 **************************************************************************************************************/
		using Map = tr::StaticStringHashMap<30, std::string>;

		/**************************************************************************************************************
		 * Constructs an empty localization manager.
		 **************************************************************************************************************/
		LocalizationManager() noexcept;

		/**************************************************************************************************************
		 * Constructs an localization manager.
		 *
		 * @param[in] map A localization map.
		 **************************************************************************************************************/
		LocalizationManager(Map map) noexcept;

		/**************************************************************************************************************
		 * Loads a localization manager.
		 *
		 * For specifics of the localization file format see @ref locformat.
		 *
		 * @exception tr::FileNotFound If the file was not found.
		 * @exception tr::FileOpenError If opening the file failed.
		 * @exception std::bad_alloc If any internal allocations fail.
		 * @exception LocFileParseWithErrors
		 * @parblock
		 * If any parsing errors occur during the parsing.
		 *
		 * Parsing errors are collected and recovered from during the loading process, and a localization manager in a
		 * valid state is produced even if errors are thrown.
		 *
		 * You may extract the manager and the error information from the thrown exception.
		 * @endparblock
		 *
		 * @param[in] file The path to the localization file.
		 **************************************************************************************************************/
		LocalizationManager(const std::filesystem::path& file);

		/**************************************************************************************************************
		 * Gets a localization string associated with a key.
		 *
		 * @warning Passing string temporaries to this function could result in a dangling string view being returned!
		 *
		 * @param[in] key A localization key.
		 *
		 * @return The value associated with the passed key, or the key itself if no associated value exists.
		 **************************************************************************************************************/
		std::string_view operator[](std::string_view key) const noexcept;

		/**************************************************************************************************************
		 * Gets access to the base map of the manager.
		 *
		 * @return An immutable reference to the base map.
		 **************************************************************************************************************/
		const Map&       map() const noexcept;

	private:
		tr::StaticStringHashMap<30, std::string> _map;
	};

	/******************************************************************************************************************
	 * Error thrown by a localization file parse with errors.
	 ******************************************************************************************************************/
	class LocFileParseWithErrors : public tr::FileError {
	public:
		/**************************************************************************************************************
		 * Constructs an error.
		 *
		 * @param[in] path The localization file path string.
		 * @param[in] errors A list of error strings.
		 * @param[in] manager The recovered localization manager.
		 **************************************************************************************************************/
		LocFileParseWithErrors(std::string path, std::vector<std::string> errors, LocalizationManager manager) noexcept;

		/**************************************************************************************************************
		 * Gets the list of error messages.
		 *
		 * A reference to a vector containing error messages.
		 **************************************************************************************************************/
		const std::vector<std::string>& errors() const noexcept;

		/**************************************************************************************************************
		 * Moves the recovered localization manager out of the error.
		 *
		 * @return The localization manager as an rvalue.
		 **************************************************************************************************************/
		LocalizationManager&&           manager() && noexcept;

		/**************************************************************************************************************
		 * Gets an error message.
		 *
		 * @return An explanatory error message.
		 **************************************************************************************************************/
		virtual const char*             what() const noexcept;

	private:
		std::vector<std::string> _errors;
		LocalizationManager      _manager;
	};
} // namespace tre
