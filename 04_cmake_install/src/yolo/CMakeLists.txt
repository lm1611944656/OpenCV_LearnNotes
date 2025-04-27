# 指定头文件所在目录，方便yolo目录下的源文件使用
include_directories(${PROJECT_SOURCE_DIR}/src/utils)

# 遍历当前文件下的所有源文件，放入YOLO_SOURCE变量中
aux_source_directory(${PROJECT_SOURCE_DIR}/src/yolo YOLO_SOURCE)
message(STATUS "yolo的源码: ${YOLO_SOURCE}")

# 设置动态库输出路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/library)

# 将yolo目录下的所有源文件，封装成动态库文件YOLO.so
add_library(YOLO SHARED ${YOLO_SOURCE})