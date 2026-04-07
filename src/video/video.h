#pragma once

#include <string>
#include <vector>
#include <functional>

namespace zen {
namespace video {

// 视频格式
enum class video_format {
    mp4,
    avi,
    mkv,
    mov,
    flv,
    webm
};

// 视频编码格式
enum class video_codec {
    h264,
    h265,
    vp8,
    vp9,
    av1,
    mpeg4
};

// 音频编码格式
enum class audio_codec {
    aac,
    mp3,
    opus,
    vorbis
};

// 视频信息
struct video_info {
    int width;
    int height;
    double frame_rate;
    int64_t duration_ms;
    int64_t total_frames;
    video_codec video_codec;
    audio_codec audio_codec;
    int video_bitrate;
    int audio_bitrate;
};

// 视频帧
class video_frame {
public:
    video_frame();
    video_frame(int width, int height, int channels = 3);
    ~video_frame();
    
    // 信息
    int get_width() const { return width_; }
    int get_height() const { return height_; }
    int get_channels() const { return channels_; }
    int64_t get_timestamp_ms() const { return timestamp_ms_; }
    
    // 数据访问
    uint8_t* get_data() { return data_.get(); }
    const uint8_t* get_data() const { return data_.get(); }
    size_t get_size() const;
    
    // 转换为图像
    image::image to_image() const;
    static video_frame from_image(const image::image& img);
    
private:
    int width_;
    int height_;
    int channels_;
    int64_t timestamp_ms_;
    std::unique_ptr<uint8_t[]> data_;
};

// 视频解码器
class video_decoder {
public:
    using frame_callback = std::function<bool(const video_frame& frame)>;
    
    video_decoder();
    ~video_decoder();
    
    // 打开
    bool open(const std::string& path);
    bool open(const uint8_t* data, size_t len);
    void close();
    
    // 信息
    video_info get_info() const;
    bool is_open() const;
    
    // 逐帧解码
    bool seek(int64_t timestamp_ms);
    video_frame decode_frame();
    std::vector<video_frame> decode_all(frame_callback callback = nullptr);
    
    // 音频
    audio::audio_data extract_audio();
    
private:
    class impl;
    std::unique_ptr<impl> impl_;
};

// 视频编码器
class video_encoder {
public:
    video_encoder();
    ~video_encoder();
    
    // 配置
    void set_video_codec(video_codec codec);
    void set_audio_codec(audio_codec codec);
    void set_resolution(int width, int height);
    void set_frame_rate(double fps);
    void set_bitrate(int video_bitrate, int audio_bitrate);
    
    // 打开
    bool open(const std::string& path, video_info info);
    void close();
    
    // 写入帧
    bool write_frame(const video_frame& frame);
    bool write_audio(const audio::audio_data& audio);
    
private:
    class impl;
    std::unique_ptr<impl> impl_;
};

// 视频处理
class video_processor {
public:
    // 调整大小
    static bool resize(const std::string& input_path, const std::string& output_path,
                        int width, int height);
    
    // 裁剪
    static bool crop(const std::string& input_path, const std::string& output_path,
                      int x, int y, int width, int height);
    
    // 剪辑
    static bool trim(const std::string& input_path, const std::string& output_path,
                      int64_t start_ms, int64_t end_ms);
    
    // 添加水印
    static bool add_watermark(const std::string& input_path, const std::string& output_path,
                                const std::string& watermark_image_path,
                                int x, int y, float alpha = 0.5f);
    
    // 转码
    static bool transcode(const std::string& input_path, const std::string& output_path,
                           video_codec video_codec, audio_codec audio_codec,
                           int video_bitrate, int audio_bitrate);
    
    // 提取帧
    static std::vector<image::image> extract_frames(const std::string& path,
                                                       int64_t start_ms, int64_t end_ms,
                                                       double fps = 1.0);
    
    // 提取音频
    static bool extract_audio(const std::string& input_path, const std::string& output_audio_path);
    
    // 视频分析
    static video_info get_info(const std::string& path);
    static std::vector<image::image> generate_thumbnails(const std::string& path,
                                                          int num_thumbnails);
};

// 视频播放器
class video_player {
public:
    using frame_callback = std::function<void(const video_frame& frame)>;
    using finish_callback = std::function<void()>;
    
    video_player();
    ~video_player();
    
    // 加载
    bool load(const std::string& path);
    
    // 播放控制
    bool play();
    void pause();
    void resume();
    void stop();
    
    // 状态
    bool is_playing() const;
    double get_current_time() const;
    double get_duration() const;
    void seek(double time);
    
    // 音量
    void set_volume(float volume);
    float get_volume() const;
    
    // 回调
    void set_frame_callback(frame_callback cb);
    void set_finish_callback(finish_callback cb);
    
private:
    class impl;
    std::unique_ptr<impl> impl_;
};

} // namespace video
} // namespace zen
