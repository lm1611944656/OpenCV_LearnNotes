# cmake 需要这一行来指定最低支持的 CMake 版本
cmake_minimum_required(VERSION 3.1)

# 定义项目名称为 ImageShow
project(ImageShow)

# 查找 OpenCV 库
# 如果 OpenCV 没有自动找到，你可能需要通过命令行或 GUI 设置 OpenCV_DIR 变量
# OpenCV_DIR 应指向包含 OpenCVConfig.cmake 文件的目录
find_package(OpenCV REQUIRED)

message(STATUS "Found OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV libraries to link: ${OpenCV_LIBS}")
message(STATUS "OpenCV include directories: ${OpenCV_INCLUDE_DIRS}")
# 遍历每个头文件目录，列出其中的头文件
# foreach(dir IN LISTS OpenCV_INCLUDE_DIRS)
#     message(STATUS "Listing header files in directory: ${dir}")
    
#     # 使用 file(GLOB ...) 列出当前目录下的头文件
#     file(GLOB headers "${dir}/*.h")
#     foreach(header IN LISTS headers)
#         message(STATUS "Header file: ${header}")
#     endforeach()

#     # 如果需要递归列出子目录中的头文件，可以使用 GLOB_RECURSE
#     file(GLOB_RECURSE recursive_headers "${dir}/*.h")
#     foreach(recursive_header IN LISTS recursive_headers)
#         message(STATUS "Recursive header file: ${recursive_header}")
#     endforeach()
# endforeach()

# 启用 C++11 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)  # 强制要求使用 C++11

# 包含头文件所在目录
include_directories(
    /home/lmz/workspace/opencv_workspace/03_DNN_YOLOV5
)

# 声明可执行文件目标，从 main.cpp 源文件构建
add_executable(ImageShow main.cpp objectdetection.cpp)

# 将你的应用程序与 OpenCV 库进行链接
target_link_libraries(ImageShow PRIVATE ${OpenCV_LIBS})
