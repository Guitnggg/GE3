#include "Audio.h"

#include "engine/core/diagnostics/HResult.h"

#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <propvarutil.h>

#include <algorithm>
#include <limits>
#include <stdexcept>

#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

using HResult::ThrowIfFailed;

Audio::~Audio()
{
	Finalize();
}

void Audio::Initialize(const std::filesystem::path& audioDirectory)
{
	if (initialized_) {
		return;
	}

	// 相対パスで指定された音声ファイルの基準フォルダを保持する
	audioDirectory_ = audioDirectory;

	// Media FoundationはWAVやMP3などをPCMへデコードするために使用する
	ThrowIfFailed(MFStartup(MF_VERSION), "Failed to initialize Media Foundation.");
	mediaFoundationStarted_ = true;

	try {
		// XAudio2本体と、最終的な音声出力を担当するマスターボイスを作る
		ThrowIfFailed(XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR), "Failed to initialize XAudio2.");
		ThrowIfFailed(xAudio2_->CreateMasteringVoice(&masteringVoice_), "Failed to create the XAudio2 mastering voice.");
		initialized_ = true;
	}
	catch (...) {
		Finalize();
		throw;
	}
}

void Audio::Finalize()
{
	// 再生をすべて止めてから、読み込み済みデータを破棄する
	StopAll();
	sounds_.clear();
	loadedPaths_.clear();

	// XAudio2関連を先に解放し、最後にMedia Foundationを終了する
	if (masteringVoice_ != nullptr) {
		masteringVoice_->DestroyVoice();
		masteringVoice_ = nullptr;
	}
	xAudio2_.Reset();

	if (mediaFoundationStarted_) {
		MFShutdown();
		mediaFoundationStarted_ = false;
	}

	initialized_ = false;
}

Audio::SoundHandle Audio::Load(const std::filesystem::path& fileName)
{
	EnsureInitialized();
	const std::filesystem::path path = ResolvePath(fileName).lexically_normal();

	// 同じパスの音声はデコード済みデータを再利用する
	if (const auto loaded = loadedPaths_.find(path); loaded != loadedPaths_.end()) {
		return loaded->second;
	}

	// デコードした音声へ新しいハンドルを割り当てる
	std::shared_ptr<SoundData> sound = Decode(path);
	if (nextSoundHandle_ == kInvalidSoundHandle) {
		throw std::overflow_error("Audio sound handle limit reached.");
	}

	const SoundHandle handle = nextSoundHandle_++;
	sounds_.emplace(handle, std::move(sound));
	loadedPaths_.emplace(path, handle);
	return handle;
}

void Audio::Unload(SoundHandle soundHandle)
{
	const auto sound = sounds_.find(soundHandle);
	if (sound == sounds_.end()) {
		return;
	}

	for (auto it = loadedPaths_.begin(); it != loadedPaths_.end(); ++it) {
		if (it->second == soundHandle) {
			loadedPaths_.erase(it);
			break;
		}
	}
	sounds_.erase(sound);
}

Audio::VoiceHandle Audio::Play(SoundHandle soundHandle, bool loop, float volume, float pitch)
{
	EnsureInitialized();
	const auto sound = sounds_.find(soundHandle);
	if (sound == sounds_.end()) {
		throw std::invalid_argument("Invalid audio sound handle.");
	}
	if (nextVoiceHandle_ == kInvalidVoiceHandle) {
		throw std::overflow_error("Audio voice handle limit reached.");
	}

	// 1回の再生ごとに独立したSourceVoiceを作るため、同じ音声を重ねて再生できる
	const auto* waveFormat = reinterpret_cast<const WAVEFORMATEX*>(sound->second->waveFormat.data());
	IXAudio2SourceVoice* sourceVoice = nullptr;
	ThrowIfFailed(
		xAudio2_->CreateSourceVoice(&sourceVoice, waveFormat, 0, 4.0f),
		"Failed to create an XAudio2 source voice.");

	try {
		// 再生パラメーターとPCMデータをSourceVoiceへ設定する
		ThrowIfFailed(sourceVoice->SetVolume((std::max)(0.0f, volume)), "Failed to set audio volume.");
		ThrowIfFailed(sourceVoice->SetFrequencyRatio(std::clamp(pitch, XAUDIO2_MIN_FREQ_RATIO, 4.0f)), "Failed to set audio pitch.");

		XAUDIO2_BUFFER buffer{};
		buffer.Flags = XAUDIO2_END_OF_STREAM;
		buffer.AudioBytes = static_cast<UINT32>(sound->second->pcmData.size());
		buffer.pAudioData = sound->second->pcmData.data();
		buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;

		ThrowIfFailed(sourceVoice->SubmitSourceBuffer(&buffer), "Failed to submit audio data to XAudio2.");
		ThrowIfFailed(sourceVoice->Start(), "Failed to start audio playback.");
	}
	catch (...) {
		sourceVoice->DestroyVoice();
		throw;
	}

	// 停止などの操作に使えるよう、再生中ボイスへハンドルを割り当てる
	const VoiceHandle handle = nextVoiceHandle_++;
	voices_.emplace(handle, PlayingVoice{ sourceVoice, sound->second, false });
	return handle;
}

void Audio::Stop(VoiceHandle voiceHandle)
{
	const auto voice = voices_.find(voiceHandle);
	if (voice == voices_.end()) {
		return;
	}
	DestroyVoice(voice->second);
	voices_.erase(voice);
}

void Audio::Pause(VoiceHandle voiceHandle)
{
	PlayingVoice* voice = FindVoice(voiceHandle);
	if (voice == nullptr || voice->paused) {
		return;
	}
	ThrowIfFailed(voice->sourceVoice->Stop(), "Failed to pause audio playback.");
	voice->paused = true;
}

void Audio::Resume(VoiceHandle voiceHandle)
{
	PlayingVoice* voice = FindVoice(voiceHandle);
	if (voice == nullptr || !voice->paused) {
		return;
	}
	ThrowIfFailed(voice->sourceVoice->Start(), "Failed to resume audio playback.");
	voice->paused = false;
}

void Audio::SetVolume(VoiceHandle voiceHandle, float volume)
{
	PlayingVoice* voice = FindVoice(voiceHandle);
	if (voice != nullptr) {
		ThrowIfFailed(voice->sourceVoice->SetVolume((std::max)(0.0f, volume)), "Failed to set audio volume.");
	}
}

void Audio::SetPitch(VoiceHandle voiceHandle, float pitch)
{
	PlayingVoice* voice = FindVoice(voiceHandle);
	if (voice != nullptr) {
		ThrowIfFailed(
			voice->sourceVoice->SetFrequencyRatio(std::clamp(pitch, XAUDIO2_MIN_FREQ_RATIO, 4.0f)),
			"Failed to set audio pitch.");
	}
}

bool Audio::IsPlaying(VoiceHandle voiceHandle) const
{
	const PlayingVoice* voice = FindVoice(voiceHandle);
	if (voice == nullptr || voice->paused) {
		return false;
	}

	XAUDIO2_VOICE_STATE state{};
	voice->sourceVoice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
	return state.BuffersQueued != 0;
}

void Audio::StopAll()
{
	for (auto& [handle, voice] : voices_) {
		(void)handle;
		DestroyVoice(voice);
	}
	voices_.clear();
}

void Audio::SetMasterVolume(float volume)
{
	EnsureInitialized();
	ThrowIfFailed(masteringVoice_->SetVolume((std::max)(0.0f, volume)), "Failed to set master audio volume.");
}

void Audio::Update()
{
	// 再生を終えたボイスを毎フレーム検出して解放する
	for (auto it = voices_.begin(); it != voices_.end();) {
		XAUDIO2_VOICE_STATE state{};
		it->second.sourceVoice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
		if (!it->second.paused && state.BuffersQueued == 0) {
			DestroyVoice(it->second);
			it = voices_.erase(it);
		}
		else {
			++it;
		}
	}
}

std::shared_ptr<Audio::SoundData> Audio::Decode(const std::filesystem::path& path) const
{
	// Media Foundationへ渡す前に、ファイルが存在することを確認する
	if (!std::filesystem::is_regular_file(path)) {
		throw std::runtime_error("Audio file was not found: " + path.string());
	}

	// SourceReaderを作成し、出力形式としてPCMを要求する
	Microsoft::WRL::ComPtr<IMFSourceReader> reader;
	ThrowIfFailed(
		MFCreateSourceReaderFromURL(path.c_str(), nullptr, &reader),
		"Failed to open the audio file with Media Foundation.");

	Microsoft::WRL::ComPtr<IMFMediaType> requestedType;
	ThrowIfFailed(MFCreateMediaType(&requestedType), "Failed to create the audio media type.");
	ThrowIfFailed(requestedType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio), "Failed to set the audio major type.");
	ThrowIfFailed(requestedType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM), "Failed to request PCM audio.");
	ThrowIfFailed(
		reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, requestedType.Get()),
		"The audio format could not be decoded to PCM.");

	// デコード後の音声形式をXAudio2用のWAVEFORMATEXへ変換する
	Microsoft::WRL::ComPtr<IMFMediaType> outputType;
	ThrowIfFailed(
		reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &outputType),
		"Failed to get the decoded audio format.");

	WAVEFORMATEX* allocatedWaveFormat = nullptr;
	UINT32 waveFormatSize = 0;
	ThrowIfFailed(
		MFCreateWaveFormatExFromMFMediaType(outputType.Get(), &allocatedWaveFormat, &waveFormatSize),
		"Failed to create the XAudio2 wave format.");

	auto sound = std::make_shared<SoundData>();
	sound->waveFormat.assign(
		reinterpret_cast<uint8_t*>(allocatedWaveFormat),
		reinterpret_cast<uint8_t*>(allocatedWaveFormat) + waveFormatSize);
	CoTaskMemFree(allocatedWaveFormat);

	// ファイル終端までサンプルを読み、連続したPCMデータへまとめる
	while (true) {
		DWORD flags = 0;
		Microsoft::WRL::ComPtr<IMFSample> sample;
		ThrowIfFailed(
			reader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &flags, nullptr, &sample),
			"Failed while decoding the audio file.");

		if (sample != nullptr) {
			Microsoft::WRL::ComPtr<IMFMediaBuffer> buffer;
			ThrowIfFailed(sample->ConvertToContiguousBuffer(&buffer), "Failed to access decoded audio data.");

			BYTE* bytes = nullptr;
			DWORD byteCount = 0;
			ThrowIfFailed(buffer->Lock(&bytes, nullptr, &byteCount), "Failed to lock decoded audio data.");
			try {
				if (sound->pcmData.size() > (std::numeric_limits<UINT32>::max)() - byteCount) {
					throw std::runtime_error("The decoded audio file is too large for one XAudio2 buffer.");
				}
				sound->pcmData.insert(sound->pcmData.end(), bytes, bytes + byteCount);
			}
			catch (...) {
				buffer->Unlock();
				throw;
			}
			buffer->Unlock();
		}

		if ((flags & MF_SOURCE_READERF_ENDOFSTREAM) != 0) {
			break;
		}
	}

	if (sound->pcmData.empty()) {
		throw std::runtime_error("The audio file did not contain any decodable samples.");
	}
	return sound;
}

std::filesystem::path Audio::ResolvePath(const std::filesystem::path& fileName) const
{
	return fileName.is_absolute() ? fileName : audioDirectory_ / fileName;
}

Audio::PlayingVoice* Audio::FindVoice(VoiceHandle voiceHandle)
{
	const auto voice = voices_.find(voiceHandle);
	return voice == voices_.end() ? nullptr : &voice->second;
}

const Audio::PlayingVoice* Audio::FindVoice(VoiceHandle voiceHandle) const
{
	const auto voice = voices_.find(voiceHandle);
	return voice == voices_.end() ? nullptr : &voice->second;
}

void Audio::DestroyVoice(PlayingVoice& voice)
{
	// SourceVoiceを停止してキューを空にしてから破棄する
	if (voice.sourceVoice != nullptr) {
		voice.sourceVoice->Stop();
		voice.sourceVoice->FlushSourceBuffers();
		voice.sourceVoice->DestroyVoice();
		voice.sourceVoice = nullptr;
	}
	voice.soundData.reset();
}

void Audio::EnsureInitialized() const
{
	if (!initialized_) {
		throw std::logic_error("Audio must be initialized before use.");
	}
}
