#pragma once

#include <string>
#include <vector>
#include <functional>

namespace zen {
namespace audio {

// 音频格式
enum class audio_format {
    wav,
    mp3,
    ogg,
    flac,
    aac
};

// 采样格式
enum class sample_format {
    u8,      // 8-bit unsigned
    s16,     // 16-bit signed
    s32,     // 32-bit signed
    float32  // 32-bit float
};

// 音频信息
struct audio_info {
    int sample_rate;
    int channels;
    sample_format format;
    int bit_depth;
    size_t frames;
    double duration;
};

// 音频数据
class audio_data {
public:
    audio_data();
    audio_data(int sample_rate, int channels, sample_format format);
    ~audio_data();
    
    // 信息
    audio_info get_info() const;
    
    // 数据访问
    void* get_data() { return data_.get(); }
    const void* get_data() const { return data_.get(); }
    size_t get_size() const { return size_; }
    size_t get_frame_count() const { return frame_count_; }
    
    // 预分配
    void allocate(size_t frame_count);
    void resize(size_t frame_count);
    void clear();
    
    // 转换
    audio_data convert_format(sample_format target_format) const;
    audio_data resample(int new_sample_rate) const;
    audio_data mix_to_mono() const;
    audio_data convert_channels(int target_channels) const;
    
    // 处理
    void normalize(float target_db = -1.0f);
    void apply_gain(float db);
    void fade_in(double seconds);
    void fade_out(double seconds);
    void trim(double start, double end);
    
    // 拼接/裁剪
    audio_data concat(const audio_data& other) const;
    audio_data slice(size_t start_frame, size_t end_frame) const;
    
private:
    int sample_rate_;
    int channels_;
    sample_format format_;
    std::unique_ptr<uint8_t[]> data_;
    size_t size_;
    size_t frame_count_;
};

// 音频解码器/编码器
class audio_codec {
public:
    // 解码
    static audio_data decode(const std::string& path);
    static audio_data decode(const std::vector<uint8_t>& data);
    
    // 编码
    static std::vector<uint8_t> encode(const audio_data& audio, audio_format format,
                                        int bitrate = 192);
    static bool save(const audio_data& audio, const std::string& path, audio_format format,
                      int bitrate = 192);
    
    // 获取信息
    static audio_info get_info(const std::string& path);
    
    // 支持的格式
    static std::vector<audio_format> get_supported_formats();
    static bool is_format_supported(audio_format format);
};

// 音频播放器
class audio_player {
public:
    using playback_callback = std::function<void(const audio_data&, double current_time)>;
    using finish_callback = std::function<void()>;
    
    audio_player();
    ~audio_player();
    
    // 加载
    bool load(const std::string& path);
    bool load(const audio_data& audio);
    
    // 播放控制
    bool play();
    void pause();
    void resume();
    void stop();
    
    // 状态
    bool is_playing() const;
    bool is_paused() const;
    double get_current_time() const;
    double get_duration() const;
    void seek(double time);
    
    // 音量
    void set_volume(float volume);  // 0.0 - 1.0
    float get_volume() const;
    
    // 循环
    void set_loop(bool loop);
    bool get_loop() const;
    
    // 回调
    void set_playback_callback(playback_callback cb);
    void set_finish_callback(finish_callback cb);
    
private:
    class impl;
    std::unique_ptr<impl> impl_;
};

// 音频录制器
class audio_recorder {
public:
    using data_callback = std::function<void(const audio_data&)>;
    
    audio_recorder();
    ~audio_recorder();
    
    // 配置
    void set_format(int sample_rate, int channels, sample_format format);
    audio_info get_format() const;
    
    // 录制控制
    bool start();
    void stop();
    bool is_recording() const;
    
    // 数据回调
    void set_data_callback(data_callback cb);
    
    // 获取数据
    audio_data get_recorded_data() const;
    void clear_data();
    
    // 保存
    bool save(const std::string& path, audio_format format);
    
private:
    class impl;
    std::unique_ptr<impl> impl_;
};

// 音频工具
class audio_utils {
public:
    // 音频分析
    static double calculate_rms(const audio_data& audio);
    static double calculate_peak(const audio_data& audio);
    static std::vector<double> calculate_fft(const audio_data& audio);
    
    // 音频合成
    static audio_data generate_sine_wave(double frequency, double duration,
                                            int sample_rate = 44100);
    static audio_data generate_noise(double duration, int sample_rate = 44100);
    static audio_data generate_silence(double duration, int sample_rate = 44100);
    
    // 音频混合
    static audio_data mix(const std::vector<audio_data>& tracks);
};

} // namespace audio
} // namespace zen
