#pragma once
#include <bitset>
#include <list>
#include <thread>
#include <tr/tr.hpp>

namespace tre {
	/******************************************************************************************************************
	 * Concept denoting an iterator suitable for AudioStream::read.
	 ******************************************************************************************************************/
	template <class It>
	concept AudioStreamOutputIterator = std::contiguous_iterator<It> && std::output_iterator<It, std::int16_t>;

	/******************************************************************************************************************
	 * Helper class for reading and managing audio data streaming.
	 ******************************************************************************************************************/
	class AudioStream {
	  public:
		/**************************************************************************************************************
		 * Sentinel value representing the beginning of the streamed file.
		 **************************************************************************************************************/
		static constexpr int START{0};

		/**************************************************************************************************************
		 * Sentinel value representing the end of the streamed file.
		 **************************************************************************************************************/
		static constexpr int END{std::numeric_limits<int>::max()};

		/**************************************************************************************************************
		 * Creates an audio stream with data from a file.
		 *
		 * @exception tr::FileNotFound If the file isn't found.
		 * @exception tr::FileOpenError If opening the file fails.
		 * @exception tr::UnsupportedAudioFile If the file is an unsupported or invalid format.
		 *
		 * @param[in] path The path to an audio file.
		 **************************************************************************************************************/
		AudioStream(const std::filesystem::path& path);

		/**************************************************************************************************************
		 * Creates an audio stream with data from a span.
		 *
		 * @param[in] view The view over audio data to be streamed.
		 * @param[in] channels The number of channels in the audio data. Must be 1 or 2.
		 * @param[in] sampleRate The sample rate of the audio data. Must be higher than 0.
		 **************************************************************************************************************/
		AudioStream(std::span<const std::int16_t> view, int channels, int sampleRate) noexcept;

		/**************************************************************************************************************
		 * Gets the length of the audio stream.
		 *
		 * @return The length of the audio stream in samples.
		 **************************************************************************************************************/
		int length() const noexcept;

		/**************************************************************************************************************
		 * Gets the number of channels in the audio stream.
		 *
		 * @return The number of channels in the audio stream.
		 **************************************************************************************************************/
		int channels() const noexcept;

		/**************************************************************************************************************
		 * Gets the sample rate of the audio stream.
		 *
		 * @return The stream sample rate in Hz.
		 **************************************************************************************************************/
		int sampleRate() const noexcept;

		/**************************************************************************************************************
		 * Gets whether the audio stream is looping.
		 *
		 * @return True if the stream is looping and false otherwise.
		 **************************************************************************************************************/
		bool looping() const noexcept;

		/**************************************************************************************************************
		 * Sets whether the audio stream is looping.
		 *
		 * @param[in] looping Whether the audio stream should loop.
		 **************************************************************************************************************/
		void setLooping(bool looping) noexcept;

		/**************************************************************************************************************
		 * Gets the loop start of the stream.
		 *
		 * @return The offset to the sample marking the start of the loop.
		 **************************************************************************************************************/
		int loopStart() const noexcept;

		/**************************************************************************************************************
		 * Sets the loop start of the stream.
		 *
		 * @param[in] loopStart The offset to the sample marking the start of the loop. If the offset is invalid, it
		 *                      will be clamped to the valid range of [0, loopEnd()).
		 **************************************************************************************************************/
		void setLoopStart(int loopStart) noexcept;

		/**************************************************************************************************************
		 * Gets the loop end of the stream.
		 *
		 * @return The offset to the sample marking the end of the loop.
		 **************************************************************************************************************/
		int loopEnd() const noexcept;

		/**************************************************************************************************************
		 * Sets the loop end of the stream.
		 *
		 * @param[in] loopEnd The offset to the sample marking the end of the loop. If the offset is invalid, it will
		 *                    be clamped to the valid range of (loopStart(), length()].
		 **************************************************************************************************************/
		void setLoopEnd(int loopEnd) noexcept;

		/**************************************************************************************************************
		 * Gets the current playback position within the stream.
		 *
		 * @return The offset to the sample next in line to be read.
		 **************************************************************************************************************/
		int position() const noexcept;

		/**************************************************************************************************************
		 * Sets the playback position within the stream.
		 *
		 * @param[in] offset The new offset within the stream. If the offset is invalid, it will be clamped to the valid
		 *                   range of [loopStart(), loopEnd()).
		 **************************************************************************************************************/
		void seek(int offset) noexcept;

		/**************************************************************************************************************
		 * Reads from the stream.
		 *
		 * @tparam It an iterator belonging to a contiguous range of 16-bit integers.
		 *
		 * @param[in] it The beginning iterator to the range to output to.
		 * @param[in] samples The number of samples to read into the range. The capacity of the range must be at least
		 *                    samples * channels().
		 **************************************************************************************************************/
		template <AudioStreamOutputIterator It> void read(It it, int samples) noexcept
		{
			read(std::to_address(it), samples);
		}

	  private:
		struct FileCloser {
			void operator()(void* file) const noexcept;
		};
		using File = std::unique_ptr<void, FileCloser>;
		struct View {
			const std::int16_t* start;
			const std::int16_t* cur;
		};

		std::variant<File, View> _source;
		int                      _length;
		int                      _channels;
		int                      _sampleRate;
		bool                     _looping;
		int                      _loopStart;
		int                      _loopEnd;

		void read(std::int16_t* out, int samples) noexcept;
	};

	/******************************************************************************************************************
	 * Audio source.
	 *
	 * Cannot be instantiated directly, see AudioSystem.
	 ******************************************************************************************************************/
	class AudioSource {
	  public:
		/**************************************************************************************************************
		 * Sentinel value representing the beginning of the streamed file.
		 **************************************************************************************************************/
		static constexpr tr::SecondsF START{tr::SecondsF::zero()};

		/**************************************************************************************************************
		 * Sentinel value representing the end of the streamed file.
		 **************************************************************************************************************/
		static constexpr tr::SecondsF END{tr::SecondsF::max()};

		/**************************************************************************************************************
		 * Sets the audio buffer that the source uses.
		 *
		 * @param[in] buffer The buffer the source will pull data from while playing.
		 **************************************************************************************************************/
		void use(tr::AudioBufferView buffer) noexcept;

		/**************************************************************************************************************
		 * Sets the audio buffer that the source uses.
		 *
		 * @exception std::bad_alloc If an internal allocation fails.
		 *
		 * @param[in] stream The buffer the source will pull data from while playing.
		 **************************************************************************************************************/
		void use(AudioStream stream);

		/**************************************************************************************************************
		 * Gets the audio classes the source belongs to.
		 *
		 * @return A reference to the bitset of audio classes of the source.
		 **************************************************************************************************************/
		const std::bitset<32>& classes() const noexcept;

		/**************************************************************************************************************
		 * Sets the audio classes the source belongs to.
		 *
		 * This will adjust the gain of the audio source to match the new classes.
		 *
		 * @param classes The new audio class bitset.
		 **************************************************************************************************************/
		void setClasses(std::bitset<32> classes) noexcept;

		/**************************************************************************************************************
		 * Gets the pitch of the source.
		 *
		 * @return The pitch multiplier of the source.
		 **************************************************************************************************************/
		float pitch() const noexcept;

		/**************************************************************************************************************
		 * Sets the pitch (and speed) of the source.
		 *
		 * @param[in] pitch The pitch multiplier of the source, clamped to [0.5, 2.0].
		 **************************************************************************************************************/
		void setPitch(float pitch) noexcept;

		/**************************************************************************************************************
		 * Gets the gain of the source.
		 *
		 * The gain of the source is multiplied by the gain factors of the audio classes it belongs to.
		 *
		 * @return The gain multiplier of the source.
		 **************************************************************************************************************/
		float gain() const noexcept;

		/**************************************************************************************************************
		 * Sets the gain of the source.
		 *
		 * @param[in] gain The gain multiplier of the source, clamped to a non-negative value.
		 **************************************************************************************************************/
		void setGain(float gain) noexcept;

		/**************************************************************************************************************
		 * Gets the distance where the source will no longer be attenuated any further.
		 *
		 * @return The maximum distance of the source.
		 **************************************************************************************************************/
		float maxDistance() const noexcept;

		/**************************************************************************************************************
		 * Sets the distance where the source will no longer be attenuated any further.
		 *
		 * @param[in] maxDistance The maximum distance of the source, clamped to a non-negative value.
		 **************************************************************************************************************/
		void setMaxDistance(float maxDistance) noexcept;

		/**************************************************************************************************************
		 * Gets the distance rolloff factor of the source.
		 *
		 * @return The distance rolloff factor of the source.
		 **************************************************************************************************************/
		float rolloff() const noexcept;

		/**************************************************************************************************************
		 * Sets the distance rolloff factor of the source.
		 *
		 * @param[in] rolloff The distance rolloff factor of the source, clamped to a non-negative value.
		 **************************************************************************************************************/
		void setRolloff(float rolloff) noexcept;

		/**************************************************************************************************************
		 * Gets the reference distance of the source, where there is no attenuation.
		 *
		 * @return Gets the reference distance of the source, where there is no attenuation.
		 **************************************************************************************************************/
		float referenceDistance() const noexcept;

		/**************************************************************************************************************
		 * Sets the reference distance of the source, where there is no attenuation.
		 *
		 * @param[in] referenceDistance The new reference distance of the source, clamped to a non-negative value.
		 **************************************************************************************************************/
		void setReferenceDistance(float referenceDistance) noexcept;

		/**************************************************************************************************************
		 * Gets the gain multiplier applied when the listener is outside the source's outer cone angle.
		 *
		 * @return The outer cone gain multiplier.
		 **************************************************************************************************************/
		float outerConeGain() const noexcept;

		/**************************************************************************************************************
		 * Sets the gain multiplier applied when the listener is outside the source's outer cone angle.
		 *
		 * @param[in] outGain The new gain multiplier, clamped to [0.0, 1.0].
		 **************************************************************************************************************/
		void setOuterConeGain(float outGain) noexcept;

		/**************************************************************************************************************
		 * Gets the width of the inner cone of the source (where no direction attenuation is done).
		 *
		 * @return The width of the inner cone of the source.
		 **************************************************************************************************************/
		tr::AngleF innerConeWidth() const noexcept;

		/**************************************************************************************************************
		 * Sets the width of the inner cone of the source (where no direction attenuation is done).
		 *
		 * @param[in] inConeW The new width, clamped to [0.0, outerConeWidth()].
		 **************************************************************************************************************/
		void setInnerConeWidth(tr::AngleF inConeW) noexcept;

		/**************************************************************************************************************
		 * Gets the width of the outer cone of the source (where direction attenuation is done).
		 *
		 * @return The width of the outer cone of the source.
		 **************************************************************************************************************/
		tr::AngleF outerConeWidth() const noexcept;

		/**************************************************************************************************************
		 * Sets the width of the outer cone of the source (where direction attenuation is done).
		 *
		 * @param[in] outConeW The new width, clamped to [innerConeWidth(), 360 degrees].
		 **************************************************************************************************************/
		void setOuterConeWidth(tr::AngleF outConeW) noexcept;

		/**************************************************************************************************************
		 * Gets the position of the source.
		 *
		 * @return The position vector of the audio source.
		 **************************************************************************************************************/
		glm::vec3 position() const noexcept;

		/**************************************************************************************************************
		 * Sets the position of the source.
		 *
		 * @param[in] position The position of the source.
		 **************************************************************************************************************/
		void setPosition(const glm::vec3& position) noexcept;

		/**************************************************************************************************************
		 * Gets the velocity of the source.
		 *
		 * @return The velocity vector of the audio source.
		 **************************************************************************************************************/
		glm::vec3 velocity() const noexcept;

		/**************************************************************************************************************
		 * Sets the velocity of the source.
		 *
		 * @param[in] velocity The velocity of the source.
		 **************************************************************************************************************/
		void setVelocity(const glm::vec3& velocity) noexcept;

		/**************************************************************************************************************
		 * Gets the direction of the source cone.
		 *
		 * @return The direction vector of the audio source.
		 **************************************************************************************************************/
		glm::vec3 direction() const noexcept;

		/**************************************************************************************************************
		 * Sets the direction of the source cone.
		 *
		 * @param[in] direction The direction of the source cone. Can also be OMNIDRECTIONAL.
		 **************************************************************************************************************/
		void setDirection(const glm::vec3& direction) noexcept;

		/**************************************************************************************************************
		 * Gets the origin of the source's position.
		 *
		 * @return The origin of the audio source.
		 **************************************************************************************************************/
		tr::AudioOrigin origin() const noexcept;

		/**************************************************************************************************************
		 * Sets the origin of the source's position.
		 *
		 * @param[in] type The new origin type.
		 **************************************************************************************************************/
		void setOrigin(tr::AudioOrigin type) noexcept;

		/**************************************************************************************************************
		 * Gets whether the source is looping.
		 *
		 * @return True if the source is looping, and false otherwise.
		 **************************************************************************************************************/
		bool looping() const noexcept;

		/**************************************************************************************************************
		 * Sets whether the source is looping.
		 *
		 * @param[in] looping Whether the source should loop.
		 **************************************************************************************************************/
		void setLooping(bool looping) noexcept;

		/**************************************************************************************************************
		 * Gets the state of the audio source.
		 *
		 * @return The state of the audio source.
		 **************************************************************************************************************/
		tr::AudioState state() const noexcept;

		/**************************************************************************************************************
		 * Plays the source.
		 **************************************************************************************************************/
		void play() noexcept;

		/**************************************************************************************************************
		 * Pauses the source.
		 **************************************************************************************************************/
		void pause() noexcept;

		/**************************************************************************************************************
		 * Stops the source and rewinds it to the beginning.
		 **************************************************************************************************************/
		void stop() noexcept;

		/**************************************************************************************************************
		 * Gets the length of the source audio.
		 *
		 * @return The length of the playing audio in seconds.
		 **************************************************************************************************************/
		tr::SecondsF length() const noexcept;

		/**************************************************************************************************************
		 * Gets the source's playback position within the current buffer.
		 *
		 * @return The source's playback position within the current buffer in seconds.
		 **************************************************************************************************************/
		tr::SecondsF offset() const noexcept;

		/**************************************************************************************************************
		 * Sets the source's playback position within the current buffer.
		 *
		 * @param[in] offset The new playback position within the current buffer in seconds.
		 **************************************************************************************************************/
		void setOffset(tr::SecondsF offset) noexcept;

		/**************************************************************************************************************
		 * Gets a streamed source's starting loop point.
		 *
		 * This function cannot be called on a non-streamed source.
		 *
		 * @return The stream's starting loop point within the file in seconds.
		 **************************************************************************************************************/
		tr::SecondsF loopStart() const noexcept;

		/**************************************************************************************************************
		 * Sets a streamed source's starting loop point.
		 *
		 * This function cannot be called on a non-streamed source.
		 *
		 * @return The stream's ending loop point within the file in seconds.
		 **************************************************************************************************************/
		tr::SecondsF loopEnd() const noexcept;

		/**************************************************************************************************************
		 * Sets a streamed source's loop points.
		 *
		 * This function cannot be called on a non-streamed source.
		 *
		 * @param[in] start The starting loop point (special value: START). This value will be clamped to a valid range.
		 * @param[in] end The ending loop point (special value: END). This value will be clamped to a valid range.
		 **************************************************************************************************************/
		void setLoopPoints(tr::SecondsF start, tr::SecondsF end) noexcept;

	  private:
		// Extension over the public audio stream interface to embed audio buffers.
		struct Stream : AudioStream {
			// Extension over the audio buffer to embed the starting file offset of the data in the buffer.
			struct Buffer : tr::AudioBuffer {
				int startFileOffset;
			};

			std::array<Buffer, 4> buffers;

			void refillBuffer(Buffer& buffer);
		};

		std::unique_ptr<Stream> _stream;
		tr::AudioSource         _source;
		float                   _gain;
		std::bitset<32>         _classes;

		AudioSource() noexcept;

		friend class AudioManager;
	};

	/******************************************************************************************************************
	 * Audio system manager with support for streaming and gradual transitions.
	 *
	 * Only one instance of the audio system is allowed to exist at a time.
	 *
	 * tr::AudioSystem should be initialized before working with this class, though it can safely be initialized.
	 ******************************************************************************************************************/
	class AudioManager {
	  public:
		/**************************************************************************************************************
		 * Initializes the audio manager.
		 **************************************************************************************************************/
		AudioManager() noexcept;

		~AudioManager() noexcept;

		/**************************************************************************************************************
		 * Gets an audio class's gain modifier.
		 *
		 * @param[in] id The class id. Must be in the range 0-31.
		 *
		 * @return The gain modifier of the class.
		 **************************************************************************************************************/
		float getClassGain(int id) const noexcept;

		/**************************************************************************************************************
		 * Sets an audio class's gain modifier.
		 *
		 * @param[in] id The class id. Must be in the range 0-31.
		 * @param[in] gain The new gain modifier.
		 **************************************************************************************************************/
		void setClassGain(int id, float gain) noexcept;

		/**************************************************************************************************************
		 * Creates an audio source.
		 *
		 * @exception std::bad_alloc If allocating the source fails.
		 * @exception tr::AudioSourceBadAlloc If allocating the source fails.
		 *
		 * @return A shared pointer to the audio source. The audio system will deallocate the source when all other
		 *         shared pointers to it are deleted and the source has ceased to play.
		 **************************************************************************************************************/
		std::shared_ptr<AudioSource> newSource();

	  private:
		std::list<std::shared_ptr<AudioSource>> _sources;
		std::array<float, 32>                   _classGains;
		std::thread                             _thread;
		bool                                    _threadActive;
		std::mutex                              _mutex;

		void thread() noexcept;

		friend class AudioSource;
	};

	/******************************************************************************************************************
	 * Gets whether the audio manager is active.
	 *
	 * @return True if the audio manager is active, and false otherwise.
	 ******************************************************************************************************************/
	bool audioManagerActive() noexcept;

	/******************************************************************************************************************
	 * Gets a reference to the active audio manager. This function cannot be called if the audio manager isn't active.
	 *
	 * @return A reference to the audio manager.
	 ******************************************************************************************************************/
	AudioManager& audioManager() noexcept;

} // namespace tre