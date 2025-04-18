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

# 到子目录下执行CMakeLists.txt文件
add_subdirectory(${PROJECT_SOURCE_DIR}/src/func1)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/func2)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/utils)
message(STATUS "项目目录：${PROJECT_SOURCE_DIR}")

# 包含头文件，方便src目录下的源文件使用
include_directories(
    ${CMAKE_SOURCE_DIR}/src/func1
    ${CMAKE_SOURCE_DIR}/src/func2
    ${CMAKE_SOURCE_DIR}/src/utils)

# 指定动态库和静态库所在位置
link_directories(${PROJECT_SOURCE_DIR}/library)

# 指定静态库的名称libUTILS.a
link_libraries(UTILS)

# 扫描 项目 目录并将所有源文件存储到 src_list 变量中
aux_source_directory(${CMAKE_SOURCE_DIR} SRC_LIST)
message(STATUS "所有源文件：${SRC_LIST}")

# 增加可执行文件
add_executable(application ${SRC_LIST})

message(STATUS "OpenCV 库：${OpenCV_LIBS}")

# 可执行文件链接动态库
target_link_libraries(application
    ${OpenCV_LIBS}
    pthread                 # 使用多线程需要包含这个库
    FUNC1                   # 链接动态库libFUNC1.so
    FUNC2                   # 链接动态库libFUNC2.so
)