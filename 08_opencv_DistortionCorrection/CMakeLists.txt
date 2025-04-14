# 指定cmake的最低版本要求
cmake_minimum_required(VERSION 3.10)

# 指定项目名称
project(application)

# ******************************************************************

# 指定可执行文件最后运行在哪个操作系统上
set(CMAKE_SYSTEM_NAME Linux)

# 指定可执行文件最后运行在哪中系统架构上(arm还是x86)
set(CMAKE_SYSTEM_PROCESSOR x86)

# 指定C语言的编译工具(换成自己的交叉工具链工具所在的绝对路径)
set(CMAKE_C_COMPILER /usr/bin/gcc)

# 指定CXX语言的编译工具(换成自己的交叉工具链工具所在的绝对路径)
set(CMAKE_CXX_COMPILER /usr/bin/g++)

# 查看自己的编译工具链
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")

# *****************************************************************

# 查找配置文件, 这个配置文件是必须的(没它不行)
find_package(OpenCV REQUIRED)
message(STATUS "Found OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV libraries to link: ${OpenCV_LIBS}")
message(STATUS "OpenCV include directories: ${OpenCV_INCLUDE_DIRS}")

# *****************************************************************
# 到子目录下执行CMakeLists.txt文件
add_subdirectory(${PROJECT_SOURCE_DIR}/src)

# message("src中的变量为：${NEW_SRC_LIST}")

# 包含头文件，方便src目录下的源文件使用
include_directories(
    ${CMAKE_SOURCE_DIR}/src)

# 指定动态库和静态库所在位置
link_directories(${PROJECT_SOURCE_DIR}/librarys)

# 增加可执行文件
add_executable(application main.cpp)

message(STATUS "OpenCV 库：${OpenCV_LIBS}")

# 可执行文件链接动态库
target_link_libraries(application
    ${OpenCV_LIBS}
    SRC_LIB
)