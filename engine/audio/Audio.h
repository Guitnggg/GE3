#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <wrl.h>
#include <xaudio2.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/// <summary>
/// XAudio2による音声の読み込みと再生を管理するクラス。
/// 音声ファイルのデコードにはMedia Foundationを使用する。
/// </summary>
class Audio {
public:
	using SoundHandle = uint32_t;
	using VoiceHandle = uint64_t;

	static constexpr SoundHandle kInvalidSoundHandle = 0;
	static constexpr VoiceHandle kInvalidVoiceHandle = 0;

	Audio() = default;
	~Audio();
	Audio(const Audio&) = delete;
	Audio& operator=(const Audio&) = delete;

	/// <summary>
	/// XAudio2とMedia Foundationを初期化する。
	/// </summary>
	/// <param name="audioDirectory">音声ファイルを格納する基準ディレクトリ</param>
	void Initialize(const std::filesystem::path& audioDirectory = L"audio");

	/// <summary>
	/// 再生中の音声と読み込み済みデータをすべて解放する。
	/// </summary>
	void Finalize();

	/// <summary>
	/// 音声ファイルを読み込む。相対パスはInitializeで指定したディレクトリを基準にする。
	/// WAV、MP3、AACなどMedia Foundationがデコード可能な形式を利用できる。
	/// </summary>
	SoundHandle Load(const std::filesystem::path& fileName);

	/// <summary>
	/// 読み込み済み音声データを解放する。
	/// </summary>
	void Unload(SoundHandle soundHandle);

	/// <summary>
	/// 音声を再生する。同じSoundHandleを同時に複数回再生できる。
	/// </summary>
	/// <param name="soundHandle">Loadが返したハンドル</param>
	/// <param name="loop">trueなら停止するまでループする</param>
	/// <param name="volume">音量。0.0fが無音、1.0fが元音量</param>
	/// <param name="pitch">ピッチ兼再生速度。1.0fが元の高さ</param>
	VoiceHandle Play(SoundHandle soundHandle, bool loop = false, float volume = 1.0f, float pitch = 1.0f);

	/// <summary>
	/// 指定した再生中の音声を停止する。
	/// </summary>
	void Stop(VoiceHandle voiceHandle);
	
	/// <summary>
	/// 指定した再生中の音声を一時停止する。
	/// </summary>
	void Pause(VoiceHandle voiceHandle);
	
	/// <summary>
	/// 一時停止中の音声を再開する。
	/// </summary>
	void Resume(VoiceHandle voiceHandle);
	
	/// <summary>
	/// 再生中の音声の音量を変更する。
	/// </summary>
	void SetVolume(VoiceHandle voiceHandle, float volume);
	
	/// <summary>
	/// 再生中の音声のピッチと再生速度を変更する。
	/// </summary>
	void SetPitch(VoiceHandle voiceHandle, float pitch);
	
	/// <summary>
	/// 指定した音声が再生中かを返す。
	/// </summary>
	bool IsPlaying(VoiceHandle voiceHandle) const;

	/// <summary>
	/// すべての再生を停止する。
	/// </summary>
	void StopAll();

	/// <summary>
	/// 全音声に適用されるマスター音量を設定する。
	/// </summary>
	void SetMasterVolume(float volume);

	/// <summary>
	/// 再生を終えたVoiceを回収する。毎フレーム呼び出す。
	/// </summary>
	void Update();

private:
	/// <summary>
	/// デコード済み音声1件分の共有データ。
	/// </summary>
	struct SoundData {
		std::vector<uint8_t> waveFormat; // XAudio2へ渡す音声フォーマット
		std::vector<uint8_t> pcmData;    // デコード済みPCMデータ
	};

	/// <summary>
	/// 現在再生しているボイス1件分の状態。
	/// </summary>
	struct PlayingVoice {
		IXAudio2SourceVoice* sourceVoice = nullptr;   // XAudio2の再生用ボイス
		std::shared_ptr<const SoundData> soundData;   // 再生中の音声データ
		bool paused = false;                          // 一時停止中か
	};

	// 音声ファイルの読み込みと内部状態の補助処理
	std::shared_ptr<SoundData> Decode(const std::filesystem::path& path) const;
	std::filesystem::path ResolvePath(const std::filesystem::path& fileName) const;
	PlayingVoice* FindVoice(VoiceHandle voiceHandle);
	const PlayingVoice* FindVoice(VoiceHandle voiceHandle) const;
	void DestroyVoice(PlayingVoice& voice);
	void EnsureInitialized() const;

	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;                            // XAudio2本体
	IXAudio2MasteringVoice* masteringVoice_ = nullptr;                    // 最終出力用ボイス
	std::filesystem::path audioDirectory_;                                // 音声ファイルの基準フォルダ
	std::unordered_map<SoundHandle, std::shared_ptr<SoundData>> sounds_;   // 読み込み済み音声
	std::unordered_map<std::filesystem::path, SoundHandle> loadedPaths_;  // 重複読み込み防止用の検索表
	std::unordered_map<VoiceHandle, PlayingVoice> voices_;                // 再生中のボイス
	SoundHandle nextSoundHandle_ = 1;                                     // 次に発行する音声番号
	VoiceHandle nextVoiceHandle_ = 1;                                     // 次に発行する再生番号
	bool mediaFoundationStarted_ = false;                                 // Media Foundationの起動状態
	bool initialized_ = false;                                            // Audioの初期化状態
};
