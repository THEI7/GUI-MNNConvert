#include "ConfigManager.h"
#include <iostream>
#include <fstream>
#include <filesystem>

/**
 * @brief 构造函数
 *        初始化配置管理器
 */
ConfigManager::ConfigManager() 
    : config_loaded_(false)
    , current_config_path_("") {
    InitializeDefaults();
}

/**
 * @brief 析构函数
 */
ConfigManager::~ConfigManager() {
}

/**
 * @brief 初始化默认配置值
 */
void ConfigManager::InitializeDefaults() {
    config_["mnnconvert_path"] = "/home/pozion/tools/MNN/MNNConvert";
    config_["output_dir"] = "/home/pozion/tools/MNN";
    config_["biz_code"] = "biz";
    config_["auto_open_output_dir"] = true;
    config_["default_font"] = "";
    config_["record_file_name"] = "";
}

/**
 * @brief 加载配置文件
 *        
 * @param config_path 配置文件路径
 */
bool ConfigManager::LoadConfig(const std::string& config_path) {
    try {
        if (!std::filesystem::exists(config_path)) {
            std::cout << "[WARNING] 配置文件不存在: " << config_path << std::endl;
            std::cout << "[INFO] 将创建默认配置文件" << std::endl;
            return CreateDefaultConfig(config_path);
        }
        
        config_ = YAML::LoadFile(config_path);
        current_config_path_ = config_path;
        config_loaded_ = true;
        
        // 验证配置项，如果缺少必要配置则添加默认值
        bool config_modified = false;
        
        if (!config_["mnnconvert_path"]) {
            config_["mnnconvert_path"] = "/home/pozion/tools/MNN/MNNConvert";
            config_modified = true;
        }
        
        if (!config_["output_dir"]) {
            config_["output_dir"] = "/home/pozion/tools/MNN";
            config_modified = true;
        }
        
        if (!config_["biz_code"]) {
            config_["biz_code"] = "biz";
            config_modified = true;
        }
        
        if (!config_["auto_open_output_dir"]) {
            config_["auto_open_output_dir"] = true;
            config_modified = true;
        }
        
        if (!config_["default_font"]) {
            config_["default_font"] = "";
            config_modified = true;
        }
        
        if (!config_["record_file_name"]) {
            config_["record_file_name"] = "";
            config_modified = true;
        }
        
        // 如果配置被修改，保存更新后的配置
        if (config_modified) {
            SaveConfig(config_path);
        }
        
        std::cout << "[OK] 配置文件加载成功: " << config_path << std::endl;
        return true;
        
    } catch (const YAML::Exception& e) {
        std::cerr << "[ERROR] YAML解析错误: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] 配置文件加载失败: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief 保存配置文件
 *        
 * @param config_path 配置文件路径
 */
bool ConfigManager::SaveConfig(const std::string& config_path) {
    try {
        std::ofstream file(config_path);
        if (!file.is_open()) {
            std::cerr << "[ERROR] 无法创建配置文件: " << config_path << std::endl;
            return false;
        }
        
        file << "# MNN模型转换工具配置文件" << std::endl;
        file << "# MNNConvert程序路径 - MNNConvert可执行文件的绝对路径" << std::endl;
        file << "mnnconvert_path: \"" << config_["mnnconvert_path"].as<std::string>() << "\"" << std::endl;
        file << "" << std::endl;
        file << "# 输出目录 - 转换后的MNN模型文件保存路径" << std::endl;
        file << "output_dir: \"" << config_["output_dir"].as<std::string>() << "\"" << std::endl;
        file << "" << std::endl;
        file << "# 业务代码 - MNN模型的业务标识" << std::endl;
        file << "biz_code: \"" << config_["biz_code"].as<std::string>() << "\"" << std::endl;
        file << "" << std::endl;
        file << "# 自动打开输出目录 - 转换完成后是否自动打开文件所在目录" << std::endl;
        file << "auto_open_output_dir: " << (config_["auto_open_output_dir"].as<bool>() ? "true" : "false") << std::endl;
        file << "" << std::endl;
        file << "# 默认字体路径 - GUI界面使用的字体文件路径(可选)" << std::endl;
        file << "default_font: \"" << config_["default_font"].as<std::string>() << "\"" << std::endl;
        file << "" << std::endl;
        file << "record_file_name: \"" << config_["record_file_name"].as<std::string>() << "\"" << std::endl;
        
        file.close();
        
        current_config_path_ = config_path;
        std::cout << "[OK] 配置文件保存成功: " << config_path << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] 配置文件保存失败: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief 获取MNNConvert程序路径
 */
std::string ConfigManager::GetMNNConvertPath() const {
    if (config_["mnnconvert_path"]) {
        return config_["mnnconvert_path"].as<std::string>();
    }
    return "/home/pozion/tools/MNN/MNNConvert";
}

/**
 * @brief 获取输出目录
 */
std::string ConfigManager::GetOutputDirectory() const {
    if (config_["output_dir"]) {
        return config_["output_dir"].as<std::string>();
    }
    return "/home/pozion/tools/MNN";
}

/**
 * @brief 获取业务代码
 */
std::string ConfigManager::GetBizCode() const {
    if (config_["biz_code"]) {
        return config_["biz_code"].as<std::string>();
    }
    return "biz";
}

/**
 * @brief 获取是否自动打开输出目录
 */
bool ConfigManager::GetAutoOpenOutputDir() const {
    if (config_["auto_open_output_dir"]) {
        return config_["auto_open_output_dir"].as<bool>();
    }
    return true;
}

/**
 * @brief 获取默认字体路径
 */
std::string ConfigManager::GetDefaultFont() const {
    if (config_["default_font"]) {
        return config_["default_font"].as<std::string>();
    }
    return "";
}

/**
 * @brief 获取记录文件名
 */
std::string ConfigManager::GetRecordFileName() const {
    if (config_["record_file_name"]) {
        return config_["record_file_name"].as<std::string>();
    }
    return "";
}

/**
 * @brief 设置MNNConvert程序路径
 *        
 * @param convert_path MNNConvert程序路径
 */
void ConfigManager::SetMNNConvertPath(const std::string& convert_path) {
    config_["mnnconvert_path"] = convert_path;
}

/**
 * @brief 设置输出目录
 *        
 * @param output_dir 输出目录路径
 */
void ConfigManager::SetOutputDirectory(const std::string& output_dir) {
    config_["output_dir"] = output_dir;
}

/**
 * @brief 设置业务代码
 *        
 * @param biz_code 业务代码
 */
void ConfigManager::SetBizCode(const std::string& biz_code) {
    config_["biz_code"] = biz_code;
}

/**
 * @brief 设置是否自动打开输出目录
 *        
 * @param auto_open 是否自动打开
 */
void ConfigManager::SetAutoOpenOutputDir(bool auto_open) {
    config_["auto_open_output_dir"] = auto_open;
}

/**
 * @brief 设置默认字体路径
 *        
 * @param font_path 字体文件路径
 */
void ConfigManager::SetDefaultFont(const std::string& font_path) {
    config_["default_font"] = font_path;
}

/**
 * @brief 设置记录文件名
 *        用于回填并写回配置
 */
void ConfigManager::SetRecordFileName(const std::string& record_name) {
    config_["record_file_name"] = record_name;
}

/**
 * @brief 检查配置是否有效
 */
bool ConfigManager::IsConfigValid() const {
    return config_loaded_ && ValidateConfig();
}

/**
 * @brief 创建默认配置文件
 *        
 * @param config_path 配置文件路径
 */
bool ConfigManager::CreateDefaultConfig(const std::string& config_path) {
    InitializeDefaults();
    bool result = SaveConfig(config_path);
    if (result) {
        config_loaded_ = true;
    }
    return result;
}

/**
 * @brief 验证配置项
 */
bool ConfigManager::ValidateConfig() const {
    try {
        // 检查必要的配置项是否存在
        if (!config_["mnnconvert_path"] || !config_["output_dir"] || !config_["biz_code"]) {
            return false;
        }
        
        // 检查MNNConvert程序是否存在
        std::string mnnconvert_path = config_["mnnconvert_path"].as<std::string>();
        if (!mnnconvert_path.empty()) {
            if (!std::filesystem::exists(mnnconvert_path)) {
                std::cerr << "[WARNING] MNNConvert程序不存在: " << mnnconvert_path << std::endl;
                // 不返回false，允许程序继续运行，用户可以后续修改配置
            } else {
                std::cout << "[OK] MNNConvert程序路径验证通过: " << mnnconvert_path << std::endl;
            }
        }
        
        // 检查输出目录是否存在或可创建
        std::string output_dir = config_["output_dir"].as<std::string>();
        if (!output_dir.empty()) {
            // 尝试创建目录路径
            std::filesystem::create_directories(output_dir);
            if (!std::filesystem::exists(output_dir)) {
                std::cerr << "[WARNING] 输出目录不存在且无法创建: " << output_dir << std::endl;
                // 不返回false，允许程序继续运行，用户可以后续修改配置
            }
        }
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] 配置验证失败: " << e.what() << std::endl;
        return false;
    }
}
