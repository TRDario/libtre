#include "../include/tre/state_manager.hpp"

tre::StateManager::StateManager(std::unique_ptr<State> state) noexcept
	: _state{std::move(state)}
{
}

bool tre::StateManager::has_state() const noexcept
{
	return _state != nullptr;
}

tre::State& tre::StateManager::state() noexcept
{
	assert(has_state());
	return *_state;
}

const tre::State& tre::StateManager::state() const noexcept
{
	assert(has_state());
	return *_state;
}

const tr::Benchmark& tre::StateManager::updateBenchmark() const noexcept
{
	return _updateBenchmark;
}

const tr::Benchmark& tre::StateManager::drawBenchmark() const noexcept
{
	return _drawBenchmark;
}

void tre::StateManager::handleEvent(const tr::Event& event)
{
	if (_state != nullptr) {
		auto next{_state->handleEvent(event)};
		if (next != nullptr) {
			_state = std::move(next);
		}
	}
}

void tre::StateManager::update(tr::Duration delta)
{
	if (_state != nullptr) {
		_updateBenchmark.start();
		auto next{_state->update(delta)};
		_updateBenchmark.stop();
		if (next != nullptr) {
			_state = std::move(next);
			_updateBenchmark.clear();
			_drawBenchmark.clear();
		}
	}
}

void tre::StateManager::draw()
{
	if (_state != nullptr) {
		_drawBenchmark.start();
		_state->draw();
		_drawBenchmark.stop();
	}
}

std::uint32_t tre::generateStateType() noexcept
{
	static std::uint32_t newID{0};
	return newID++;
}
