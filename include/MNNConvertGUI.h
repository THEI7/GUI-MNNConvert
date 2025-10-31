#ifndef MNNCONVERTGUI_H
#define MNNCONVERTGUI_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include "ConfigManager.h"

/**
 * @brief MNN模型转换图形用户界面类
 *        提供交互式界面来操作MNNConvert程序
 */
class MNNConvertGUI {
public:
    MNNConvertGUI();
    ~MNNConvertGUI();

    bool Initialize(int width = 800, int height = 600);
    void Run();
    void Cleanup();

private:
    // SDL相关成员
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    TTF_Font* font_;
    bool running_;

    // 界面尺寸
    int window_width_;
    int window_height_;

    // 文件路径相关
    std::string input_file_path_;
    std::string output_file_path_;
    std::string biz_code_;

    // 界面状态
    bool input_file_selected_;
    bool output_file_selected_;
    int active_input_field_;  // 0: 无, 1: 输入文件, 2: 输出文件, 3: bizCode

    // 界面元素位置
    SDL_Rect input_file_button_;
    SDL_Rect output_file_button_;
    SDL_Rect biz_code_input_;
    SDL_Rect convert_button_;
    SDL_Rect clear_button_;

    // 颜色定义
    SDL_Color color_background_;
    SDL_Color color_button_;
    SDL_Color color_button_hover_;
    SDL_Color color_text_;
    SDL_Color color_input_bg_;
    SDL_Color color_success_;
    SDL_Color color_error_;

    // 状态信息
    std::string status_message_;
    bool show_status_;
    Uint32 status_start_time_;

    // 多线程相关
    std::atomic<bool> is_converting_;
    std::thread conversion_thread_;
    std::mutex status_mutex_;
    std::atomic<bool> conversion_success_;
    std::string conversion_result_message_;
    
    // 拖拽和转换完成状态
    bool is_dragging_over_;
    Uint32 conversion_completed_time_;
    bool show_conversion_success_;
    
    // 文件名修改状态
    bool waiting_for_filename_input_;
    std::string pending_input_file_path_;
    
    // 图形化对话框状态
    bool show_filename_dialog_;
    std::string dialog_filename_input_;
    SDL_Rect dialog_rect_;
    SDL_Rect dialog_input_rect_;
    SDL_Rect dialog_ok_button_;
    SDL_Rect dialog_cancel_button_;
    bool dialog_input_active_;
    
    // 鼠标位置
    int mouse_x_;
    int mouse_y_;
    
    // 配置管理
    ConfigManager config_manager_;
    std::string config_file_path_;

    // 私有方法
    void HandleEvents();
    void Update();
    void Render();
    
    void RenderButton(const SDL_Rect& rect, const std::string& text, bool is_hovered = false, int font_size = 16);
    void RenderText(const std::string& text, int x, int y, SDL_Color color = {0, 0, 0, 255}, SDL_Color bg_color = {255, 255, 255, 255});
    void RenderCenteredText(const std::string& text, int center_x, int y, SDL_Color color = {0, 0, 0, 255}, SDL_Color bg_color = {255, 255, 255, 255});
    void RenderInputField(const SDL_Rect& rect, const std::string& text, bool is_active = false);
    void RenderDropZone();
    void RenderFileStatusInfo();
    void RenderRoundedRect(const SDL_Rect& rect, int radius, SDL_Color color, bool filled = true);
    void RenderFilenameDialog();
    void HandleDialogMouseClick(int x, int y);
    void HandleDialogKeyInput(SDL_Keycode key, const std::string& text_input);
    void ShowFilenameDialog();
    void HideFilenameDialog();
    void ConfirmFilenameInput();
    void CancelFilenameInput();
    
    bool IsPointInRect(int x, int y, const SDL_Rect& rect);
    void HandleMouseClick(int x, int y);
    void HandleKeyInput(SDL_Keycode key, const std::string& text_input);
    void HandleFileDrop(const char* file_path);
    
    void SelectInputFile();
    void SelectOutputFile();
    void ExecuteConversion();
    void ClearFields();
    
    void ShowStatus(const std::string& message, bool is_error = false);
    std::string OpenFileDialog(const std::string& title, const std::string& filter);
    std::string SaveFileDialog(const std::string& title, const std::string& default_name);
    
    bool ExecuteCommand(const std::string& command);
    std::string GetFileExtension(const std::string& filename);
    std::string GetBaseName(const std::string& filepath);
    
    // 异步转换相关方法
    void StartConversionThread(const std::string& command);
    void ConversionWorkerThread(const std::string& command);
    void CheckConversionResult();
    void CleanupConversionThread();
    
    // 备用文件选择方法
    std::string FallbackOpenFileDialog();
    std::string FallbackSaveFileDialog(const std::string& default_name);
    
    // 直接终端输入方法
    std::string DirectFileInput(const std::string& purpose, const std::string& filter_or_default, bool is_save);
    
    // 多种文件对话框实现
    std::string TryNativeFileDialog(const std::string& title, const std::string& filter, bool is_save);
    std::string TryZenityDialog(const std::string& title, const std::string& filter, bool is_save);
    std::string TryAlternativeFileDialog(const std::string& title, const std::string& filter, bool is_save);
    std::string TryKDialogFileDialog(const std::string& title, const std::string& filter, bool is_save);
    std::string TryXdgFileDialog(const std::string& title, const std::string& filter, bool is_save);
    std::string TryYadFileDialog(const std::string& title, const std::string& filter, bool is_save);
    
    // 打开文件位置
    bool OpenFileLocation(const std::string& file_path);
    
    // 配置管理相关方法
    bool LoadConfiguration();
    bool SaveConfiguration();
    std::string GenerateOutputPath(const std::string& input_path);
    void ApplyConfigurationSettings();
};

#endif // MNNCONVERTGUI_H


