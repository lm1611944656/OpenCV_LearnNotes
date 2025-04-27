# 指定 CMake 的最低版本要求
cmake_minimum_required(VERSION 3.10)

# 指定项目名称
project(application
        VERSION 1.0.0)

# ******************************************************************

# 指定可执行文件最后运行在哪个操作系统上
set(CMAKE_SYSTEM_NAME Linux)

# 指定可执行文件最后运行在哪种系统架构上 (arm 还是 x86)
set(CMAKE_SYSTEM_PROCESSOR x86)

# 指定 C 语言的编译工具 (换成自己的交叉工具链工具所在的绝对路径)
set(CMAKE_C_COMPILER /usr/bin/gcc)

# 指定 CXX 语言的编译工具 (换成自己的交叉工具链工具所在的绝对路径)
set(CMAKE_CXX_COMPILER /usr/bin/g++)

# 查看自己的编译工具链
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")

# *****************************************************************
# 查找配置文件, 这个配置文件是必须的 (没它不行)
find_package(OpenCV REQUIRED)
message(STATUS "Found OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV libraries to link: ${OpenCV_LIBS}")
message(STATUS "OpenCV include directories: ${OpenCV_INCLUDE_DIRS}")

# 提取 OpenCV 库的目录路径
# 提取 OpenCV 库的实际路径
set(OPENCV_LIB_DIRS "")
foreach(LIB_NAME ${OpenCV_LIBS})
    # 查找库的实际路径
    find_library(LIB_PATH ${LIB_NAME} PATHS /usr/local/lib /usr/lib/x86_64-linux-gnu)
    
    if(LIB_PATH)
        # 获取库文件所在的目录
        get_filename_component(LIB_DIR ${LIB_PATH} DIRECTORY)
        
        # 添加到集合中，避免重复
        list(APPEND OPENCV_LIB_DIRS ${LIB_DIR})
    else()
        message(WARNING "Library ${LIB_NAME} not found!")
    endif()
endforeach()

# 去重
list(REMOVE_DUPLICATES OPENCV_LIB_DIRS)

# 打印提取到的库目录路径
message(STATUS "OpenCV的库目录: ${OPENCV_LIB_DIRS}")
#*******************************************************************************

# 到子目录下执行 CMakeLists.txt 文件
add_subdirectory(${PROJECT_SOURCE_DIR}/src/yolo)
add_subdirectory(${PROJECT_SOURCE_DIR}/src/utils)
message(STATUS "项目目录：${PROJECT_SOURCE_DIR}")

# 扫描项目目录并将所有源文件存储到 SRC_LIST 变量中
aux_source_directory(${CMAKE_SOURCE_DIR} SRC_LIST)
message(STATUS "所有源文件：${SRC_LIST}")

# 增加可执行文件
add_executable(${PROJECT_NAME} ${SRC_LIST})

# 为目标添加头文件
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/src/yolo
    ${CMAKE_SOURCE_DIR}/src/utils
    ${OpenCV_INCLUDE_DIRS}  # 添加 OpenCV 的头文件路径
)

# 为目标链接库目录
target_link_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/library
)

# 可执行文件链接动态库
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    pthread                 # 使用多线程需要包含这个库
    UTILS                   # 链接动态库 libUTILS.so
    YOLO                    # 链接动态库 libYOLO.so
)

# *****************************************************************

# 安装规则
# 硬编码安装路径
set(CMAKE_INSTALL_PREFIX 
    "/home/lmz/test_demo" 
    CACHE PATH "Installation prefix" FORCE)

# 获取所有头文件路径
file(GLOB_RECURSE HEADER_FILES
    "${CMAKE_SOURCE_DIR}/src/yolo/*.h"
    "${CMAKE_SOURCE_DIR}/src/utils/*.h"
)

# 获取 OpenCV 的头文件路径
file(GLOB_RECURSE OPENCV_HEADER_FILES
    "${OpenCV_INCLUDE_DIRS}/*.h"
    "${OpenCV_INCLUDE_DIRS}/*.hpp"
)

# 合并所有头文件路径
set(ALL_HEADER_FILES ${HEADER_FILES} ${OPENCV_HEADER_FILES})

# 安装头文件到指定目录，不保留子目录结构
install(FILES ${ALL_HEADER_FILES}
    DESTINATION include
)

# 安装 OpenCV 的库文件到指定目录
install(DIRECTORY ${OpenCV_LIBRARY_DIRS}
    DESTINATION lib
    FILES_MATCHING PATTERN "*.so*"
)

# 安装库文件到指定目录
install(TARGETS UTILS YOLO
    LIBRARY DESTINATION lib)

# 设置 rpath，让可执行文件知道动态库的路径
set_target_properties(${PROJECT_NAME} PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "$ORIGIN/../lib"
)

# 安装可执行文件到指定目录
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin)