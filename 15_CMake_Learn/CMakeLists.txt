# 1. 指定cmake的最低版本要求
cmake_minimum_required(VERSION 3.10)

# 2. 指定项目名称，项目版本，项目描述，项目使用的语言
project(application
        VERSION 1.1.1
        DESCRIPTION "QQ程序"
        LANGUAGES CXX)   


message("当前系统名称：${CMAKE_SYSTEM_NAME}")    # 获取系统名称
message("是否是WIN32: ${WIN32}")                # 判断是否是windows
message("是否是UNIX: ${UNIX}")                 # 判断是否是UNIX

# 查找配置文件, 这个配置文件是必须的 (没它不行)
find_package(OpenCV REQUIRED)
message(STATUS "Found OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV libraries to link: ${OpenCV_LIBS}")
message(STATUS "OpenCV include directories: ${OpenCV_INCLUDE_DIRS}")

# 去指定的目录下执行cmake文件
add_subdirectory(${PROJECT_SOURCE_DIR}/tasks)
add_subdirectory(${PROJECT_SOURCE_DIR}/tasks2)

# 3.生成可执行文件
add_executable(${PROJECT_NAME} main.cpp)

# 为目标添加头文件
target_include_directories(${PROJECT_NAME}  PUBLIC
    ${PROJECT_SOURCE_DIR}/tasks
    ${PROJECT_SOURCE_DIR}/tasks2
)

# 为目标指定库目录所在目录
target_link_directories(${PROJECT_NAME}  PUBLIC
    ${PROJECT_SOURCE_DIR}/libarys
)

# 为目标添加库
target_link_libraries(${PROJECT_NAME}
    pthread                 # 使用多线程需要包含这个库
    slib                    # libslib.so
    alib                    # libalib.a
)

# 软件打包安装
#**********************************************************************

# # 打印系统的默认安装路径
# message("cmake的默认安装路径: ${CMAKE_INSTALL_PREFIX}")

# # 修改安装路径
# set(CMAKE_INSTALL_PREFIX   "/home/lmz/test_demo" 
#     CACHE PATH "Installation prefix" FORCE)

# # 获取所有头文件路径
# file(GLOB_RECURSE HEADER_FILES
#     "${CMAKE_SOURCE_DIR}/tasks/*.h"
# )

# # 获取 OpenCV 的头文件路径
# file(GLOB_RECURSE OPENCV_HEADER_FILES
#     "${OpenCV_INCLUDE_DIRS}/*.h"
#     "${OpenCV_INCLUDE_DIRS}/*.hpp"
# )

# # 合并所有头文件路径
# set(ALL_HEADER_FILES ${HEADER_FILES} ${OPENCV_HEADER_FILES})

# # 安装头文件到指定目录，不保留子目录结构
# install(FILES ${ALL_HEADER_FILES}
#     DESTINATION include
# )

# # 设置 rpath，让可执行文件知道动态库的路径
# set_target_properties(${PROJECT_NAME} PROPERTIES
#     BUILD_WITH_INSTALL_RPATH TRUE
#     INSTALL_RPATH "$ORIGIN/../lib"
# )

# # 安装
# install(
#     TARGETS ${PROJECT_NAME}      # TARGET 指代可执行文件
#     RUNTIME DESTINATION bin      # 为可执行文件指定存放目录 /home/lmz/test_demo/bin
#     LIBRARY DESTINATION lib      # 为动态库指定存放目录 /home/lmz/test_demo/lib
#     ARCHIVE DESTINATION lib      # 为静态库指定存放目录 /home/lmz/test_demo/lib
# )