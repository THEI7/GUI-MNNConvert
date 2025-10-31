cmake_minimum_required(VERSION 3.10)
project(MNNConvertGUI)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找SDL2库
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
pkg_check_modules(SDL2_GFX REQUIRED SDL2_gfx)

# 查找yaml-cpp库
find_package(yaml-cpp REQUIRED)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# 包含目录
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_TTF_INCLUDE_DIRS})
include_directories(${SDL2_GFX_INCLUDE_DIRS})

# 源文件
set(SOURCES
    main.cpp
    src/MNNConvertGUI.cpp
    src/ConfigManager.cpp
)

# 头文件
set(HEADERS
    include/MNNConvertGUI.h
    include/ConfigManager.h
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 链接库
target_link_libraries(${PROJECT_NAME} 
    ${SDL2_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${SDL2_GFX_LIBRARIES}
    yaml-cpp
    stdc++fs
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 设置编译器选项
target_compile_options(${PROJECT_NAME} PRIVATE ${SDL2_CFLAGS_OTHER})
target_compile_options(${PROJECT_NAME} PRIVATE ${SDL2_TTF_CFLAGS_OTHER})
target_compile_options(${PROJECT_NAME} PRIVATE ${SDL2_GFX_CFLAGS_OTHER})

# 设置MNNConvert可执行文件路径
set(MNNCONVERT_PATH "${CMAKE_SOURCE_DIR}/../MNNConvert")
add_definitions(-DMNNCONVERT_PATH="${MNNCONVERT_PATH}")

# 打印信息
message(STATUS "SDL2 libraries: ${SDL2_LIBRARIES}")
message(STATUS "SDL2_TTF libraries: ${SDL2_TTF_LIBRARIES}")
message(STATUS "SDL2_GFX libraries: ${SDL2_GFX_LIBRARIES}")
message(STATUS "MNNConvert path: ${MNNCONVERT_PATH}")




