# 指定头文件所在目录，方便func2目录下的源文件使用
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${OpenCV_INCLUDE_DIRS})

# 遍历当前文件下的所有源文件，放入IMGCLS_SOURCE变量中
aux_source_directory(${PROJECT_SOURCE_DIR}/src IMGCLS_SOURCE)
message(STATUS "src的源码: ${IMGCLS_SOURCE}")


# 指定动态库生成的位置
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/library)

# 将src目录下的所有源文件，封装成动态库文件IMGCLS.so
add_library(IMGCLS SHARED ${IMGCLS_SOURCE})