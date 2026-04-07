#pragma once

#include <string>
#include <vector>

namespace zen {
namespace image {

// 图像格式
enum class image_format {
    png,
    jpeg,
    gif,
    bmp,
    tiff,
    webp
};

// 图像
class image {
public:
    image();
    image(int width, int height, int channels = 4);
    ~image();
    
    // 加载/保存
    static image load(const std::string& path);
    static image load(const uint8_t* data, size_t len);
    
    bool save(const std::string& path, image_format format = image_format::png, int quality = 90) const;
    std::vector<uint8_t> encode(image_format format = image_format::png, int quality = 90) const;
    
    // 信息
    int get_width() const { return width_; }
    int get_height() const { return height_; }
    int get_channels() const { return channels_; }
    size_t get_size() const { return width_ * height_ * channels_; }
    
    // 像素访问
    uint8_t* get_data() { return data_.get(); }
    const uint8_t* get_data() const { return data_.get(); }
    
    uint8_t& at(int x, int y, int channel = 0);
    const uint8_t& at(int x, int y, int channel = 0) const;
    
    // 调整大小
    image resize(int new_width, int new_height) const;
    void resize_in_place(int new_width, int new_height);
    
    // 裁剪
    image crop(int x, int y, int width, int height) const;
    
    // 旋转/翻转
    image rotate(double angle_degrees) const;
    image flip_horizontal() const;
    image flip_vertical() const;
    
    // 颜色转换
    image convert(int channels) const;
    image to_grayscale() const;
    image to_rgba() const;
    image to_rgb() const;
    
    // 滤镜
    image blur(int radius) const;
    image sharpen() const;
    image edge_detection() const;
    image emboss() const;
    
    // 亮度/对比度/饱和度
    image adjust_brightness(float factor) const;
    image adjust_contrast(float factor) const;
    image adjust_saturation(float factor) const;
    
    // 绘制
    void clear(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255);
    void draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    void draw_line(int x1, int y1, int x2, int y2,
                    uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255, int thickness = 1);
    void draw_rect(int x, int y, int width, int height,
                     uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255, int thickness = 1);
    void fill_rect(int x, int y, int width, int height,
                    uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    void draw_circle(int cx, int cy, int radius,
                      uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255, int thickness = 1);
    void fill_circle(int cx, int cy, int radius,
                      uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    
    // 水印
    void add_watermark(const image& watermark, int x, int y, float alpha = 0.5f);
    
    // 复制
    image clone() const;
    
    // 判断有效性
    bool is_valid() const { return data_ != nullptr; }
    
private:
    int width_;
    int height_;
    int channels_;
    std::unique_ptr<uint8_t[]> data_;
    
    void allocate();
};

// 图像工具
class image_utils {
public:
    // 生成占位图
    static image create_placeholder(int width, int height,
                                     const std::string& text = "",
                                     uint8_t bg_r = 200, uint8_t bg_g = 200, uint8_t bg_b = 200);
    
    // 图像比较
    static double compare(const image& img1, const image& img2);
    
    // 生成缩略图
    static image create_thumbnail(const image& img, int max_size);
    
    // 拼接
    static image hconcat(const std::vector<image>& images);
    static image vconcat(const std::vector<image>& images);
    
    // 拆分通道
    static std::vector<image> split_channels(const image& img);
    static image merge_channels(const std::vector<image>& channels);
};

} // namespace image
} // namespace zen
