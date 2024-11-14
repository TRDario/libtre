/**********************************************************************************************************************
 * @file state_manager.hpp
 * @brief Provides a game state management framework.
 **********************************************************************************************************************/

#pragma once
#include <tr/tr.hpp>

namespace tre {
	/******************************************************************************************************************
	 * Abstract game state interface.
	 ******************************************************************************************************************/
	struct State {
		/**************************************************************************************************************
		 * Virtual destructor.
		 **************************************************************************************************************/
		virtual ~State() = default;

		/**************************************************************************************************************
		 * Gets the type of the state.
		 *
		 * @return An ID uniquely associated with a state type.
		 **************************************************************************************************************/
		virtual std::uint32_t type() const noexcept = 0;

		/**************************************************************************************************************
		 * Handles an event.
		 *
		 * @param[in] event The event to handle.
		 *
		 * @return
		 * @parblock
		 * When used inside a state manager, the return value of this function can dictate the progression to a new
		 *state.
		 *
		 * Returning a non-nullptr state pointer will cause the manager to switch from the current state to the state
		 * that was returned from this function.
		 *
		 * Returning nullptr will maintain the current state.
		 * @endparblock
		 **************************************************************************************************************/
		virtual std::unique_ptr<State> handleEvent(const tr::Event& event) = 0;

		/**************************************************************************************************************
		 * Updates the state.
		 *
		 * @param[in] delta The time step since the last update.
		 *
		 * @return
		 * @parblock
		 * When used inside a state manager, the return value of this function can dictate the progression to a new
		 *state.
		 *
		 * Returning a non-nullptr state pointer will cause the manager to switch from the current state to the state
		 * that was returned from this function.
		 *
		 * Returning nullptr will maintain the current state.
		 * @endparblock
		 **************************************************************************************************************/
		virtual std::unique_ptr<State> update(tr::Duration delta) = 0;

		/**************************************************************************************************************
		 * Draws the state to the screen.
		 **************************************************************************************************************/
		virtual void draw() = 0;
	};

	/******************************************************************************************************************
	 * Game state manager.
	 ******************************************************************************************************************/
	class StateManager {
	  public:
		/**************************************************************************************************************
		 * Constructs a manager with no held state.
		 **************************************************************************************************************/
		StateManager() noexcept = default;

		/**************************************************************************************************************
		 * Constructs a manager with initial held state.
		 *
		 * @param[in] state An owning polymorphic pointer to a state.
		 **************************************************************************************************************/
		StateManager(std::unique_ptr<State> state) noexcept;

		/**************************************************************************************************************
		 * Gets whether the manager is holding a state.
		 *
		 * @return True if the manager is holding a state, and false otherwise.
		 **************************************************************************************************************/
		bool has_state() const noexcept;

		/**************************************************************************************************************
		 * Gets the held state.
		 *
		 * If the manager is not holding a state, a failed assertion may be triggered.
		 *
		 * @return A mutable reference to the held state.
		 **************************************************************************************************************/
		State& state() noexcept;

		/**************************************************************************************************************
		 * Gets the held state.
		 *
		 * If the manager is not holding a state, a failed assertion may be triggered.
		 *
		 * @return An immutable reference to the held state.
		 **************************************************************************************************************/
		const State& state() const noexcept;

		/**************************************************************************************************************
		 * Gets the benchmark that measures update time.
		 *
		 * @return A reference to the update benchmark.
		 **************************************************************************************************************/
		const tr::Benchmark& updateBenchmark() const noexcept;

		/**************************************************************************************************************
		 * Gets the benchmark that measures drawing time.
		 *
		 * @return A reference to the draw benchmark.
		 **************************************************************************************************************/
		const tr::Benchmark& drawBenchmark() const noexcept;

		/**************************************************************************************************************
		 * Passes an event to the held state.
		 *
		 * In case no state is held, this function does nothing.
		 *
		 * Any exceptions that occur in the state's @em handleEvent method will bubble up.
		 *
		 * @param[in] event The event to the state.
		 **************************************************************************************************************/
		void handleEvent(const tr::Event& event);

		/**************************************************************************************************************
		 * Updates the state.
		 *
		 * In case no state is held, this function does nothing.
		 *
		 * Any exceptions that occur in the state's @em update method will bubble up.
		 *
		 * @param[in] delta The time step since the last update.
		 **************************************************************************************************************/
		void update(tr::Duration delta);

		/**************************************************************************************************************
		 * Draws the state.
		 *
		 * In case no state is held, this function does nothing.
		 *
		 * Any exceptions that occur in the state's @em draw method will bubble up.
		 **************************************************************************************************************/
		void draw();

	  private:
		std::unique_ptr<State> _state;
		tr::Benchmark _updateBenchmark;
		tr::Benchmark _drawBenchmark;
	};

	/******************************************************************************************************************
	 * Generates a new valid state type ID.
	 *
	 * @return An unused state tpye ID.
	 ******************************************************************************************************************/
	std::uint32_t generateStateType() noexcept;
} // namespace tre
