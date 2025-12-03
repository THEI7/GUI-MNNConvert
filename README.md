# MNN模型转换工具 GUI版本

这是一个基于SDL2的图形用户界面应用程序，用于简化MNNConvert命令行工具的使用。通过直观的界面操作，您可以轻松地将ONNX模型转换为MNN格式。


## 系统要求

- Linux操作系统 (Ubuntu/Debian推荐)
- CMake 3.10或更高版本
- C++17兼容的编译器 (GCC 8.0+或Clang 7.0+)
- SDL2开发库 (版本2.0.5+)
- SDL2_ttf开发库 (版本2.0.14+)
- yaml-cpp开发库 (版本0.6.0+) - 配置文件支持
- zenity (用于文件对话框)
- 中文字体包 (推荐)

## 依赖安装

### Ubuntu/Debian系统 (推荐):
```bash
# 更新包管理器索引
sudo apt update

# 安装基础编译工具
sudo apt install cmake build-essential pkg-config

# 安装SDL2图形库和字体库
sudo apt install libsdl2-dev libsdl2-ttf-dev  libsdl2-gfx-dev

# 安装YAML配置文件支持库
sudo apt install libyaml-cpp-dev

# 安装文件对话框工具
sudo apt install zenity

# 安装中文字体包（推荐，用于更好的中文显示）
sudo apt install fonts-wqy-microhei fonts-wqy-zenhei
sudo apt install fonts-arphic-uming fonts-arphic-ukai

# 验证安装（可选）
pkg-config --modversion sdl2 sdl2_ttf yaml-cpp
```

### Fedora/CentOS 8+/RHEL 8+系统:
```bash
# 安装基础编译工具
sudo dnf install cmake gcc-c++ pkg-config

# 安装SDL2相关库
sudo dnf install SDL2-devel SDL2_ttf-devel

# 安装YAML支持库
sudo dnf install yaml-cpp-devel

# 安装文件对话框工具
sudo dnf install zenity

# 安装中文字体包
sudo dnf install wqy-microhei-fonts wqy-zenhei-fonts
```

### CentOS 7/RHEL 7系统:
```bash
# 安装基础编译工具
sudo yum install cmake gcc-c++ pkg-config

# 启用EPEL仓库（用于获取更多包）
sudo yum install epel-release

# 安装SDL2相关库
sudo yum install SDL2-devel SDL2_ttf-devel

# 安装YAML支持库
sudo yum install yaml-cpp-devel

# 安装文件对话框工具
sudo yum install zenity

# 安装中文字体包
sudo yum install wqy-microhei-fonts wqy-zenhei-fonts
```

### Arch Linux系统:
```bash
# 安装基础编译工具
sudo pacman -S cmake gcc pkg-config

# 安装SDL2相关库
sudo pacman -S sdl2 sdl2_ttf

# 安装YAML支持库
sudo pacman -S yaml-cpp

# 安装文件对话框工具
sudo pacman -S zenity

# 安装中文字体包
sudo pacman -S wqy-microhei wqy-zenhei
```

### openSUSE系统:
```bash
# 安装基础编译工具
sudo zypper install cmake gcc-c++ pkg-config

# 安装SDL2相关库
sudo zypper install libSDL2-devel libSDL2_ttf-devel

sudo apt-get install -y libsdl2-gfx-dev


# 安装YAML支持库
sudo zypper install yaml-cpp-devel

# 安装文件对话框工具
sudo zypper install zenity

# 安装中文字体包
sudo zypper install wqy-microhei-fonts wqy-zenhei-fonts
```

### 从源码编译yaml-cpp（如果包管理器没有）:
```bash
# 克隆源码
git clone https://github.com/jbeder/yaml-cpp.git
cd yaml-cpp

# 编译安装
mkdir build && cd build
cmake -DYAML_BUILD_SHARED_LIBS=ON ..
make -j$(nproc)
sudo make install

# 更新库缓存
sudo ldconfig
```

### MNNConvert工具安装:
[https://github.com/alibaba/MNN/wiki/convert#%E6%A8%A1%E5%9E%8B%E8%BD%AC%E6%8D%A2%E5%B7%A5%E5%85%B7](MNNconvert工具下载连接)
```bash
git clone git@github.com:alibaba/MNN.git && \
cd MNN && \
mkdir build && \
cd build && \
cmake .. -DMNN_BUILD_CONVERTER=ON -DMNN_BUILD_TORCH=ON && \
make -j8
```

## 构建步骤

### 快速构建（推荐）:
```bash
cd /home/fangjunjie/tools/MNN/myAPP
./quick_rebuild.sh
```

### 手动构建:
1. 进入项目目录:
```bash
cd /home/fangjunjie/tools/MNN/myAPP
```

2. 创建构建目录:
```bash
mkdir -p build
cd build
```

3. 配置项目:
```bash
cmake ..
```

4. 编译项目:
```bash
make -j$(nproc)
```

5. 返回项目根目录并运行:
```bash
cd ..
./build/MNNConvertGUI
```

### 配置文件设置:
项目使用 `config/default.yaml` 配置文件，包含以下关键设置：

```yaml
# MNNConvert程序路径 - 可根据实际情况修改
mnnconvert_path: "/home/fangjunjie/tools/MNN/MNNConvert"

# 输出目录 - 转换后文件的保存位置
output_dir: "/home/fangjunjie/tools/MNN"

# 业务代码 - 默认的bizCode参数
biz_code: "biz"

# 是否自动打开输出目录
auto_open_output_dir: true
```

**重要**: 首次运行前，请检查并更新 `config/default.yaml` 中的 `mnnconvert_path` 为您系统中 MNNConvert 程序的实际路径。

## 使用说明

### 界面操作流程:

1. **选择输入文件**: 点击"选择输入文件"按钮，选择要转换的ONNX模型文件
2. **设置输出文件**: 点击"选择输出文件"按钮，设置转换后MNN模型的保存路径
3. **设置业务代码**: 在业务代码输入框中输入自定义的bizCode (默认为"biz")
4. **开始转换**: 点击"开始转换"按钮执行模型转换
5. **查看状态**: 在界面底部查看转换状态和结果反馈

### 快捷操作:

- 选择输入文件后，系统会自动生成对应的输出文件名
- 使用"清空"按钮可以重置所有设置
- 支持键盘输入修改业务代码
- 状态消息会自动在3秒后消失

## 命令行等效操作

该GUI工具实际执行的命令格式为:
```bash
./MNNConvert -f ONNX --modelFile <输入文件> --MNNModel <输出文件> --bizCode <业务代码>
```

例如:
```bash
./MNNConvert -f ONNX --modelFile seres_1029_02.onnx --MNNModel seres_1029_02.mnn --bizCode biz
```

## 注意事项

1. **配置文件**: 首次运行前请检查 `config/default.yaml` 配置文件，特别是 `mnnconvert_path` 路径设置
2. **MNNConvert路径**: 通过配置文件指定MNNConvert程序路径，支持绝对路径和相对路径
3. **工作目录**: 程序需要从项目根目录启动，以正确加载配置文件
4. **文件权限**: 确保对输入文件有读取权限，对输出目录有写入权径
5. **模型格式**: 目前仅支持ONNX到MNN的转换
6. **字体支持**: 推荐安装中文字体包以获得更好的显示效果
7. **对话框依赖**: 文件选择功能依赖zenity等图形工具
8. **多线程安全**: 转换过程在后台线程进行，界面始终保持响应
