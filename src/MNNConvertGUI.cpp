#include "MNNConvertGUI.h"
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <sys/stat.h>
#include <cmath>
#include <vector>
#include <filesystem>
#include <SDL2/SDL2_gfxPrimitives.h>

/**
 * @brief 构造函数
 *        初始化所有成员变量
 */
MNNConvertGUI::MNNConvertGUI() 
    : window_(nullptr)
    , renderer_(nullptr)
    , font_(nullptr)
    , running_(false)
    , window_width_(800)
    , window_height_(600)
    , input_file_path_("")
    , output_file_path_("")
    , biz_code_("biz")
    , input_file_selected_(false)
    , output_file_selected_(false)
    , active_input_field_(0)
    , color_background_{255, 255, 255, 255}
    , color_button_{70, 130, 180, 255}
    , color_button_hover_{100, 149, 237, 255}
    , color_text_{255, 255, 255, 255}
    , color_input_bg_{255, 255, 255, 255}
    , color_success_{34, 139, 34, 255}
    , color_error_{220, 20, 60, 255}
    , status_message_("")
    , show_status_(false)
    , status_start_time_(0)
    , is_converting_(false)
    , conversion_success_(false)
    , conversion_result_message_("")
    , is_dragging_over_(false)
    , conversion_completed_time_(0)
    , show_conversion_success_(false)
    , waiting_for_filename_input_(false)
    , pending_input_file_path_("")
    , show_filename_dialog_(false)
    , dialog_filename_input_("")
    , dialog_rect_{0, 0, 0, 0}
    , dialog_input_rect_{0, 0, 0, 0}
    , dialog_ok_button_{0, 0, 0, 0}
    , dialog_cancel_button_{0, 0, 0, 0}
    , dialog_input_active_(false)
    , mouse_x_(0)
    , mouse_y_(0)
    , config_file_path_("config/default.yaml") {
    
    // 加载配置文件
    LoadConfiguration();
    
    // 极简界面 - 只保留拖拽区域，不需要设置其他控件位置
}

/**
 * @brief 析构函数
 *        清理资源
 */
MNNConvertGUI::~MNNConvertGUI() {
    CleanupConversionThread();
    Cleanup();
}

/**
 * @brief 初始化SDL系统和创建窗口
 *        
 * @param width 窗口宽度
 * @param height 窗口高度
 */
bool MNNConvertGUI::Initialize(int width, int height) {
    window_width_ = width;
    window_height_ = height;

    // 初始化SDL - 包含视频和事件子系统
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        std::cerr << "[ERROR] SDL初始化失败: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // 检查SDL版本信息
    SDL_version compiled;
    SDL_version linked;
    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);

    // 初始化SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "[ERROR] SDL_ttf初始化失败: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    // 创建窗口 - 添加输入焦点标志
    window_ = SDL_CreateWindow(
        "MNN模型转换工具 - 支持拖拽转换",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        window_width_,
        window_height_,
        SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS
    );

    if (!window_) {
        std::cerr << "[ERROR] 窗口创建失败: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return false;
    }
    
    // 启用文件拖拽支持
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    
    // 确保窗口获得焦点
    SDL_RaiseWindow(window_);

    // 创建渲染器
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_) {
        std::cerr << "[ERROR] 渲染器创建失败: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window_);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    // 加载支持中文的字体
    
    // 字体路径列表（按优先级排序）：JetBrains Mono → HarmonyOS Sans → WQY 微米黑 → 其余中文与常见后备
    const char* chinese_fonts[] = {
        "/usr/local/share/fonts/SmileySans-Oblique.ttf",
        "/usr/local/share/fonts/HarmonyOS_Sans_Regular.ttf",
        "/usr/local/share/fonts/JetBrainsMonoNL-ExtraBoldItalic.ttf",
        "/usr/local/share/fonts/JetBrainsMonoNL-SemiBoldItalic.ttf",
        "/usr/local/share/fonts/JetBrainsMono-Regular.ttf",
        "/usr/local/share/fonts/HarmonyOS_Sans_Regular.ttf",
        "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",
        nullptr
    };
    
    font_ = nullptr;
    for (int i = 0; chinese_fonts[i] != nullptr; i++) {
        font_ = TTF_OpenFont(chinese_fonts[i], 18);
        if (font_) {
            break;
        }
    }
    
    if (!font_) {
        std::cerr << "[ERROR] 无法加载任何支持中文的字体！" << std::endl;
        std::cerr << "[INFO] 请安装中文字体包：" << std::endl;
        std::cerr << "       sudo apt install fonts-wqy-microhei fonts-wqy-zenhei" << std::endl;
        std::cerr << "       或者: sudo apt install fonts-arphic-uming fonts-arphic-ukai" << std::endl;
    }

    // 初始化事件过滤和状态
    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
    return true;
}

/**
 * @brief 主运行循环
 */
void MNNConvertGUI::Run() {
    running_ = true;

    while (running_) {
        HandleEvents();
        Update();
        Render();
        SDL_Delay(16); // 约60FPS
    }
}

/**
 * @brief 清理资源
 */
void MNNConvertGUI::Cleanup() {
    if (font_) {
        TTF_CloseFont(font_);
        font_ = nullptr;
    }
    
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    
    TTF_Quit();
    SDL_Quit();
}

/**
 * @brief 处理SDL事件
 */
void MNNConvertGUI::HandleEvents() {
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                running_ = false;
                break;
            
            case SDL_WINDOWEVENT:
                break;
                
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    HandleMouseClick(event.button.x, event.button.y);
                }
                break;
                
            case SDL_MOUSEBUTTONUP:
                break;
                
            case SDL_MOUSEMOTION:
                mouse_x_ = event.motion.x;
                mouse_y_ = event.motion.y;
                break;
                
            case SDL_KEYDOWN:
                HandleKeyInput(event.key.keysym.sym, "");
                break;
                
            case SDL_TEXTINPUT:
                HandleKeyInput(SDLK_UNKNOWN, event.text.text);
                break;
                
            case SDL_DROPFILE:
                HandleFileDrop(event.drop.file);
                SDL_free(event.drop.file);
                break;
                
            default:
                break;
        }
    }
}

/**
 * @brief 更新应用程序状态
 */
void MNNConvertGUI::Update() {
    // 检查状态消息是否需要隐藏
    if (show_status_ && SDL_GetTicks() - status_start_time_ > 3000) {
        show_status_ = false;
    }
    
    // 检查转换完成后的绿色显示是否需要隐藏（2秒后）
    if (show_conversion_success_ && SDL_GetTicks() - conversion_completed_time_ > 2000) {
        show_conversion_success_ = false;
    }
    
    // 检查转换结果
    CheckConversionResult();

    // 更新拖拽区域缩放动画
    UpdateDropZoneAnimation();
}

/**
 * @brief 渲染界面
 */
void MNNConvertGUI::Render() {
    // 清空屏幕 - 白色背景
    SDL_SetRenderDrawColor(renderer_, 
        color_background_.r, color_background_.g, color_background_.b, color_background_.a);
    SDL_RenderClear(renderer_);

    // 极简界面：只渲染圆角拖拽区域
    RenderDropZone();

    // 在拖拽区域下方显示简洁的状态信息
    RenderFileStatusInfo();
    
    // 如果显示文件名对话框，渲染对话框
    if (show_filename_dialog_) {
        RenderFilenameDialog();
    }

    SDL_RenderPresent(renderer_);
}

/**
 * @brief 渲染按钮
 * @param rect 按钮矩形区域
 * @param text 按钮文本
 * @param is_hovered 是否鼠标悬停
 */
void MNNConvertGUI::RenderButton(const SDL_Rect& rect, const std::string& text, bool is_hovered, int font_size) {
    (void)font_size; // 避免未使用参数警告
    SDL_Color button_color = is_hovered ? color_button_hover_ : color_button_;
    SDL_Color text_color = {0, 0, 0, 255};
    SDL_Color text_bg_color = {0, 0, 0, 255};

    // "确定"按钮：绿色背景，白色文字
    if (text == "确定") {
        button_color = is_hovered ? SDL_Color{60, 180, 90, 255} : SDL_Color{34, 139, 34, 255};
        text_color = SDL_Color{255, 255, 255, 255};
        text_bg_color = button_color;
    }
    else if (text == "取消") {
        button_color = is_hovered ? SDL_Color{255, 0, 0, 255} : SDL_Color{255, 0, 0, 255};
        text_color = SDL_Color{255, 255, 255, 255};
        text_bg_color = button_color;
    }

    // 使用圆角按钮
    int corner_radius = 8;
    RenderRoundedRect(rect, corner_radius, button_color, true);

    // 对话框按钮（确定/取消）不绘制边框，其余按钮保留细边
    if (!(text == "确定" || text == "取消")) {
        SDL_Color border_color = {0, 0, 0, 255};
        RenderRoundedRect(rect, corner_radius, border_color, false);
    }

    // 文字/符号居中：将“确定”显示为“○”，“取消”显示为“×”
    if (font_ && !text.empty()) {
        std::string display_text = text;
        if (text == "确定") {
            display_text = "○";
        } else if (text == "取消") {
            display_text = "×";
        }

        SDL_Surface* text_surface = TTF_RenderUTF8_Blended(font_, display_text.c_str(), text_color);
        if (text_surface) {
            int text_w = text_surface->w;
            int text_h = text_surface->h;
            int text_x = rect.x + (rect.w - text_w) / 2;
            int text_y = rect.y + (rect.h - text_h) / 2;
            SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer_, text_surface);
            if (text_texture) {
                SDL_Rect dst_rect = {text_x, text_y, text_w, text_h};
                SDL_RenderCopy(renderer_, text_texture, nullptr, &dst_rect);
                SDL_DestroyTexture(text_texture);
            }
            SDL_FreeSurface(text_surface);
        }
    }
}

/**
 * @brief 渲染文本
 * @param text 要渲染的文本
 * @param x X坐标
 * @param y Y坐标
 * @param color 文本颜色
 * @param bg_color 文字背景颜色
 */
void MNNConvertGUI::RenderText(const std::string& text, int x, int y, SDL_Color color, SDL_Color bg_color) {
    if (!font_ || text.empty()) return;
    
    // 使用UTF-8渲染，支持中文，使用Blended模式以获得最佳抗锯齿
    SDL_Surface* text_surface = TTF_RenderUTF8_Blended(font_, text.c_str(), color);
    if (!text_surface) {
        std::cerr << "[WARNING] 文本渲染失败: " << TTF_GetError() << " 文本: " << text << std::endl;
        return;
    }
    
    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer_, text_surface);
    if (!text_texture) {
        std::cerr << "[WARNING] 纹理创建失败: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(text_surface);
        return;
    }
    // 使透明度渐变生效
    SDL_SetTextureBlendMode(text_texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(text_texture, color.a);
    
    SDL_Rect dest_rect = {x, y, text_surface->w, text_surface->h};
    SDL_RenderCopy(renderer_, text_texture, nullptr, &dest_rect);
    
    SDL_DestroyTexture(text_texture);
    SDL_FreeSurface(text_surface);
}

/**
 * @brief 渲染居中文本
 * @param text 要渲染的文本
 * @param center_x 中心X坐标
 * @param y Y坐标
 * @param color 文本颜色
 * @param bg_color 文字背景颜色
 */
void MNNConvertGUI::RenderCenteredText(const std::string& text, int center_x, int y, SDL_Color color, SDL_Color bg_color) {
    if (!font_ || text.empty()) return;
    
    // 使用UTF-8渲染，支持中文，使用Blended模式以获得最佳抗锯齿
    SDL_Surface* text_surface = TTF_RenderUTF8_Blended(font_, text.c_str(), color);
    if (!text_surface) {
        std::cerr << "[WARNING] 文本渲染失败: " << TTF_GetError() << " 文本: " << text << std::endl;
        return;
    }
    
    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer_, text_surface);
    if (!text_texture) {
        std::cerr << "[WARNING] 纹理创建失败: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(text_surface);
        return;
    }
    // 使透明度渐变生效
    SDL_SetTextureBlendMode(text_texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(text_texture, color.a);
    
    // 计算居中位置：中心X坐标减去文本宽度的一半
    int x = center_x - text_surface->w / 2;
    SDL_Rect dest_rect = {x, y, text_surface->w, text_surface->h};
    SDL_RenderCopy(renderer_, text_texture, nullptr, &dest_rect);
    
    SDL_DestroyTexture(text_texture);
    SDL_FreeSurface(text_surface);
}

/**
 * @brief 渲染输入框
 * @param rect 输入框矩形区域
 * @param text 输入框文本
 * @param is_active 是否为活跃状态
 */
void MNNConvertGUI::RenderInputField(const SDL_Rect& rect, const std::string& text, bool is_active) {
    // 绘制输入框背景（圆角）
    int radius = std::min(rect.h / 2, 10);
    RenderRoundedRect(rect, radius, color_input_bg_, true);
    
    // 不绘制边框线条，保持纯色圆角背景
    
    // 渲染输入文本 - 使用黑色文字以便在白色背景上可见
    if (!text.empty()) {
        SDL_Color text_color = {0, 0, 0, 255}; // 黑色文字
        // 计算文本高度，使其在输入框内垂直居中
        int text_w = 0;
        int text_h = 0;
        if (font_) {
            if (TTF_SizeUTF8(font_, text.c_str(), &text_w, &text_h) != 0) {
                text_h = 0;
            }
        }
        int text_y = rect.y + (rect.h - text_h) / 2;
        RenderText(text, rect.x + 5, text_y, text_color);
    }

    // 绘制插入光标（闪烁），仅当输入框激活时
    if (is_active && font_) {
        // 每500ms闪烁一次
        Uint32 ticks = SDL_GetTicks();
        bool show_caret = ((ticks / 500) % 2) == 0;
        if (show_caret) {
            int text_w = 0;
            int text_h = 0;
            if (!text.empty()) {
                // 通过TTF计算当前文本宽度，确定光标位置
                if (TTF_SizeUTF8(font_, text.c_str(), &text_w, &text_h) != 0) {
                    text_w = 0;
                }
            }
            int caret_x = rect.x + 5 + text_w;
            int caret_top = rect.y + 6;
            int caret_bottom = rect.y + rect.h - 6;
            aalineRGBA(renderer_, caret_x, caret_top, caret_x, caret_bottom, 0, 0, 0, 255);
        }
    }
}

/**
 * @brief 检查点是否在矩形内
 * @param x X坐标
 * @param y Y坐标  
 * @param rect 矩形区域
 */
bool MNNConvertGUI::IsPointInRect(int x, int y, const SDL_Rect& rect) {
    return x >= rect.x && x <= rect.x + rect.w && 
           y >= rect.y && y <= rect.y + rect.h;
}

/**
 * @brief 处理鼠标点击事件
 * @param x 鼠标X坐标
 * @param y 鼠标Y坐标
 */
void MNNConvertGUI::HandleMouseClick(int x, int y) {
    // 如果显示文件名对话框，处理对话框的鼠标点击
    if (show_filename_dialog_) {
        HandleDialogMouseClick(x, y);
        return;
    }
    
    active_input_field_ = 0; // 重置活跃输入框
    
    // 极简界面下只处理拖拽事件，不再需要按钮点击处理
    SDL_StopTextInput();
}

/**
 * @brief 处理键盘输入
 * @param key 按键代码
 * @param text_input 文本输入
 */
void MNNConvertGUI::HandleKeyInput(SDL_Keycode key, const std::string& text_input) {
    // 如果显示文件名对话框，处理对话框的键盘输入
    if (show_filename_dialog_) {
        HandleDialogKeyInput(key, text_input);
        return;
    }
    
    // 原有的biz_code输入处理
    if (active_input_field_ == 3 && !text_input.empty()) {
        // 处理biz_code输入
        if (text_input[0] >= 32 && text_input[0] <= 126) { // 可打印字符
            biz_code_ += text_input;
        }
    }
    
    if (key == SDLK_BACKSPACE && active_input_field_ == 3 && !biz_code_.empty()) {
        biz_code_.pop_back();
    }
    
    if (key == SDLK_ESCAPE) {
        active_input_field_ = 0;
        SDL_StopTextInput();
    }
}

/**
 * @brief 选择输入文件
 */
void MNNConvertGUI::SelectInputFile() {
    ShowStatus("[INFO] 请在终端中输入文件路径");
    
    std::string file_path = DirectFileInput("ONNX模型文件", "*.onnx", false);
    
    if (!file_path.empty()) {
        input_file_path_ = file_path;
        input_file_selected_ = true;
        
        // 使用配置文件中的输出目录自动生成输出文件路径
        if (output_file_path_.empty()) {
            output_file_path_ = GenerateOutputPath(input_file_path_);
        }
        
        ShowStatus("[OK] 输入文件选择成功");
    } else {
        ShowStatus("[INFO] 未选择文件");
    }
}

/**
 * @brief 选择输出文件
 */
void MNNConvertGUI::SelectOutputFile() {
    // 智能生成默认文件名
    std::string default_name = "output.mnn";
    if (!input_file_path_.empty()) {
        // 如果已选择输入文件，基于输入文件生成默认输出文件名
        std::string full_path = input_file_path_;
        size_t pos = full_path.find_last_of('.');
        if (pos != std::string::npos) {
            default_name = full_path.substr(0, pos) + ".mnn";
        } else {
            default_name = full_path + ".mnn";
        }
    }
    ShowStatus("[INFO] 请在终端中输入保存路径");
    
    std::string file_path = DirectFileInput("MNN模型保存", default_name, true);
    
    if (!file_path.empty()) {
        output_file_path_ = file_path;
        output_file_selected_ = true;
        ShowStatus("[OK] 输出文件路径设置成功");
    } else {
        ShowStatus("[INFO] 未设置输出文件");
    }
}

/**
 * @brief 执行模型转换
 */
void MNNConvertGUI::ExecuteConversion() {
    // 如果正在等待文件名输入，使用待处理的输入文件
    std::string current_input_file = waiting_for_filename_input_ ? pending_input_file_path_ : input_file_path_;
    
    if (current_input_file.empty()) {
        ShowStatus("[ERROR] 请先选择输入文件", true);
        return;
    }
    
    if (output_file_path_.empty()) {
        ShowStatus("[ERROR] 请设置输出文件路径", true);
        return;
    }
    
    if (biz_code_.empty()) {
        ShowStatus("[ERROR] 请输入业务代码", true);
        return;
    }
    
    // 生成完整的输出文件路径（添加.mnn扩展名）
    if (output_file_path_.find(".mnn") == std::string::npos) {
        output_file_path_ += ".mnn";
    }
    
    // 使用配置文件中的输出目录
    std::string output_dir = config_manager_.GetOutputDirectory();
    try {
        std::filesystem::create_directories(output_dir);
    } catch (const std::exception& e) {
        std::cerr << "[WARNING] 无法创建输出目录: " << output_dir << std::endl;
    }
    
    // 如果输出路径不是绝对路径，则添加到输出目录
    if (output_file_path_.find('/') == std::string::npos) {
        output_file_path_ = std::filesystem::path(output_dir) / output_file_path_;
    }
    
    // 确保之前的转换线程已经清理
    CleanupConversionThread();
    
    // 构建命令 - 使用配置文件中的MNNConvert路径
    std::string mnnconvert_path = config_manager_.GetMNNConvertPath();
    std::stringstream cmd;
    cmd << "\"" << mnnconvert_path << "\" -f ONNX --modelFile \"" << input_file_path_ 
        << "\" --MNNModel \"" << output_file_path_ << "\" --bizCode " << biz_code_;
    
    std::string command = cmd.str();
    
    ShowStatus("[INFO] 正在转换模型...");
    
    // 启动异步转换
    StartConversionThread(command);
}

/**
 * @brief 清空所有字段
 */
void MNNConvertGUI::ClearFields() {
    input_file_path_.clear();
    output_file_path_.clear();
    biz_code_ = config_manager_.GetBizCode();
    input_file_selected_ = false;
    output_file_selected_ = false;
    active_input_field_ = 0;
    SDL_StopTextInput();
    ShowStatus("[INFO] 已清空所有字段");
}

/**
 * @brief 显示状态消息
 * @param message 状态消息
 * @param is_error 是否为错误消息
 */
void MNNConvertGUI::ShowStatus(const std::string& message, bool is_error) {
    status_message_ = message;
    show_status_ = true;
    status_start_time_ = SDL_GetTicks();
    
    if (is_error) {
        std::cerr << message << std::endl;
    } else {
        std::cout << message << std::endl;
    }
}

/**
 * @brief 打开文件对话框（简化版本，使用zenity）
 * @param title 对话框标题
 * @param filter 文件过滤器
 */
std::string MNNConvertGUI::OpenFileDialog(const std::string& title, const std::string& filter) {
    
    std::string result = "";
    result = TryNativeFileDialog(title, filter, false);
    if (!result.empty()) {
        return result;
    }
    
    // 方案2: 使用改进的zenity（如果可用）
    result = TryZenityDialog(title, filter, false);
    if (!result.empty()) {
        return result;
    }
    
    // 方案3: 使用其他图形文件选择器
    result = TryAlternativeFileDialog(title, filter, false);
    if (!result.empty()) {
        return result;
    }
    
    ShowStatus("[INFO] 请在终端中输入文件路径");
    return FallbackOpenFileDialog();
}

/**
 * @brief 保存文件对话框（简化版本，使用zenity）
 * @param title 对话框标题
 * @param default_name 默认文件名
 */
std::string MNNConvertGUI::SaveFileDialog(const std::string& title, const std::string& default_name) {
    
    std::string result = "";
    result = TryNativeFileDialog(title, "", true);
    if (!result.empty()) {
        return result;
    }
    
    // 方案2: 使用改进的zenity（如果可用）
    result = TryZenityDialog(title, default_name, true);
    if (!result.empty()) {
        return result;
    }
    
    // 方案3: 使用其他图形文件选择器
    result = TryAlternativeFileDialog(title, "", true);
    if (!result.empty()) {
        return result;
    }
    
    // 方案4: 回退到终端输入
    ShowStatus("[INFO] 请在终端中输入保存路径");
    return FallbackSaveFileDialog(default_name);
}

/**
 * @brief 执行系统命令
 * @param command 要执行的命令
 */
bool MNNConvertGUI::ExecuteCommand(const std::string& command) {
    int result = system(command.c_str());
    return result == 0;
}

/**
 * @brief 获取文件扩展名
 * @param filename 文件名
 */
std::string MNNConvertGUI::GetFileExtension(const std::string& filename) {
    size_t pos = filename.find_last_of('.');
    if (pos != std::string::npos) {
        return filename.substr(pos);
    }
    return "";
}

/**
 * @brief 获取文件基本名称（不含路径）
 * @param filepath 文件路径
 */
std::string MNNConvertGUI::GetBaseName(const std::string& filepath) {
    size_t pos = filepath.find_last_of('/');
    if (pos != std::string::npos) {
        return filepath.substr(pos + 1);
    }
    return filepath;
}

/**
 * @brief 启动转换线程
 * @param command 要执行的命令
 */
void MNNConvertGUI::StartConversionThread(const std::string& command) {
    is_converting_ = true;
    conversion_thread_ = std::thread(&MNNConvertGUI::ConversionWorkerThread, this, command);
}

/**
 * @brief 转换工作线程
 * @param command 要执行的命令
 */
void MNNConvertGUI::ConversionWorkerThread(const std::string& command) {
    bool success = ExecuteCommand(command);
    
    // 使用锁保护共享数据
    std::lock_guard<std::mutex> lock(status_mutex_);
    conversion_success_ = success;
    if (success) {
        conversion_result_message_ = "[OK] 模型转换成功完成！";
    } else {
        conversion_result_message_ = "[ERROR] 模型转换失败，请检查输入文件和参数";
    }
    
    is_converting_ = false;
}

/**
 * @brief 检查转换结果
 */
void MNNConvertGUI::CheckConversionResult() {
    if (!is_converting_ && conversion_thread_.joinable()) {
        // 线程已完成，获取结果
        std::lock_guard<std::mutex> lock(status_mutex_);
        ShowStatus(conversion_result_message_, !conversion_success_);
        
        // 如果转换成功，显示绿色状态2秒
        if (conversion_success_) {
            show_conversion_success_ = true;
            conversion_completed_time_ = SDL_GetTicks();
            
            // 根据配置决定是否自动打开文件所在位置
            if (!output_file_path_.empty() && config_manager_.GetAutoOpenOutputDir()) {
                OpenFileLocation(output_file_path_);
            }
        }
        
        // 清理线程
        conversion_thread_.join();
    }
}

/**
 * @brief 清理转换线程
 */
void MNNConvertGUI::CleanupConversionThread() {
    if (conversion_thread_.joinable()) {
        conversion_thread_.join();
    }
    is_converting_ = false;
}

/**
 * @brief 备用文件打开对话框（终端输入）
 */
std::string MNNConvertGUI::FallbackOpenFileDialog() {
    std::string input_path;
    std::cout << "请输入ONNX文件路径: ";
    std::getline(std::cin, input_path);
    
    // 移除路径两端的引号和空格
    if (!input_path.empty()) {
        // 去除前后空格
        size_t start = input_path.find_first_not_of(" \t\r\n");
        size_t end = input_path.find_last_not_of(" \t\r\n");
        if (start != std::string::npos) {
            input_path = input_path.substr(start, end - start + 1);
        }
        
        // 去除引号
        if (input_path.length() >= 2) {
            if ((input_path.front() == '"' && input_path.back() == '"') ||
                (input_path.front() == '\'' && input_path.back() == '\'')) {
                input_path = input_path.substr(1, input_path.length() - 2);
            }
        }
        
        // 验证文件是否存在
        struct stat st;
        if (stat(input_path.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
            return input_path;
        } else {
            return "";
        }
    }
    
    return "";
}

/**
 * @brief 备用文件保存对话框（终端输入）
 */
std::string MNNConvertGUI::FallbackSaveFileDialog(const std::string& default_name) {
    std::string input_path;
    std::cout << "请输入MNN文件保存路径 [" << default_name << "]: ";
    std::getline(std::cin, input_path);
    
    // 如果输入为空，使用默认路径
    if (input_path.empty()) {
        input_path = default_name;
    } else {
        // 移除路径两端的引号和空格
        size_t start = input_path.find_first_not_of(" \t\r\n");
        size_t end = input_path.find_last_not_of(" \t\r\n");
        if (start != std::string::npos) {
            input_path = input_path.substr(start, end - start + 1);
        }
        
        // 去除引号
        if (input_path.length() >= 2) {
            if ((input_path.front() == '"' && input_path.back() == '"') ||
                (input_path.front() == '\'' && input_path.back() == '\'')) {
                input_path = input_path.substr(1, input_path.length() - 2);
            }
        }
    }
    
    // 检查目录是否存在
    size_t last_slash = input_path.find_last_of('/');
    if (last_slash != std::string::npos) {
        std::string dir_path = input_path.substr(0, last_slash);
        struct stat st;
        if (stat(dir_path.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
            return "";
        }
    }
    
    return input_path;
}

/**
 * @brief 渲染拖拽区域
 */
void MNNConvertGUI::RenderDropZone() {
    // 获取默认拖拽区域（若未初始化当前矩形则进行初始化）
    SDL_Rect default_rect = ComputeDefaultDropZoneRect();
    if (!drop_rect_initialized_) {
        drop_rect_current_ = default_rect;
        drop_rect_initialized_ = true;
    }
    SDL_Rect drop_zone = drop_rect_current_;
    
    // 选择颜色 - 根据不同状态显示不同颜色
    SDL_Color zone_color;
    SDL_Color border_color;
    GetDropZoneColors(zone_color, border_color);
    
    // 圆角半径
    int corner_radius = 20;
    
    // 绘制圆角背景
    RenderRoundedRect(drop_zone, corner_radius, zone_color, true);
    
    // 不再绘制边框线条
    
    // 拖拽区域保持完全空白，不显示任何文字
}

void MNNConvertGUI::GetDropZoneColors(SDL_Color& zone_color, SDL_Color& border_color) const {
    if (show_conversion_success_) {
        zone_color = {20, 80, 20, 255};
        border_color = {50, 200, 50, 255};
        return;
    }
    if (is_dragging_over_) {
        zone_color = {200, 200, 200, 255};
        border_color = {150, 150, 150, 255};
        return;
    }
    if (is_converting_) {
        zone_color = {60, 40, 10, 255};
        border_color = {200, 150, 50, 255};
        return;
    }
    if (!input_file_path_.empty()) {
        zone_color = {20, 40, 20, 255};
        border_color = {50, 150, 50, 255};
        return;
    }
    zone_color = {20, 40, 20, 255};
    border_color = {50, 150, 50, 255};
}

/**
 * @brief 渲染文件状态信息（在拖拽区域外面）
 */
void MNNConvertGUI::RenderFileStatusInfo() {
    // 极简界面下的状态信息 - 居中显示在拖拽区域下方
    int info_y = window_height_ - 90;
    int center_x = window_width_ / 2;
    
    // 优先显示转换状态提示信息
    if (show_conversion_success_) {
        // 文件转换完成后显示绿色提示2秒
        // Show conversion completed and filename (all in English)
        {
            std::string filename = GetBaseName(output_file_path_); // Assume output_file_path_ is the saved file path
            std::string display_name = filename.length() > 30 ? filename.substr(0, 27) + "..." : filename;
            RenderCenteredText("File: " + display_name + " converted successfully", center_x, info_y, {50, 255, 50, 255},{0, 0, 0, 255});
        }
    } else if (is_converting_) {
        // Show current file info - compact
        std::string filename = GetBaseName(input_file_path_);
        std::string display_name = filename.length() > 30 ? filename.substr(0, 27) + "..." : filename;
        // Show yellow progress message when converting
        RenderCenteredText("Converting file: " + display_name + ", please wait...", center_x, info_y, {255, 200, 50, 255}, {240, 240, 240, 255});
    } else if (input_file_path_.empty()) {
        // 弹出输入对话框时不再渲染该提示，避免一帧突变
        if (show_filename_dialog_) {
            return;
        }
        // 无文件时提示；缩小动画中淡出，放大动画中淡入
        Uint8 alpha = 255;
        if (animating_drop_shrink_ || animating_drop_expand_) {
            Uint32 now = SDL_GetTicks();
            Uint32 elapsed = now - anim_start_time_;
            float t = (anim_duration_ms_ == 0) ? 1.0f : std::min(1.0f, elapsed / static_cast<float>(anim_duration_ms_));
            float ease = (t < 0.5f) ? (2.0f * t * t) : (1.0f - ((-2.0f * t + 2.0f) * (-2.0f * t + 2.0f)) / 2.0f);
            float factor = animating_drop_shrink_ ? (1.0f - ease) : ease; // 缩小时 1->0, 放大时 0->1
            int a = static_cast<int>(255.0f * factor);
            if (a < 0) a = 0; if (a > 255) a = 255;
            alpha = static_cast<Uint8>(a);
        }
        SDL_Color fg = {0, 0, 0, alpha};
        SDL_Color bg = {255, 255, 255, 255};
        RenderCenteredText("Drag and drop an .onnx file onto the area above to convert automatically", center_x, info_y, fg, bg);
    } else {
        std::string filename = GetBaseName(output_file_path_); // Assume output_file_path_ is the saved file path
        std::string display_name = filename.length() > 30 ? filename.substr(0, 27) + "..." : filename;
        // Show current file info - compact English version
        RenderCenteredText(
            "File: " + display_name + " converted. Drag another .onnx file above to convert again.",
            center_x, info_y, {120, 120, 120, 255}, {240, 240, 240, 255}
        );
    }
}

/**
 * @brief 处理文件拖拽事件
 * @param file_path 拖拽的文件路径
 */
void MNNConvertGUI::HandleFileDrop(const char* file_path) {
    // 设置拖拽悬停状态，给用户视觉反馈
    is_dragging_over_ = true;
    
    std::string path_str(file_path);
    
    // 检查文件扩展名
    std::string extension = GetFileExtension(path_str);
    if (extension != ".onnx" && extension != ".ONNX") {
        is_dragging_over_ = false; // 重置拖拽状态
        ShowStatus("[ERROR] 仅支持 .onnx 格式文件", true);
        return;
    }
    
    // 检查文件是否存在
    struct stat st;
    if (stat(file_path, &st) != 0 || !S_ISREG(st.st_mode)) {
        is_dragging_over_ = false; // 重置拖拽状态
        ShowStatus("[ERROR] 文件无法访问", true);
        return;
    }
    
    // 保存待处理的文件路径
    pending_input_file_path_ = path_str;
    
    // 重置拖拽状态
    is_dragging_over_ = false;
    
    // 启动拖拽区域缩放动画，动画结束后再弹出文件名输入对话框
    BeginDropZoneShrinkAnimation();
}


/**
 * @brief 直接通过终端输入文件路径
 * @param purpose 输入目的说明
 * @param filter_or_default 文件过滤器或默认值
 * @param is_save 是否为保存操作
 * @return 选择的文件路径
 */
std::string MNNConvertGUI::DirectFileInput(const std::string& purpose, const std::string& filter_or_default, bool is_save) {
    (void)purpose; // 避免未使用参数警告
    
    std::cout << "路径: ";
    std::string input_path;
    std::getline(std::cin, input_path);
    
    // 处理用户输入
    if (input_path.empty() && is_save && !filter_or_default.empty()) {
        input_path = filter_or_default;
    }
    
    if (input_path.empty()) {
        return "";
    }
    
    // 去除首尾空格和引号
    while (!input_path.empty() && (input_path.front() == ' ' || input_path.front() == '\t')) {
        input_path.erase(0, 1);
    }
    while (!input_path.empty() && (input_path.back() == ' ' || input_path.back() == '\t')) {
        input_path.pop_back();
    }
    if (!input_path.empty() && (input_path.front() == '"' || input_path.front() == '\'')) {
        input_path.erase(0, 1);
    }
    if (!input_path.empty() && (input_path.back() == '"' || input_path.back() == '\'')) {
        input_path.pop_back();
    }
    
    if (!is_save) {
        // 输入文件 - 检查文件是否存在
        struct stat st;
        if (stat(input_path.c_str(), &st) != 0 || !S_ISREG(st.st_mode)) {
            return "";
        }
        
        // 检查文件扩展名
        // 检查文件扩展名（静默检查）
        GetFileExtension(input_path);
    } else {
        // 输出文件 - 检查目录是否存在
        size_t pos = input_path.find_last_of("/\\");
        if (pos != std::string::npos) {
            std::string dir_path = input_path.substr(0, pos);
            struct stat st;
            if (stat(dir_path.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
                return "";
            }
        }
    }
    
    return input_path;
}

/**
 * @brief 尝试使用系统原生文件对话框
 */
std::string MNNConvertGUI::TryNativeFileDialog(const std::string& title, const std::string& filter, bool is_save) {
    // 检测桌面环境并使用相应的原生文件对话框
    const char* desktop = getenv("XDG_CURRENT_DESKTOP");
    if (!desktop) desktop = getenv("DESKTOP_SESSION");
    
    if (desktop) {
        std::string desktop_env = desktop;
        
        // GNOME/Ubuntu 使用 nautilus
        if (desktop_env.find("GNOME") != std::string::npos || 
            desktop_env.find("Unity") != std::string::npos ||
            desktop_env.find("ubuntu") != std::string::npos) {
            return TryXdgFileDialog(title, filter, is_save);
        }
        
        // KDE 使用 kdialog
        if (desktop_env.find("KDE") != std::string::npos ||
            desktop_env.find("Plasma") != std::string::npos) {
            return TryKDialogFileDialog(title, filter, is_save);
        }
    }
    
    // 尝试使用 xdg-open 相关工具
    return TryXdgFileDialog(title, filter, is_save);
}

/**
 * @brief 尝试使用改进的zenity对话框
 */
std::string MNNConvertGUI::TryZenityDialog(const std::string& title, const std::string& filter, bool is_save) {
    // 检查zenity是否可用
    if (system("which zenity > /dev/null 2>&1") != 0) {
        return "";
    }
    
    std::string command;
    if (is_save) {
        command = "timeout 15 zenity --file-selection --save --title=\"" + title + "\"";
        // 如果有默认文件名，添加到命令中
        if (!filter.empty()) {
            command += " --filename=\"" + filter + "\"";
        }
    } else {
        command = "timeout 15 zenity --file-selection --title=\"" + title + "\" --file-filter=\"" + filter + "\"";
    }
    
    // 添加更多选项以改善用户体验
    command += " --width=800 --height=600";
    
    
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "";
    }
    
    char buffer[1024];
    std::string result = "";
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result = buffer;
        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }
    }
    
    int exit_code = pclose(pipe);
    if (exit_code == 0 && !result.empty()) {
        return result;
    }
    
    return "";
}

/**
 * @brief 尝试使用KDE的kdialog
 */
std::string MNNConvertGUI::TryKDialogFileDialog(const std::string& title, const std::string& filter, bool is_save) {
    if (system("which kdialog > /dev/null 2>&1") != 0) {
        return "";
    }
    
    std::string command;
    if (is_save) {
        command = "timeout 30 kdialog --getsavefilename . --title \"" + title + "\"";
    } else {
        // 转换filter格式 *.onnx -> *.onnx|ONNX files
        std::string kde_filter = filter + "|ONNX Files";
        command = "timeout 30 kdialog --getopenfilename . \"" + kde_filter + "\" --title \"" + title + "\"";
    }
    
    
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "";
    }
    
    char buffer[1024];
    std::string result = "";
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result = buffer;
        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }
    }
    
    int exit_code = pclose(pipe);
    if (exit_code == 0 && !result.empty()) {
        return result;
    }
    
    return "";
}

/**
 * @brief 尝试使用xdg标准文件对话框
 */
std::string MNNConvertGUI::TryXdgFileDialog(const std::string& title, const std::string& filter, bool is_save) {
    (void)title;  // 避免未使用参数警告
    (void)filter; // 避免未使用参数警告
    // 尝试直接启动文件管理器到特定目录
    std::string home_dir = getenv("HOME") ? getenv("HOME") : "/home";
    
    
    // 启动文件管理器（不等待）
    std::string fm_command = "nohup xdg-open \"" + home_dir + "\" > /dev/null 2>&1 &";
    system(fm_command.c_str());
    
    // 等待用户在终端输入路径
    ShowStatus("[INFO] 已启动文件管理器，请在终端输入路径");
    
    if (is_save) {
        return FallbackSaveFileDialog("output.mnn");
    } else {
        return FallbackOpenFileDialog();
    }
}

/**
 * @brief 尝试使用其他可用的文件选择器
 */
std::string MNNConvertGUI::TryAlternativeFileDialog(const std::string& title, const std::string& filter, bool is_save) {
    // 尝试其他可能的文件对话框工具
    std::vector<std::string> alternatives = {
        "yad",      // Yet Another Dialog
        "gtkdialog", // GTK Dialog
        "Xdialog"   // X Dialog
    };
    
    for (const auto& tool : alternatives) {
        if (system(("which " + tool + " > /dev/null 2>&1").c_str()) == 0) {
            
            if (tool == "yad") {
                return TryYadFileDialog(title, filter, is_save);
            }
            // 可以添加其他工具的实现
        }
    }
    
    return "";
}

/**
 * @brief 使用yad文件对话框
 */
std::string MNNConvertGUI::TryYadFileDialog(const std::string& title, const std::string& filter, bool is_save) {
    std::string command;
    if (is_save) {
        command = "timeout 30 yad --file-selection --save --title=\"" + title + "\" --width=800 --height=600";
    } else {
        command = "timeout 30 yad --file-selection --title=\"" + title + "\" --file-filter=\"" + filter + "\" --width=800 --height=600";
    }
    
    
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "";
    }
    
    char buffer[1024];
    std::string result = "";
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result = buffer;
        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }
    }
    
    int exit_code = pclose(pipe);
    if (exit_code == 0 && !result.empty()) {
        return result;
    }
    
    return "";
}

/**
 * @brief 渲染圆角矩形
 * @param rect 矩形区域
 * @param radius 圆角半径
 * @param color 颜色
 * @param filled 是否填充
 */
void MNNConvertGUI::RenderRoundedRect(const SDL_Rect& rect, int radius, SDL_Color color, bool filled) {
    if (!renderer_) return;
    
    int x1 = rect.x;
    int y1 = rect.y;
    int x2 = rect.x + rect.w - 1;
    int y2 = rect.y + rect.h - 1;
    Uint8 r = color.r, g = color.g, b = color.b, a = color.a;
    
    // 使用 SDL2_gfx 基础绘制，并用 AA 线条/圆弧进行边缘抗锯齿
    if (filled) {
        // 先用带圆角的填充盒子绘制主体
        roundedBoxRGBA(renderer_, x1, y1, x2, y2, radius, r, g, b, a);
        
        // 叠加抗锯齿轮廓线条与圆角边缘，以平滑边缘
        // 直边
        aalineRGBA(renderer_, x1 + radius, y1, x2 - radius, y1, r, g, b, a);
        aalineRGBA(renderer_, x1 + radius, y2, x2 - radius, y2, r, g, b, a);
        aalineRGBA(renderer_, x1, y1 + radius, x1, y2 - radius, r, g, b, a);
        aalineRGBA(renderer_, x2, y1 + radius, x2, y2 - radius, r, g, b, a);
        // 四个圆角（使用 AA 椭圆近似圆弧）
        aaellipseRGBA(renderer_, x1 + radius, y1 + radius, radius, radius, r, g, b, a);
        aaellipseRGBA(renderer_, x2 - radius, y1 + radius, radius, radius, r, g, b, a);
        aaellipseRGBA(renderer_, x1 + radius, y2 - radius, radius, radius, r, g, b, a);
        aaellipseRGBA(renderer_, x2 - radius, y2 - radius, radius, radius, r, g, b, a);
    } else {
        // 仅轮廓：先画非 AA 的轮廓，再用 AA 线覆盖平滑
        roundedRectangleRGBA(renderer_, x1, y1, x2, y2, radius, r, g, b, a);
        
        // 直边（AA）
        aalineRGBA(renderer_, x1 + radius, y1, x2 - radius, y1, r, g, b, a);
        aalineRGBA(renderer_, x1 + radius, y2, x2 - radius, y2, r, g, b, a);
        aalineRGBA(renderer_, x1, y1 + radius, x1, y2 - radius, r, g, b, a);
        aalineRGBA(renderer_, x2, y1 + radius, x2, y2 - radius, r, g, b, a);
        // 圆角（AA）
        aaellipseRGBA(renderer_, x1 + radius, y1 + radius, radius, radius, r, g, b, a);
        aaellipseRGBA(renderer_, x2 - radius, y1 + radius, radius, radius, r, g, b, a);
        aaellipseRGBA(renderer_, x1 + radius, y2 - radius, radius, radius, r, g, b, a);
        aaellipseRGBA(renderer_, x2 - radius, y2 - radius, radius, radius, r, g, b, a);
    }
}

/**
 * @brief 打开文件所在位置
 *        在文件管理器中打开并选中指定文件
 *
 * @param file_path 要定位的文件路径
 */
bool MNNConvertGUI::OpenFileLocation(const std::string& file_path) {
    if (file_path.empty()) {
        std::cerr << "[ERROR] 文件路径为空" << std::endl;
        return false;
    }
    
    // 检查文件是否存在
    struct stat st;
    if (stat(file_path.c_str(), &st) != 0) {
        std::cerr << "[ERROR] 文件不存在: " << file_path << std::endl;
        return false;
    }
    
    std::cout << "[INFO] 尝试打开文件位置: " << file_path << std::endl;
    
    // 获取文件所在目录
    std::string dir_path = file_path;
    size_t pos = dir_path.find_last_of('/');
    if (pos != std::string::npos) {
        dir_path = dir_path.substr(0, pos);
    } else {
        dir_path = ".";
    }
    
    // 检测桌面环境并使用相应的文件管理器
    const char* desktop = getenv("XDG_CURRENT_DESKTOP");
    if (!desktop) desktop = getenv("DESKTOP_SESSION");
    
    std::vector<std::string> commands_to_try;
    
    if (desktop) {
        std::string desktop_env = desktop;
        
        // GNOME/Ubuntu - 使用nautilus选择文件
        if (desktop_env.find("GNOME") != std::string::npos || 
            desktop_env.find("Unity") != std::string::npos ||
            desktop_env.find("ubuntu") != std::string::npos) {
            commands_to_try.push_back("nautilus --select \"" + file_path + "\"");
            commands_to_try.push_back("nautilus \"" + dir_path + "\"");
        }
        
        // KDE - 使用dolphin选择文件
        if (desktop_env.find("KDE") != std::string::npos ||
            desktop_env.find("Plasma") != std::string::npos) {
            commands_to_try.push_back("dolphin --select \"" + file_path + "\"");
            commands_to_try.push_back("dolphin \"" + dir_path + "\"");
        }
        
        // XFCE - 使用thunar
        if (desktop_env.find("XFCE") != std::string::npos) {
            commands_to_try.push_back("thunar \"" + dir_path + "\"");
        }
        
        // MATE - 使用caja
        if (desktop_env.find("MATE") != std::string::npos) {
            commands_to_try.push_back("caja --select \"" + file_path + "\"");
            commands_to_try.push_back("caja \"" + dir_path + "\"");
        }
    }
    
    // 通用方法 - 按优先级排序
    commands_to_try.push_back("xdg-open \"" + dir_path + "\"");
    commands_to_try.push_back("nautilus --select \"" + file_path + "\"");
    commands_to_try.push_back("nautilus \"" + dir_path + "\"");
    commands_to_try.push_back("dolphin --select \"" + file_path + "\"");
    commands_to_try.push_back("dolphin \"" + dir_path + "\"");
    commands_to_try.push_back("thunar \"" + dir_path + "\"");
    commands_to_try.push_back("pcmanfm \"" + dir_path + "\"");
    commands_to_try.push_back("caja \"" + dir_path + "\"");
    commands_to_try.push_back("nemo \"" + dir_path + "\"");
    
    // 尝试执行命令
    for (const auto& command : commands_to_try) {
        std::string full_command = command + " > /dev/null 2>&1 &";
        
        std::cout << "[INFO] 尝试命令: " << command << std::endl;
        
        // 检查命令是否可用（提取命令的第一个单词）
        std::string app_name = command.substr(0, command.find(' '));
        std::string check_command = "which " + app_name + " > /dev/null 2>&1";
        
        if (system(check_command.c_str()) == 0) {
            // 命令可用，尝试执行
            int result = system(full_command.c_str());
            if (result == 0) {
                std::cout << "[OK] 成功打开文件位置" << std::endl;
                return true;
            }
        }
    }
    
    // 如果所有方法都失败，至少显示目录路径
    std::cout << "[WARNING] 无法自动打开文件管理器" << std::endl;
    std::cout << "[INFO] 文件位置: " << file_path << std::endl;
    std::cout << "[INFO] 所在目录: " << dir_path << std::endl;
    
    return false;
}

/**
 * @brief 加载配置文件
 */
bool MNNConvertGUI::LoadConfiguration() {
    bool result = false;
    try {
        // 优先尝试当前设置的路径
        if (std::filesystem::exists(config_file_path_)) {
            result = config_manager_.LoadConfig(config_file_path_);
        } else {
            // 回退到上级目录（适配从 build/ 目录直接运行的场景）
            std::string alt_path = std::string("../") + config_file_path_;
            if (std::filesystem::exists(alt_path)) {
                config_file_path_ = alt_path;
                result = config_manager_.LoadConfig(config_file_path_);
            } else {
                // 最后尝试使用绝对路径（基于当前工作目录）
                std::filesystem::path abs_try = std::filesystem::current_path() / config_file_path_;
                if (std::filesystem::exists(abs_try)) {
                    config_file_path_ = abs_try.string();
                    result = config_manager_.LoadConfig(config_file_path_);
                } else {
                    // 均不存在则创建默认配置到原路径
                    result = config_manager_.LoadConfig(config_file_path_);
                }
            }
        }
    } catch (...) {
        result = config_manager_.LoadConfig(config_file_path_);
    }
    if (result) {
        ApplyConfigurationSettings();
        std::cout << "[OK] 配置文件加载完成" << std::endl;
        std::cout << "[INFO] 配置文件路径: " << config_file_path_ << std::endl;
        std::cout << "[INFO] 预填记录名: " << config_manager_.GetRecordFileName() << std::endl;
    } else {
        std::cout << "[WARNING] 配置文件加载失败，使用默认配置" << std::endl;
    }
    return result;
}

/**
 * @brief 保存配置文件
 */
bool MNNConvertGUI::SaveConfiguration() {
    return config_manager_.SaveConfig(config_file_path_);
}

/**
 * @brief 根据输入路径生成输出路径
 *        
 * @param input_path 输入文件路径
 */
std::string MNNConvertGUI::GenerateOutputPath(const std::string& input_path) {
    if (input_path.empty()) {
        return "";
    }
    
    // 获取输入文件的基础文件名（不包含路径和扩展名）
    std::filesystem::path input_file(input_path);
    std::string filename = input_file.stem().string(); // 不含扩展名的文件名
    
    // 获取配置中的输出目录
    std::string output_dir = config_manager_.GetOutputDirectory();
    
    // 确保输出目录存在
    try {
        std::filesystem::create_directories(output_dir);
    } catch (const std::exception& e) {
        std::cerr << "[WARNING] 无法创建输出目录: " << output_dir << std::endl;
        std::cerr << "[WARNING] 错误信息: " << e.what() << std::endl;
        // 如果无法创建配置目录，回退到输入文件所在目录
        output_dir = input_file.parent_path().string();
    }
    
    // 生成完整的输出文件路径
    std::filesystem::path output_path = std::filesystem::path(output_dir) / (filename + ".mnn");
    
    std::cout << "[INFO] 生成输出路径: " << output_path.string() << std::endl;
    return output_path.string();
}

/**
 * @brief 应用配置设置
 */
void MNNConvertGUI::ApplyConfigurationSettings() {
    // 设置默认的biz_code
    if (biz_code_.empty()) {
        biz_code_ = config_manager_.GetBizCode();
    }
    
    // 可以在这里添加其他配置项的应用逻辑
    std::cout << "[INFO] 应用配置设置完成" << std::endl;
    std::cout << "[INFO] MNNConvert程序路径: " << config_manager_.GetMNNConvertPath() << std::endl;
    std::cout << "[INFO] 输出目录: " << config_manager_.GetOutputDirectory() << std::endl;
    std::cout << "[INFO] 默认业务代码: " << config_manager_.GetBizCode() << std::endl;
    std::cout << "[INFO] 自动打开输出目录: " << (config_manager_.GetAutoOpenOutputDir() ? "是" : "否") << std::endl;
}

/**
 * @brief 显示文件名输入对话框
 */
void MNNConvertGUI::ShowFilenameDialog() {
    show_filename_dialog_ = true;
    // 预填充为配置里的 record_file_name
    dialog_filename_input_ = config_manager_.GetRecordFileName();
    dialog_input_active_ = true;
    
    // 计算对话框位置和大小
    int dialog_width = 400;
    int dialog_height = 200;
    dialog_rect_.x = (window_width_ - dialog_width) / 2;
    dialog_rect_.y = (window_height_ - dialog_height) / 2;
    dialog_rect_.w = dialog_width;
    dialog_rect_.h = dialog_height;
    
    // 输入框位置与大小：根据字体高度动态调整
    dialog_input_rect_.x = dialog_rect_.x + 20;
    dialog_input_rect_.y = dialog_rect_.y + 80;
    dialog_input_rect_.w = dialog_width - 40;
    int font_height = 0;
    if (font_) {
        // 使用字体高度（更稳定），无法获取时回退为 18
        font_height = TTF_FontHeight(font_);
        if (font_height <= 0) font_height = 18;
    } else {
        font_height = 18;
    }
    // 垂直内边距：使文字不贴边，随字体大小略增
    int vertical_padding = std::max(6, font_height / 3);
    dialog_input_rect_.h = font_height + vertical_padding * 2;
    
    // 按钮位置
    int button_width = 80;
    int button_height = 30;
    int button_spacing = 20;
    // 根据输入框高度稍微下移按钮，避免过大字体造成拥挤
    int button_y = dialog_rect_.y + dialog_height - 50 + (dialog_input_rect_.h - 30) / 2;
    
    dialog_ok_button_.x = dialog_rect_.x + (dialog_width - 2 * button_width - button_spacing) / 2;
    dialog_ok_button_.y = button_y;
    dialog_ok_button_.w = button_width;
    dialog_ok_button_.h = button_height;
    
    dialog_cancel_button_.x = dialog_ok_button_.x + button_width + button_spacing;
    dialog_cancel_button_.y = button_y;
    dialog_cancel_button_.w = button_width;
    dialog_cancel_button_.h = button_height;
    
    // 启动文本输入
    SDL_StartTextInput();
    
    // 启动对话框背景颜色渐变：从当前拖拽区域颜色过渡到对话框目标颜色
    {
        SDL_Color zone_c, border_c;
        GetDropZoneColors(zone_c, border_c);
        (void)border_c;
        dialog_bg_start_color_ = zone_c;
        dialog_bg_target_color_ = {220, 220, 220, 255};
        dialog_bg_anim_start_ = SDL_GetTicks();
        dialog_bg_animating_ = true;
    }
    
    ShowStatus(std::string("[INFO] 请输入输出文件名（不含扩展名），默认: ") + dialog_filename_input_);
}

SDL_Rect MNNConvertGUI::ComputeDefaultDropZoneRect() const {
    int margin = 80;
    int top_margin = 100;
    SDL_Rect r {margin, top_margin, window_width_ - 2 * margin, window_height_ - top_margin - 120};
    return r;
}

void MNNConvertGUI::BeginDropZoneShrinkAnimation() {
    drop_rect_start_ = ComputeDefaultDropZoneRect();
    // 目标为对话框的大小和位置
    int dialog_width = 400;
    int dialog_height = 200;
    drop_rect_target_.x = (window_width_ - dialog_width) / 2;
    drop_rect_target_.y = (window_height_ - dialog_height) / 2;
    drop_rect_target_.w = dialog_width;
    drop_rect_target_.h = dialog_height;
    drop_rect_current_ = drop_rect_start_;
    anim_start_time_ = SDL_GetTicks();
    animating_drop_shrink_ = true;
}

void MNNConvertGUI::BeginDropZoneExpandAnimation() {
    // 从对话框矩形恢复到默认拖拽区域
    int dialog_width = 400;
    int dialog_height = 200;
    SDL_Rect dialog_rect { (window_width_ - dialog_width) / 2,
                           (window_height_ - dialog_height) / 2,
                           dialog_width,
                           dialog_height };
    drop_rect_start_ = dialog_rect;
    drop_rect_target_ = ComputeDefaultDropZoneRect();
    drop_rect_current_ = drop_rect_start_;
    anim_start_time_ = SDL_GetTicks();
    animating_drop_expand_ = true;
}

void MNNConvertGUI::UpdateDropZoneAnimation() {
    if (!animating_drop_shrink_ && !animating_drop_expand_) return;
    Uint32 now = SDL_GetTicks();
    Uint32 elapsed = now - anim_start_time_;
    float t = (anim_duration_ms_ == 0) ? 1.0f : std::min(1.0f, elapsed / static_cast<float>(anim_duration_ms_));
    // 缓入缓出（ease-in-out，采用二次插值）
    float ease = (t < 0.5f)
        ? (2.0f * t * t)
        : (1.0f - ((-2.0f * t + 2.0f) * (-2.0f * t + 2.0f)) / 2.0f);
    auto lerp = [&](int a, int b) -> int { return a + static_cast<int>((b - a) * ease); };
    drop_rect_current_.x = lerp(drop_rect_start_.x, drop_rect_target_.x);
    drop_rect_current_.y = lerp(drop_rect_start_.y, drop_rect_target_.y);
    drop_rect_current_.w = lerp(drop_rect_start_.w, drop_rect_target_.w);
    drop_rect_current_.h = lerp(drop_rect_start_.h, drop_rect_target_.h);
    if (t >= 1.0f) {
        if (animating_drop_shrink_) {
            animating_drop_shrink_ = false;
            // 动画完成后弹出文件名输入对话框
            ShowFilenameDialog();
        } else if (animating_drop_expand_) {
            animating_drop_expand_ = false;
        }
    }
}

/**
 * @brief 隐藏文件名输入对话框
 */
void MNNConvertGUI::HideFilenameDialog() {
    show_filename_dialog_ = false;
    dialog_input_active_ = false;
    SDL_StopTextInput();
    // 对话框关闭后，拖拽区域从对话框大小恢复到默认大小
    BeginDropZoneExpandAnimation();
}

/**
 * @brief 确认文件名输入
 */
void MNNConvertGUI::ConfirmFilenameInput() {
    if (dialog_filename_input_.empty()) {
        ShowStatus("[ERROR] 请输入有效的文件名", true);
        return;
    }
    
    // 设置输出文件路径
    output_file_path_ = dialog_filename_input_;
    // 将最新名字写回配置
    config_manager_.SetRecordFileName(dialog_filename_input_);
    config_manager_.SaveConfig(config_file_path_);
    
    // 设置输入文件路径
    input_file_path_ = pending_input_file_path_;
    input_file_selected_ = true;
    
    // 隐藏对话框
    HideFilenameDialog();
    
    // 开始转换
    ShowStatus("[OK] 开始转换模型...");
    ExecuteConversion();
}

/**
 * @brief 取消文件名输入
 */
void MNNConvertGUI::CancelFilenameInput() {
    // 清空待处理的文件路径
    pending_input_file_path_.clear();
    output_file_path_.clear();
    
    // 隐藏对话框
    HideFilenameDialog();
    
    ShowStatus("[INFO] 已取消文件输入");
}

/**
 * @brief 渲染文件名输入对话框
 */
void MNNConvertGUI::RenderFilenameDialog() {
    // 绘制半透明背景遮罩
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    SDL_Rect mask = {0, 0, window_width_, window_height_};
    SDL_RenderFillRect(renderer_, &mask);
    
    // 绘制对话框背景（圆角）
    int dialog_radius = 12;
    SDL_Color dialog_bg = {220, 220, 220, 255};
    if (dialog_bg_animating_) {
        Uint32 now = SDL_GetTicks();
        Uint32 elapsed = now - dialog_bg_anim_start_;
        float t = (dialog_bg_anim_duration_ms_ == 0) ? 1.0f : std::min(1.0f, elapsed / static_cast<float>(dialog_bg_anim_duration_ms_));
        // 缓入缓出
        float ease = (t < 0.5f) ? (2.0f * t * t) : (1.0f - ((-2.0f * t + 2.0f) * (-2.0f * t + 2.0f)) / 2.0f);
        auto lerpC = [&](Uint8 a, Uint8 b) -> Uint8 { return static_cast<Uint8>(a + (b - a) * ease); };
        dialog_bg.r = lerpC(dialog_bg_start_color_.r, dialog_bg_target_color_.r);
        dialog_bg.g = lerpC(dialog_bg_start_color_.g, dialog_bg_target_color_.g);
        dialog_bg.b = lerpC(dialog_bg_start_color_.b, dialog_bg_target_color_.b);
        dialog_bg.a = lerpC(dialog_bg_start_color_.a, dialog_bg_target_color_.a);
        if (t >= 1.0f) {
            dialog_bg_animating_ = false;
        }
    }
    RenderRoundedRect(dialog_rect_, dialog_radius, dialog_bg, true);
    
    // 绘制标题
    RenderCenteredText("输入转换后的文件名字（不含扩展名）", dialog_rect_.x + dialog_rect_.w / 2, dialog_rect_.y + 20, {0, 0, 0, 255}, {240, 240, 240, 255});
    
    // 显示输入文件信息
    std::string input_filename = GetBaseName(pending_input_file_path_);
    std::string display_name = input_filename.length() > 25 ? input_filename.substr(0, 22) + "..." : input_filename;
    RenderCenteredText("输入文件: " + display_name, dialog_rect_.x + dialog_rect_.w / 2, dialog_rect_.y + 45, {100, 100, 100, 255}, {240, 240, 240, 255});
    
    // 绘制输入框
    RenderInputField(dialog_input_rect_, dialog_filename_input_, dialog_input_active_);
    
    // 绘制按钮
    bool ok_hovered = IsPointInRect(mouse_x_, mouse_y_, dialog_ok_button_);
    bool cancel_hovered = IsPointInRect(mouse_x_, mouse_y_, dialog_cancel_button_);
    
    RenderButton(dialog_ok_button_, "确定", ok_hovered);
    RenderButton(dialog_cancel_button_, "取消", cancel_hovered);
}

/**
 * @brief 处理对话框鼠标点击
 * @param x 鼠标X坐标
 * @param y 鼠标Y坐标
 */
void MNNConvertGUI::HandleDialogMouseClick(int x, int y) {
    // 检查是否点击了确定按钮
    if (IsPointInRect(x, y, dialog_ok_button_)) {
        ConfirmFilenameInput();
        return;
    }
    
    // 检查是否点击了取消按钮
    if (IsPointInRect(x, y, dialog_cancel_button_)) {
        CancelFilenameInput();
        return;
    }
    
    // 检查是否点击了输入框
    if (IsPointInRect(x, y, dialog_input_rect_)) {
        dialog_input_active_ = true;
        return;
    }
    
    // 点击对话框外部，激活输入框
    if (IsPointInRect(x, y, dialog_rect_)) {
        dialog_input_active_ = true;
    }
}

/**
 * @brief 处理对话框键盘输入
 * @param key 按键代码
 * @param text_input 文本输入
 */
void MNNConvertGUI::HandleDialogKeyInput(SDL_Keycode key, const std::string& text_input) {
    if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
        // 用户按回车，确认输入
        ConfirmFilenameInput();
        return;
    }
    
    if (key == SDLK_ESCAPE) {
        // 用户按ESC，取消输入
        CancelFilenameInput();
        return;
    }
    
    if (key == SDLK_TAB) {
        // 切换输入框激活状态
        dialog_input_active_ = !dialog_input_active_;
        return;
    }
    
    // 处理文本输入
    if (dialog_input_active_) {
        if (key == SDLK_BACKSPACE && !dialog_filename_input_.empty()) {
            // 删除最后一个字符
            dialog_filename_input_.pop_back();
        } else if (!text_input.empty() && text_input[0] >= 32 && text_input[0] <= 126) {
            // 添加字符到文件名
            dialog_filename_input_ += text_input;
        }
    }
}

