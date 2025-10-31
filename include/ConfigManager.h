#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <string>
#include <yaml-cpp/yaml.h>

/**
 * @brief 配置管理类
 *        负责YAML配置文件的读写操作
 */
class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();

    bool LoadConfig(const std::string& config_path);
    bool SaveConfig(const std::string& config_path);
    
    // 获取配置项
    std::string GetMNNConvertPath() const;
    std::string GetOutputDirectory() const;
    std::string GetBizCode() const;
    bool GetAutoOpenOutputDir() const;
    std::string GetDefaultFont() const;
    std::string GetRecordFileName() const;
    
    // 设置配置项
    void SetMNNConvertPath(const std::string& convert_path);
    void SetOutputDirectory(const std::string& output_dir);
    void SetBizCode(const std::string& biz_code);
    void SetAutoOpenOutputDir(bool auto_open);
    void SetDefaultFont(const std::string& font_path);
    void SetRecordFileName(const std::string& record_name);
    
    // 配置文件是否存在且有效
    bool IsConfigValid() const;
    
    // 创建默认配置文件
    bool CreateDefaultConfig(const std::string& config_path);

private:
    YAML::Node config_;
    bool config_loaded_;
    std::string current_config_path_;
    
    // 默认配置值
    void InitializeDefaults();
    
    // 验证配置项
    bool ValidateConfig() const;
};

#endif // CONFIGMANAGER_H
