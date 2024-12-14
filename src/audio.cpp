#include "../include/tre/audio.hpp"
#include <algorithm>
#include <boost/container/static_vector.hpp>
#include <sndfile.h>

using namespace std::chrono_literals;

namespace tre {
	AudioManager* _audioManager{nullptr};
}

tre::AudioStream::AudioStream(const std::filesystem::path& path)
	: _looping{false}, _loopStart{0}
{
	if (!is_regular_file(path)) {
		throw tr::FileNotFound{path};
	}

	SF_INFO info;
#ifdef _WIN32
	std::unique_ptr<SNDFILE, decltype(&sf_close)> file{sf_wchar_open(path.c_str(), SFM_READ, &info), sf_close};
#else
	std::unique_ptr<SNDFILE, decltype(&sf_close)> file{sf_open(path.c_str(), SFM_READ, &info), sf_close};
#endif

	if (file == nullptr) {
		throw tr::FileOpenError{path};
	}
	if (info.channels > 2) {
		throw tr::UnsupportedAudioFile{path};
	}
	if (info.format & (SF_FORMAT_OGG | SF_FORMAT_VORBIS | SF_FORMAT_FLOAT | SF_FORMAT_DOUBLE)) {
		sf_command(file.get(), SFC_SET_SCALE_FLOAT_INT_READ, nullptr, true);
	}

	_source     = File{file.release()};
	_channels   = info.channels;
	_length     = info.frames;
	_sampleRate = info.samplerate;
	_loopEnd    = _length;
}

tre::AudioStream::AudioStream(std::span<const std::int16_t> view, int channels, int sampleRate) noexcept
	: _source{std::in_place_type<View>, view.data(), view.data()}
	, _length{int(view.size())}
	, _channels{channels}
	, _sampleRate{sampleRate}
	, _looping{false}
	, _loopStart{0}
	, _loopEnd{_length}
{
	assert(!view.empty());
	assert(channels == 1 || channels == 2);
	assert(sampleRate > 0);
}

void tre::AudioStream::FileCloser::operator()(void* file) const noexcept
{
	sf_close((SNDFILE*)(file));
}

int tre::AudioStream::length() const noexcept
{
	return _length;
}

int tre::AudioStream::channels() const noexcept
{
	return _channels;
}

int tre::AudioStream::sampleRate() const noexcept
{
	return _sampleRate;
}

bool tre::AudioStream::looping() const noexcept
{
	return _looping;
}

void tre::AudioStream::setLooping(bool looping) noexcept
{
	_looping = looping;
	if (looping && position() >= loopEnd()) {
		seek(loopStart());
	}
}

int tre::AudioStream::loopStart() const noexcept
{
	return _loopStart;
}

void tre::AudioStream::setLoopStart(int loopStart) noexcept
{
	_loopStart = std::clamp(loopStart, 0, loopEnd() - 1);
}

int tre::AudioStream::loopEnd() const noexcept
{
	return _loopEnd;
}

void tre::AudioStream::setLoopEnd(int loopEnd) noexcept
{
	_loopEnd = std::clamp(loopEnd, loopStart() + 1, length());
	if (looping() && position() >= _loopEnd) {
		seek(loopStart());
	}
}

int tre::AudioStream::position() const noexcept
{
	auto fileOffset{[&](const File& file) { return sf_seek((SNDFILE*)(file.get()), 0, SF_SEEK_CUR); }};
	auto viewOffset{[&](const View& view) { return (view.cur - view.start) / _channels; }};
	return std::visit(tr::Overloaded{fileOffset, viewOffset}, _source);
}

void tre::AudioStream::seek(int offset) noexcept
{
	if (looping() && offset >= loopEnd()) {
		offset = loopStart();
	}
	else {
		offset = std::clamp(offset, START, length());
	}

	auto fileSeek{[&](File& file) { sf_seek((SNDFILE*)(file.get()), offset, SF_SEEK_SET); }};
	auto viewSeek{[&](View& view) { view.cur = view.start + offset; }};
	std::visit(tr::Overloaded{fileSeek, viewSeek}, _source);
}

void tre::AudioStream::read(std::int16_t* out, int samples) noexcept
{
	auto fileRead{[&](File& file) { sf_readf_short((SNDFILE*)(file.get()), out, samples); }};
	auto viewRead{[&](View& view) {
		auto end{view.cur + samples * _channels};
		assert(end <= view.start + _length * _channels);
		std::copy(view.cur, end, out);
		view.cur = end;
	}};
	std::visit(tr::Overloaded{fileRead, viewRead}, _source);
}

void tre::AudioSource::Stream::refillBuffer(Buffer& buffer)
{
	constexpr int AUDIO_STREAM_BUFFER_SIZE{16384};

	static boost::container::static_vector<std::int16_t, AUDIO_STREAM_BUFFER_SIZE * 2> dataBuffer;
	buffer.startFileOffset = position();
	auto samplesLeft{(looping() ? loopEnd() : length()) - buffer.startFileOffset};
	auto samplesToRead{std::min<int>(AUDIO_STREAM_BUFFER_SIZE, samplesLeft)};
	dataBuffer.resize(samplesToRead * channels());
	read(dataBuffer.begin(), samplesToRead);
	buffer.set(tr::rangeBytes(dataBuffer), channels() == 2 ? tr::AudioFormat::STEREO16 : tr::AudioFormat::MONO16,
			   sampleRate());
	if (samplesToRead < AUDIO_STREAM_BUFFER_SIZE && looping()) {
		seek(loopStart());
	}
}

tre::AudioSource::AudioSource() noexcept
	: _gain{1.0f}
{
}

void tre::AudioSource::use(tr::AudioBufferView buffer) noexcept
{
	std::lock_guard lock{audioManager()._mutex};
	_stream.reset();
	_source.setBuffer(buffer);
}

void tre::AudioSource::use(AudioStream stream)
{
	std::lock_guard lock{audioManager()._mutex};
	_source.setBuffer(std::nullopt);
	_stream = std::make_unique<Stream>(std::move(stream));
}

const std::bitset<32>& tre::AudioSource::classes() const noexcept
{
	return _classes;
}

void tre::AudioSource::setClasses(std::bitset<32> classes) noexcept
{
	_classes = classes;
	setGain(gain());
}

float tre::AudioSource::pitch() const noexcept
{
	return _source.pitch();
}

void tre::AudioSource::setPitch(float pitch) noexcept
{
	return _source.setPitch(pitch);
}

float tre::AudioSource::gain() const noexcept
{
	return _gain;
}

void tre::AudioSource::setGain(float gain) noexcept
{
	float multiplier{1};
	for (int i = 0; i < 32; ++i) {
		if (_classes[i]) {
			multiplier *= audioManager()._classGains[i];
		}
	}

	_gain = std::clamp(gain, 0.0f, 1.0f);
	_source.setGain(_gain * multiplier);
}

float tre::AudioSource::maxDistance() const noexcept
{
	return _source.maxDistance();
}

void tre::AudioSource::setMaxDistance(float maxDistance) noexcept
{
	_source.setMaxDistance(maxDistance);
}

float tre::AudioSource::rolloff() const noexcept
{
	return _source.rolloff();
}

void tre::AudioSource::setRolloff(float rolloff) noexcept
{
	_source.setRolloff(rolloff);
}

float tre::AudioSource::referenceDistance() const noexcept
{
	return _source.referenceDistance();
}

void tre::AudioSource::setReferenceDistance(float referenceDistance) noexcept
{
	_source.setReferenceDistance(referenceDistance);
}

float tre::AudioSource::outerConeGain() const noexcept
{
	return _source.outerConeGain();
}

void tre::AudioSource::setOuterConeGain(float outGain) noexcept
{
	_source.setOuterConeGain(outGain);
}

tr::AngleF tre::AudioSource::innerConeWidth() const noexcept
{
	return _source.innerConeWidth();
}

void tre::AudioSource::setInnerConeWidth(tr::AngleF inConeW) noexcept
{
	_source.setInnerConeWidth(inConeW);
}

tr::AngleF tre::AudioSource::outerConeWidth() const noexcept
{
	return _source.outerConeWidth();
}

void tre::AudioSource::setOuterConeWidth(tr::AngleF outConeW) noexcept
{
	_source.setOuterConeWidth(outConeW);
}

glm::vec3 tre::AudioSource::position() const noexcept
{
	return _source.position();
}

void tre::AudioSource::setPosition(const glm::vec3& position) noexcept
{
	_source.setPosition(position);
}

glm::vec3 tre::AudioSource::velocity() const noexcept
{
	return _source.velocity();
}

void tre::AudioSource::setVelocity(const glm::vec3& velocity) noexcept
{
	_source.setVelocity(velocity);
}

glm::vec3 tre::AudioSource::direction() const noexcept
{
	return _source.direction();
}

void tre::AudioSource::setDirection(const glm::vec3& direction) noexcept
{
	_source.setDirection(direction);
}

tr::AudioOrigin tre::AudioSource::origin() const noexcept
{
	return _source.origin();
}

void tre::AudioSource::setOrigin(tr::AudioOrigin type) noexcept
{
	_source.setOrigin(type);
}

bool tre::AudioSource::looping() const noexcept
{
	return _stream != nullptr ? _stream->looping() : _source.looping();
}

void tre::AudioSource::setLooping(bool looping) noexcept
{
	if (_stream != nullptr) {
		std::lock_guard lock{audioManager()._mutex};
		_stream->setLooping(looping);
	}
	else {
		_source.setLooping(looping);
	}
}

tr::AudioState tre::AudioSource::state() const noexcept
{
	return _source.state();
}

void tre::AudioSource::play() noexcept
{
	if (_stream != nullptr) {
		std::lock_guard lock{audioManager()._mutex};
		if (state() == tr::AudioState::INITIAL || state() == tr::AudioState::STOPPED) {
			_source.setBuffer(std::nullopt);
			for (auto& buffer : _stream->buffers) {
				_stream->refillBuffer(buffer);
				_source.queueBuffer(buffer);
				if (_stream->position() == _stream->length()) {
					break;
				}
			}
		}
	}
	_source.play();
}

void tre::AudioSource::pause() noexcept
{
	_source.pause();
}

void tre::AudioSource::stop() noexcept
{
	if (_stream != nullptr) {
		std::lock_guard lock{audioManager()._mutex};
		_source.stop();
		_stream->seek(_stream->loopStart());
	}
	else {
		_source.stop();
	}
}

tr::SecondsF tre::AudioSource::length() const noexcept
{
	if (_stream != nullptr) {
		return tr::SecondsF{float(_stream->length()) / _stream->sampleRate()};
	}
	else {
		auto buffer{_source.buffer()};
		return buffer.has_value() ? buffer->length() : tr::SecondsF::zero();
	}
}

tr::SecondsF tre::AudioSource::offset() const noexcept
{
	if (_stream != nullptr) {
		std::lock_guard lock{audioManager()._mutex};
		const auto      state{this->state()};
		if (state == tr::AudioState::INITIAL || state == tr::AudioState::STOPPED) {
			return tr::SecondsF{_stream->position() / float(_stream->sampleRate())};
		}
		auto& buffer{*std::ranges::find(_stream->buffers, _source.buffer())};
		return tr::SecondsF{buffer.startFileOffset / float(_stream->sampleRate())} + _source.offset();
	}
	else {
		return _source.offset();
	}
}

void tre::AudioSource::setOffset(tr::SecondsF offset) noexcept
{
	if (_stream != nullptr) {
		auto state{this->state()};
		/* LOCK SCOPE */ {
			std::lock_guard lock{audioManager()._mutex};
			_stream->seek(offset.count() * _stream->sampleRate());
		}
		_source.stop();
		switch (state) {
		case tr::AudioState::PLAYING:
			play();
			break;
		case tr::AudioState::PAUSED:
			play();
			pause();
			break;
		case tr::AudioState::INITIAL:
		case tr::AudioState::STOPPED:
			break;
		}
	}
	else {
		_source.setOffset(offset);
	}
}

tr::SecondsF tre::AudioSource::loopStart() const noexcept
{
	assert(_stream != nullptr);
	return tr::SecondsF{float(_stream->loopStart()) / _stream->sampleRate()};
}

tr::SecondsF tre::AudioSource::loopEnd() const noexcept
{
	assert(_stream != nullptr);
	return tr::SecondsF{float(_stream->loopEnd()) / _stream->sampleRate()};
}

void tre::AudioSource::setLoopPoints(tr::SecondsF start, tr::SecondsF end) noexcept
{
	assert(_stream != nullptr);
	assert(start < end);
	start = std::clamp(start, START, length());
	end   = std::clamp(end, START, length());
	std::lock_guard lock{audioManager()._mutex};
	if (start >= loopEnd()) {
		_stream->setLoopEnd(end.count() * _stream->sampleRate());
		_stream->setLoopStart(start.count() * _stream->sampleRate());
	}
	else {
		_stream->setLoopStart(start.count() * _stream->sampleRate());
		_stream->setLoopEnd(end.count() * _stream->sampleRate());
	}
}

tre::AudioManager::AudioManager() noexcept
	: _threadActive{false}
{
	assert(!audioManagerActive());
	_classGains.fill(1.0f);
	_audioManager = this;
}

tre::AudioManager::~AudioManager() noexcept
{
	_threadActive = false;
	if (_thread.joinable()) {
		_thread.join();
	}
}

float tre::AudioManager::getClassGain(int id) const noexcept
{
	return _classGains[id];
}

void tre::AudioManager::setClassGain(int id, float gain) noexcept
{
	_classGains[id] = gain;
	for (AudioSource& source : _sources | std::views::transform([](auto ptr) -> auto& { return *ptr; })) {
		if (source._classes[id]) {
			source.setGain(source.gain());
		}
	}
}

std::shared_ptr<tre::AudioSource> tre::AudioManager::newSource()
{
	std::lock_guard lock{_mutex};
	_sources.emplace_back(new AudioSource{});
	if (!_threadActive) {
		if (_thread.joinable()) {
			_thread.join();
		}
		_threadActive = true;
		_thread       = std::thread{&AudioManager::thread, this};
	}
	return _sources.back();
}

void tre::AudioManager::thread() noexcept
{
	while (_threadActive) {
		try {
			std::lock_guard lock{_mutex};

			std::ranges::remove_if(_sources, [](const auto& ptr) {
				return ptr.use_count() == 1 && ptr->state() != tr::AudioState::PLAYING;
			});
			if (_sources.empty()) {
				_threadActive = false;
				return;
			}

			for (AudioSource& source : _sources | std::views::transform([](auto& ptr) -> auto& { return *ptr; })) {
				if (source._stream != nullptr) {
					auto& stream{*source._stream};

					boost::container::static_vector<tr::AudioBufferView, 4> buffers;
					while (source._source.processedBuffers() > 0) {
						buffers.push_back(source._source.unqueueBuffer());
					}
					for (auto& buffer : buffers) {
						stream.refillBuffer(*std::ranges::find(stream.buffers, buffer));
						source._source.queueBuffer(buffer);
						if (stream.position() == stream.length()) {
							break;
						}
					}
				}
			}
		}
		catch (...) {
		}
		std::this_thread::sleep_for(10ms);
	}
}

bool tre::audioManagerActive() noexcept
{
	return _audioManager != nullptr;
}

tre::AudioManager& tre::audioManager() noexcept
{
	assert(audioManagerActive());
	return *_audioManager;
}