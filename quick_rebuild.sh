#!/bin/bash

# 快速重新构建脚本

echo "=============== 快速重新构建 ==============="
echo ""
echo "[INFO] 重新构建程序以应用最新修改..."

# 检查是否在正确的目录
# if [ ! -f "MNNConvertGUI.cpp" ]; then
#     echo "[ERROR] 请在myAPP目录中运行此脚本"
#     exit 1
# fi

# 进入构建目录
if [ ! -d "build" ]; then
    echo "[INFO] 创建构建目录..."
    mkdir build
    cd build
    cmake ..
else
    rm -rf build
    mkdir build
    cd build
    cmake ..
fi



# 快速构建
echo "[INFO] 编译程序..."
if make -j$(nproc); then
    echo "[OK] 构建成功！"
    
    if [ -f "MNNConvertGUI" ]; then
        echo ""
        echo "=============== 功能更新 ==============="
        echo ""
        echo "[UPDATE] 文件对话框卡死问题已修复："
        echo "  ✓ 文件对话框现在有10秒超时保护"
        echo "  ✓ 增强的错误处理和状态反馈"
        echo "  ✓ 如果zenity失败，提供终端输入备用方案"
        echo "  ✓ 更好的路径验证和目录检查"
        echo ""
        echo "[UPDATE] 输出文件路径逻辑已更新："
        echo "  ✓ 现在输出文件会与输入文件保持相同路径"
        echo "  ✓ 只将扩展名从 .onnx 改为 .mnn"
        echo ""
        echo "例如："
        echo "  输入文件: /home/user/models/model.onnx"
        echo "  输出文件: /home/user/models/model.mnn"
        echo ""
        echo "[IMPORTANT] 如果文件对话框仍然有问题："
        echo "  - 程序会自动切换到终端输入模式"
        echo "  - 您可以直接在终端中输入文件路径"
        echo "  - 支持拖拽文件到终端自动输入路径"
        echo ""
        echo "立即运行程序测试新功能？"
        read -p "是否运行程序? (y/n): " -n 1 -r
        echo ""
        
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            echo "[INFO] 启动程序..."
            echo "请选择一个输入文件，观察输出文件路径是否正确生成"
            echo ""
            ./MNNConvertGUI
        fi
        
        echo ""
        echo "[INFO] 程序已更新，可以正常使用了！"
        echo "[INFO] 可执行文件位置: $(pwd)/MNNConvertGUI"
        
    else
        echo "[ERROR] 可执行文件未生成"
        exit 1
    fi
    
else
    echo "[ERROR] 构建失败"
    echo ""
    echo "如果构建失败，请尝试完整重新构建："
    echo "  rm -rf build"
    echo "  ./debug_build.sh"
    exit 1
fi
