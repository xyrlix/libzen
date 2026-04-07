#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace zen {
namespace graphics {

// 基础图形处理

// 颜色
struct color {
    uint8_t r, g, b, a;
    
    color() : r(0), g(0), b(0), a(255) {}
    color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
    
    static color from_rgba(uint32_t rgba);
    static color from_argb(uint32_t argb);
    static color from_rgb(uint32_t rgb);
    
    uint32_t to_rgba() const;
    uint32_t to_argb() const;
    uint32_t to_rgb() const;
    
    static const color black;
    static const color white;
    static const color red;
    static const color green;
    static const color blue;
    static const color yellow;
    static const color cyan;
    static const color magenta;
};

// 点
struct point {
    int x, y;
    
    point() : x(0), y(0) {}
    point(int x, int y) : x(x), y(y) {}
    
    double distance_to(const point& other) const;
    point offset(int dx, int dy) const { return {x + dx, y + dy}; }
};

// 大小
struct size {
    int width, height;
    
    size() : width(0), height(0) {}
    size(int w, int h) : width(w), height(h) {}
    
    bool is_empty() const { return width <= 0 || height <= 0; }
    size scale(double factor) const { return {int(width * factor), int(height * factor)}; }
};

// 矩形
struct rect {
    int x, y, width, height;
    
    rect() : x(0), y(0), width(0), height(0) {}
    rect(int x, int y, int w, int h) : x(x), y(y), width(w), height(h) {}
    
    int left() const { return x; }
    int right() const { return x + width; }
    int top() const { return y; }
    int bottom() const { return y + height; }
    
    point center() const { return {x + width / 2, y + height / 2}; }
    
    bool is_empty() const { return width <= 0 || height <= 0; }
    bool contains(const point& pt) const;
    bool intersects(const rect& other) const;
    bool contains(const rect& other) const;
    
    rect intersection(const rect& other) const;
    rect union_(const rect& other) const;
    
    rect inflate(int dx, int dy) const;
    rect offset(int dx, int dy) const { return {x + dx, y + dy, width, height}; }
    
    static rect from_ltrb(int left, int top, int right, int bottom);
};

// 图形上下文（2D 绘图）
class graphics_context {
public:
    graphics_context();
    virtual ~graphics_context();
    
    // 清除
    void clear(color c = color::black);
    
    // 绘制形状
    void draw_line(const point& p1, const point& p2, color c, int thickness = 1);
    void draw_rect(const rect& r, color c, int thickness = 1);
    void fill_rect(const rect& r, color c);
    void draw_circle(const point& center, int radius, color c, int thickness = 1);
    void fill_circle(const point& center, int radius, color c);
    void draw_ellipse(const rect& r, color c, int thickness = 1);
    void fill_ellipse(const rect& r, color c);
    void draw_rounded_rect(const rect& r, int radius, color c, int thickness = 1);
    void fill_rounded_rect(const rect& r, int radius, color c);
    
    // 绘制文本
    void draw_text(const std::string& text, const point& pos, color c,
                     const std::string& font = "", int size = 12);
    size measure_text(const std::string& text, const std::string& font = "", int size = 12);
    
    // 绘制图像
    void draw_image(const image::image& img, const point& pos);
    void draw_image(const image::image& img, const rect& src, const rect& dst);
    
    // 变换
    void push_transform();
    void pop_transform();
    void translate(int dx, int dy);
    void rotate(double angle_degrees);
    void scale(double sx, double sy);
    
    // 裁剪
    void set_clip_rect(const rect& r);
    void reset_clip();
    
    // 混合模式
    void set_composite_mode(composite_mode mode);
    
    // 抗锯齿
    void set_antialias(bool enable);
    
    // 获取画布
    image::image get_canvas() const;
    
protected:
    std::unique_ptr<image::image> canvas_;
    
    enum class composite_mode {
        source_over,
        source_in,
        source_out,
        source_atop,
        destination_over,
        destination_in,
        destination_out,
        destination_atop,
        lighter,
        copy,
        xor
    };
    
    composite_mode composite_mode_;
    bool antialias_;
    
    std::vector<std::function<void()>> transform_stack_;
};

// 字体管理器
class font_manager {
public:
    font_manager();
    ~font_manager();
    
    // 加载字体
    bool load_font(const std::string& name, const std::string& path);
    bool load_font_from_memory(const std::string& name, const uint8_t* data, size_t len);
    
    // 获取字体
    bool has_font(const std::string& name) const;
    std::vector<std::string> get_fonts() const;
    
    // 系统字体
    std::vector<std::string> get_system_fonts() const;
    
    // 文本度量
    size measure_text(const std::string& text, const std::string& font, int size);
    
private:
    class impl;
    std::unique_ptr<impl> impl_;
};

// 图形路径
class graphics_path {
public:
    graphics_path();
    ~graphics_path();
    
    void move_to(const point& p);
    void line_to(const point& p);
    void curve_to(const point& cp1, const point& cp2, const point& end);
    void quadratic_to(const point& cp, const point& end);
    void arc_to(const point& center, int radius, double start_angle, double end_angle);
    void close_path();
    
    rect get_bounds() const;
    bool contains(const point& pt) const;
    
    void stroke(graphics_context& ctx, color c, int thickness = 1);
    void fill(graphics_context& ctx, color c);
    
private:
    class impl;
    std::unique_ptr<impl> impl_;
};

// 渐变
class gradient {
public:
    enum class type {
        linear,
        radial
    };
    
    gradient(type t);
    ~gradient();
    
    void add_stop(double offset, color c);
    
    void set_start_point(const point& p);
    void set_end_point(const point& p);
    void set_center_point(const point& p);
    void set_radius(double radius);
    
private:
    class impl;
    std::unique_ptr<impl> impl_;
    
    friend class graphics_context;
};

// 动画
class animation {
public:
    using frame_callback = std::function<void(double progress)>;
    using finish_callback = std::function<void()>;
    
    animation(double duration_ms, frame_callback on_frame);
    ~animation();
    
    void start();
    void stop();
    void pause();
    void resume();
    void reset();
    
    bool is_running() const;
    bool is_paused() const;
    
    void set_finish_callback(finish_callback on_finish);
    
    // 缓动函数
    static double ease_linear(double t);
    static double ease_in_quad(double t);
    static double ease_out_quad(double t);
    static double ease_in_out_quad(double t);
    static double ease_in_cubic(double t);
    static double ease_out_cubic(double t);
    static double ease_in_out_cubic(double t);
    
private:
    class impl;
    std::unique_ptr<impl> impl_;
};

// 图形工具
class graphics_utils {
public:
    // 几何计算
    static double distance(const point& p1, const point& p2);
    static point rotate_point(const point& pt, const point& center, double angle_degrees);
    static rect scale_rect(const rect& r, double sx, double sy);
    
    // 颜色转换
    static color hsv_to_rgb(double h, double s, double v);
    static void rgb_to_hsv(color c, double& h, double& s, double& v);
    static color blend_colors(color c1, color c2, double alpha);
    
    // 形状生成
    static graphics_path create_rounded_rect_path(const rect& r, int radius);
    static graphics_path create_star_path(const point& center, int num_points, 
                                             int outer_radius, int inner_radius);
    static graphics_path create_polygon_path(const std::vector<point>& points);
};

} // namespace graphics
} // namespace zen
