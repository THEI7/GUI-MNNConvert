#include "MNNConvertGUI.h"
#include <iostream>

/**
 * @brief 程序主入口点
 *        创建并运行MNN转换GUI应用程序
 *
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 */
int main(int argc, char* argv[]) {
    (void)argc; // 避免未使用参数警告
    (void)argv; // 避免未使用参数警告
    std::cout << "[INFO] MNN模型转换工具启动中..." << std::endl;
    
    // 创建GUI应用程序实例
    MNNConvertGUI app;
    
    // 初始化应用程序
    if (!app.Initialize(800, 500)) {
        std::cerr << "[ERROR] 应用程序初始化失败" << std::endl;
        return -1;
    }

    // 运行应用程序主循环
    app.Run();
    
    std::cout << "[OK] 应用程序正常退出" << std::endl;
    return 0;
}




